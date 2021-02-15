#include <iostream>
#include <signal.h>
#include "ChatServer.hpp"
#include "ChatService.hpp"

using namespace std;

//处理服务器ctrl+c结束后，重置user的状态信息
void reset_handler(int)
{
    ChatService::instance()->reset();
    exit(0);
}

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        cerr << "command invaild example: ./ExeNAME  IpAddress  port" << endl;
        exit(-1);
    }

    //解析IP地址和端口号
    char *ip = argv[1];
    uint16_t port = atoi(argv[2]);

    signal(SIGINT, reset_handler);

    EventLoop loop;
    InetAddress addr(ip, port);
    ChatServer server(&loop, addr, "ChatServer");

    server.start();
    loop.loop();

    return 0;
}