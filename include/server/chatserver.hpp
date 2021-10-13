#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
using namespace muduo;
using namespace muduo::net;

class ChatServer
{
    public:
    //初始化聊天服务器对象
    ChatServer(EventLoop* loop,
            const InetAddress& listenAddr,
            const string& nameArg);
    void start();
    private:
    //上报链接相关信息的回调函数
    void oneConnection(const TcpConnectionPtr& conn);
    //上报读写事件相关信息的回调函数
    void oneMessage(const TcpConnectionPtr& conn,
                            Buffer* buffer,
                            Timestamp time);    
    TcpServer _server;//组合的muduo库，实现服务器功能的类对象
    EventLoop *_loop;//指向事件循环对象的指向
};


#endif