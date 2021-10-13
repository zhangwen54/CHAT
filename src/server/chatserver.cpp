/*
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
using namespace muduo;
using namespace muduo::net;
*/
#include <functional>
#include <string>
#include "chatserver.hpp"
#include "json.hpp"
#include "chatservice.hpp"

using namespace std;
using json = nlohmann::json;

ChatServer::ChatServer(EventLoop *loop,
                       const InetAddress &listenAddr,
                       const string &nameArg)
    : _server(loop, listenAddr, nameArg), _loop(loop)
{
    _server.setConnectionCallback(std::bind(&ChatServer::oneConnection, this, std::placeholders::_1));
    _server.setMessageCallback(std::bind(&ChatServer::oneMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    _server.setThreadNum(4);
}
void ChatServer::start() { _server.start(); }

//上报链接相关信息的回调函数
void ChatServer::oneConnection(const TcpConnectionPtr &conn)
{
    //客户端断开连接
    if (!conn->connected())
    {
        ChatService::instance()->clientCloseException(conn);
        conn->shutdown();
    }
}
//上报读写事件相关信息的回调函数
void ChatServer::oneMessage(const TcpConnectionPtr &conn,
                            Buffer *buffer,
                            Timestamp time)
{
    string buf = buffer->retrieveAllAsString();
    // 测试，添加json打印代码
    //cout<<buf<<endl;
    // 数据的反序列化
    json js = json::parse(buf);
    // 达到的目的：完全解耦网络模块的代码和业务模块的代码
    // 通过js["msgid"] 获取=》业务handler=》conn  js  time
    auto msgHandler = ChatService::instance()->getHandler(js["msgid"].get<int>());
    // 回调消息绑定好的事件处理器，来执行相应的业务处理
    msgHandler(conn, js, time);
}
