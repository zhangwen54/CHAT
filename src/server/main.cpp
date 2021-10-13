#include <iostream>
#include <signal.h>
#include "chatservice.hpp"
#include "chatserver.hpp"

using namespace std;

//处理服务器ctrl+c结束后，重置user的状态信息
void resetHandler(int)
{
    ChatService::instance()->reset();
    exit(0);
}

int main()
{
    signal(SIGINT, resetHandler);

    EventLoop loop;
    InetAddress addr("192.168.10.183", static_cast<uint16_t>(6666));
    ChatServer server(&loop, addr, "demo");
    server.start();
    loop.loop();
    return 0;
}