#include "Redis.hpp"
#include <iostream>

Redis::Redis() : publish_context_(nullptr), subcribe_context_(nullptr)
{
}

Redis::~Redis()
{
    if (publish_context_ != nullptr)
    {
        redisFree(publish_context_);
    }

    if (subcribe_context_ != nullptr)
    {
        redisFree(subcribe_context_);
    }
}

//连接Redis服务器
bool Redis::connect()
{
    publish_context_ = redisConnect("127.0.0.1", 6379);
    if (publish_context_ == nullptr)
    {
        cerr << "connect redis failed!" << endl;
        return false;
    }

    subcribe_context_ = redisConnect("127.0.0.1", 6379);
    if (subcribe_context_ == nullptr)
    {
        cerr << "connect redis failed!" << endl;
        return false;
    }

    thread t([&]() {
        observer_channel_message();
    });
    t.detach();

    cout << "connect redis-server success!" << endl;
    return true;
}

//向Redis指定的通道channel发布消息
bool Redis::publish(int channel, string message)
{
    //相当于publish 键 值
    redisReply *reply = (redisReply *)redisCommand(publish_context_, "PUBLISH %d %s", channel, message.c_str());
    if (reply == nullptr)
    {
        cerr << "publish command failed!" << endl;
        return false;
    }

    freeReplyObject(reply);
    return true;
}

//向Redis指定的通道subscribe订阅消息
bool Redis::subscribe(int channel)
{
    //redisCommand 会先把命令缓存到context中，然后调用RedisAppendCommand发送给redis
    //redis执行subscribe是阻塞，不会响应，不会给我们一个reply
    if (REDIS_ERR == redisAppendCommand(subcribe_context_, "SUBSCRIBE %d", channel))
    {
        cerr << "subscibe command failed" << endl;
        return false;
    }

    int done = 0;
    while (!done)
    {
        if (REDIS_ERR == redisBufferWrite(subcribe_context_, &done))
        {
            cerr << "subscribe command failed" << endl;
            return false;
        }
    }

    return true;
}

//取消订阅
bool Redis::unsubscribe(int channel)
{
    //redisCommand 会先把命令缓存到context中，然后调用RedisAppendCommand发送给redis
    //redis执行subscribe是阻塞，不会响应，不会给我们一个reply
    if (REDIS_ERR == redisAppendCommand(subcribe_context_, "UNSUBSCRIBE %d", channel))
    {
        cerr << "subscibe command failed" << endl;
        return false;
    }

    int done = 0;
    while (!done)
    {
        if (REDIS_ERR == redisBufferWrite(subcribe_context_, &done))
        {
            cerr << "subscribe command failed" << endl;
            return false;
        }
    }

    return true;
}

//独立线程中接收订阅通道的消息
void Redis::observer_channel_message()
{
    redisReply *reply = nullptr;
    while (REDIS_OK == redisGetReply(subcribe_context_, (void **)&reply))
    {
        //reply里面是返回的数据有三个，0. message , 1.通道号，2.消息
        if (reply != nullptr && reply->element[2] != nullptr && reply->element[2]->str != nullptr)
        {
            //给业务层上报消息
            notify_message_handler_(atoi(reply->element[1]->str), reply->element[2]->str);
        }

        freeReplyObject(reply);
    }

    cerr << "----------------------- oberver_channel_message quit--------------------------" << endl;
}

//初始化业务层上报通道消息的回调对象
void Redis::init_notify_handler(redis_handler handler)
{
    notify_message_handler_ = handler;
}
