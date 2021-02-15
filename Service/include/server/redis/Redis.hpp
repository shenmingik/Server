#ifndef REDIS_H
#define REDIS_H

#include <hiredis/hiredis.h>
#include <thread>
#include <functional>

using namespace std;
using redis_handler = function<void(int,string)>;

class Redis
{
public:
    Redis();
    ~Redis();

    //连接Redis服务器
    bool connect();

    //向Redis指定的通道channel发布消息
    bool publish(int channel, string message);

    //向Redis指定的通道subscribe订阅消息
    bool subscribe(int channel);

    //取消订阅
    bool unsubscribe(int channel);

    //独立线程中接收订阅通道的消息
    void observer_channel_message();

    //初始化业务层上报通道消息的回调对象
    void init_notify_handler(redis_handler handler);

private:
    //hiredis同步上下文对象，负责publish消息
    redisContext *publish_context_;

    //负责subscribe消息
    redisContext *subcribe_context_;

    //回调操作，收到消息给service上报
    redis_handler notify_message_handler_;
};

#endif