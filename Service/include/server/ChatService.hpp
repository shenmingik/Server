#ifndef CHATSERVICE_H
#define CHATSERVICE_H

#include <unordered_map>
#include <functional>
#include <mutex>
#include <muduo/net/TcpConnection.h>
#include "json.hpp"
#include "UserModel.hpp"
#include "OfflineMessageModel.hpp"
#include "FriendModel.hpp"
#include "GroupModel.hpp"
#include "Redis.hpp"

using namespace std;
using namespace muduo;
using namespace muduo::net;
using namespace placeholders;

using json = nlohmann::json;
using MsgHandler = function<void(const TcpConnectionPtr &conn, json &js, Timestamp time)>;

//聊天服务器业务类，单例模式设计，因为一个就够了。
//映射事件回调用

class ChatService
{
public:
    //获取单例对象
    static ChatService *instance();

    //登录
    void login(const TcpConnectionPtr &conn, json &js, Timestamp time);

    //注册
    void regist(const TcpConnectionPtr &conn, json &js, Timestamp time);

    //一对一聊天业务
    void one_chat(const TcpConnectionPtr &conn, json &js, Timestamp time);

    //添加好友业务
    bool add_friend(const TcpConnectionPtr &conn, json &js, Timestamp time);

    //创建 群组
    bool create_group(const TcpConnectionPtr &conn, json &js, Timestamp time);

    //加入群组
    bool add_group(const TcpConnectionPtr &conn, json &js, Timestamp time);

    //群聊业务
    void group_chat(const TcpConnectionPtr &conn, json &js, Timestamp time);

    //注销业务
    void loginout(const TcpConnectionPtr &conn, json &js, Timestamp time);

    //redis订阅消息触发的回调函数
    void redis_subscribe_message_handler(int channel, string message);

    //获取消息对应的处理器
    MsgHandler get_handler(int msgid);

    //处理客户端异常退出
    void client_close_exception(const TcpConnectionPtr &conn);

    //服务器异常，业务重置
    void reset();

private:
    //注册消息以及对应的回调操作
    ChatService();

private:
    //存储事件触发的回调函数
    unordered_map<int, MsgHandler> msg_handler_map_;

    //存储在线用户的连接情况，便于服务器给用户发消息，注意线程安全
    unordered_map<int, TcpConnectionPtr> user_connection_map_;
    mutex conn_mutex_;

    //redis操作对象
    Redis redis_;

    UserModel user_model_;
    OfflineMessageModel offline_message_model_;
    FriendModel friend_model_;
    GroupModel group_model_;
};

#endif