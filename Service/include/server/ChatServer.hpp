#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>

using namespace muduo;
using namespace muduo::net;

//聊天服务器主类
class ChatServer
{
public:
    //初始化聊天服务器对象
    ChatServer(EventLoop *loop, const InetAddress &listenAddr, const string &nameArg);

    //启动服务
    void start();

private:
    //上报连接相关信息的回调函数
    void on_connection(const TcpConnectionPtr &);

    //上报读写时间相关信息的回调函数
    void on_message(const TcpConnectionPtr &, Buffer *, Timestamp);

private:
    TcpServer server_; //组合的muduo库，实现服务器功能的类对象
    EventLoop *loop_;  //指向事件循环对象的指针
};

#endif