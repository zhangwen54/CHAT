#include "redis.hpp"
#include <iostream>
using namespace std;

Redis::Redis()
    : _publish_context(nullptr), _subcribe_context(nullptr)
{
}

Redis::~Redis()
{
    if (_publish_context != nullptr)
    {
        redisFree(_publish_context);
    }

    if (_subcribe_context != nullptr)
    {
        redisFree(_subcribe_context);
    }
}

bool Redis::connect()
{
    // 负责publish发布消息的上下文连接
    _publish_context = redisConnect("192.168.10.183", 6379);
    if (nullptr == _publish_context)
    {
        cerr << "connect redis failed!" << endl;
        return false;
    }
    //负责publish客户端的auth密码
    auto _publish_context_reply = (redisReply *)redisCommand(_publish_context, "AUTH %s", "123");
    if (_publish_context_reply->type == REDIS_REPLY_ERROR)
    {
        printf("Redis--_publish_context认证失败！\n");
    }
    else
    {
        printf("Redis--_publish_context认证成功！\n");
    }

    // 负责subscribe订阅消息的上下文连接
    _subcribe_context = redisConnect("192.168.10.183", 6379);
    if (nullptr == _subcribe_context)
    {
        cerr << "connect redis failed!" << endl;
        return false;
    }
    //负责subscribe客户端的auth密码
    auto _subcribe_context_reply = (redisReply *)redisCommand(_subcribe_context, "AUTH %s", "123");
    if (_subcribe_context_reply->type == REDIS_REPLY_ERROR)
    {
        printf("Redis--_subcribe_context认证失败！\n");
    }
    else
    {
        printf("Redis--_subcribe_context认证成功！\n");
    }

    // 在单独的线程中，监听通道上的事件，有消息给业务层进行上报
    thread t([&]()
             { observer_channel_message(); });
    t.detach();

    cout << "connect redis-server success!" << endl;

    return true;
}

// 向redis指定的通道channel发布消息
bool Redis::publish(int channel, string message)
{
    redisReply *reply = (redisReply *)redisCommand(_publish_context, "PUBLISH %d %s", channel, message.c_str());
    if (nullptr == reply)
    {
        cerr << "publish command failed!" << endl;
        return false;
    }
    freeReplyObject(reply);
    return true;
}

// 向redis指定的通道subscribe订阅消息
bool Redis::subscribe(int channel)
{
    // SUBSCRIBE命令本身会造成线程阻塞等待通道里面发生消息，这里只做订阅通道，不接收通道消息
    // 通道消息的接收专门在observer_channel_message函数中的独立线程中进行
    // 只负责发送命令，不阻塞接收redis server响应消息，否则和notifyMsg线程抢占响应资源
    if (REDIS_ERR == redisAppendCommand(this->_subcribe_context, "SUBSCRIBE %d", channel))
    {
        cerr << "subscribe command failed!" << endl;
        return false;
    }
    // redisBufferWrite可以循环发送缓冲区，直到缓冲区数据发送完毕（done被置为1）
    int done = 0;
    while (!done)
    {
        if (REDIS_ERR == redisBufferWrite(this->_subcribe_context, &done))
        {
            cerr << "subscribe command failed!" << endl;
            return false;
        }
    }
    // redisGetReply

    return true;
}

// 向redis指定的通道unsubscribe取消订阅消息
bool Redis::unsubscribe(int channel)
{
    if (REDIS_ERR == redisAppendCommand(this->_subcribe_context, "UNSUBSCRIBE %d", channel))
    {
        cerr << "unsubscribe command failed!" << endl;
        return false;
    }
    // redisBufferWrite可以循环发送缓冲区，直到缓冲区数据发送完毕（done被置为1）
    int done = 0;
    while (!done)
    {
        if (REDIS_ERR == redisBufferWrite(this->_subcribe_context, &done))
        {
            cerr << "unsubscribe command failed!" << endl;
            return false;
        }
    }
    return true;
}

// 在独立线程中接收订阅通道中的消息
void Redis::observer_channel_message()
{
    redisReply *reply = nullptr;
    while (REDIS_OK == redisGetReply(this->_subcribe_context, (void **)&reply))
    {
        // 订阅收到的消息是一个带三元素的数组
        if (reply != nullptr && reply->element[2] != nullptr && reply->element[2]->str != nullptr)
        {
            /*此处使用回调函数，是因为redis中发生了一些事情，需要将这个事情上报到业务层，如何上报呢，
            redis常规方法是在redis内部申请一个业务对象，然后调用业务对象的方法就可以操作redis的信息
            了，但是业务层和redis对象应该是解耦的，两者在此项目中属于独立关系，需要各自单独操作，此时
            当redis发生一些情况时，需要上报到业务层，就需要业务层传来一个回调函数，此处就是redis调用
            业务层回调函数的情况。通过这样的方式，redis只需要操作这个回调函数进行操作，不再需要知道业务层
            的实际对象是谁，也不需要成员内部申请业务层。*/
            // 给业务层上报通道上发生的消息
            _notify_message_handler(atoi(reply->element[1]->str), reply->element[2]->str);
        }

        freeReplyObject(reply);
    }

    cerr << ">>>>>>>>>>>>> observer_channel_message quit <<<<<<<<<<<<<" << endl;
}

void Redis::init_notify_handler(function<void(int, string)> fn)
{
    this->_notify_message_handler = fn;
}