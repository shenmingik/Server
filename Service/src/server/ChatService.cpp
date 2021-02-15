#include <muduo/base/Logging.h>
#include <vector>
#include <map>
#include <iostream>
#include "ChatService.hpp"
#include "public.hpp"

using namespace muduo;
using namespace std;

//获取单例对象
ChatService *ChatService::instance()
{
    static ChatService service;
    return &service;
}

//注册消息以及对应的回调操作
ChatService::ChatService()
{
    msg_handler_map_.insert({LOGIN_MSG, bind(&ChatService::login, this, _1, _2, _3)});
    msg_handler_map_.insert({LOGINOUT_MSG, bind(&ChatService::loginout, this, _1, _2, _3)});
    msg_handler_map_.insert({REG_MSG, bind(&ChatService::regist, this, _1, _2, _3)});
    msg_handler_map_.insert({ONE_CHAT_MSG, bind(&ChatService::one_chat, this, _1, _2, _3)});
    msg_handler_map_.insert({ADD_FRIEND_MSG, bind(&ChatService::add_friend, this, _1, _2, _3)});
    msg_handler_map_.insert({CREATE_GROUP_MSG, bind(&ChatService::create_group, this, _1, _2, _3)});
    msg_handler_map_.insert({ADD_GROUP_MSG, bind(&ChatService::add_group, this, _1, _2, _3)});
    msg_handler_map_.insert({GROUP_CHAT_MSG, bind(&ChatService::group_chat, this, _1, _2, _3)});

    if (redis_.connect())
    {
        redis_.init_notify_handler(bind(&ChatService::redis_subscribe_message_handler, this, _1, _2));
    }
}

//获取消息对应的处理器
MsgHandler ChatService::get_handler(int msgid)
{
    //记录错误日志，msgid没有对应的事件处理回调
    auto it = msg_handler_map_.find(msgid);
    //如果没有对应的msgid
    if (it == msg_handler_map_.end())
    {
        //返回一个默认处理器，打印错误日志
        return [=](const TcpConnectionPtr &conn, json &js, Timestamp time) {
            LOG_ERROR << "msgid: " << msgid << " can not find handler!";
        };
    }
    else
    {
        return msg_handler_map_[msgid];
    }
}

//登录
void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int id = js["id"].get<int>();
    string password = js["password"];

    User user = user_model_.query(id);
    //cout << "js:" << js << endl;

    if (user.get_id() == id && user.get_password() == password)
    {
        //用户在线
        if (user.get_state() == "online")
        {
            //用户不存在
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 2;
            response["errmsg"] = "id is online";

            conn->send(response.dump());
        }
        else
        {
            //登陆成功
            {
                //记录用户连接信息，注意线程安全
                lock_guard<mutex> lock(conn_mutex_);
                user_connection_map_.insert({id, conn});
            }

            //订阅Redis
            redis_.subscribe(id);

            //更新用户状态信息
            user.set_state("online");
            user_model_.update_state(user);

            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 0;
            response["id"] = user.get_id();
            response["name"] = user.get_name();

            //查询用户是否有离线消息
            vector<string> vec = offline_message_model_.query(id);
            //增加离线消息
            if (!vec.empty())
            {
                response["offlinemsg"] = vec;
                //读取完后删除消息
                offline_message_model_.remove(id);
            }

            //查询该用户的好友信息并返回
            vector<User> user_vec = friend_model_.query(id);
            if (!user_vec.empty())
            {
                vector<string> friend_vec;
                for (User &user : user_vec)
                {
                    json js;
                    js["id"] = user.get_id();
                    js["name"] = user.get_name();
                    js["state"] = user.get_state();
                    friend_vec.push_back(js.dump());
                }
                response["friends"] = friend_vec;
            }

            //查询该用户的群组信息并返回
            vector<Group> group_vec = group_model_.query_group(id);
            if (!group_vec.empty())
            {
                // group:[{groupid:[xxx, xxx, xxx, xxx]}]
                vector<string> groupV;
                for (Group &group : group_vec)
                {
                    json grpjson;
                    grpjson["id"] = group.get_id();
                    grpjson["groupname"] = group.get_name();
                    grpjson["groupdesc"] = group.get_desc();

                    vector<string> userV;
                    for (GroupUser &user : group.get_User())
                    {
                        json js;
                        js["id"] = user.get_id();
                        js["name"] = user.get_name();
                        js["state"] = user.get_state();
                        js["role"] = user.get_role();
                        userV.push_back(js.dump());
                    }
                    grpjson["users"] = userV;

                    groupV.push_back(grpjson.dump());
                }

                response["groups"] = groupV;
            }

            conn->send(response.dump());
        }
    }
    else
    {
        //用户不存在或密码错误
        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["errno"] = 1;
        response["errmsg"] = "id or password error";

        conn->send(response.dump());
    }
}

//注册
void ChatService::regist(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    string name = js["name"];
    string password = js["password"];

    User user;
    user.set_name(name);
    user.set_password(password);

    bool state = user_model_.insert(user);
    if (state)
    {
        //注册成功
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 0;
        response["id"] = user.get_id();

        conn->send(response.dump());
    }
    else
    {
        //注册失败
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 1;

        conn->send(response.dump());
    }
}

//处理客户端异常退出
void ChatService::client_close_exception(const TcpConnectionPtr &conn)
{
    //1.删除user_map    2.用户改为offline   3.线程安全
    User user;
    {
        lock_guard<mutex> lock(conn_mutex_);
        for (auto it = user_connection_map_.begin(); it != user_connection_map_.end(); it++)
        {
            if (it->second == conn)
            {
                user.set_id(it->first);
                user_connection_map_.erase(it);
                break;
            }
        }
    }

    //取消订阅
    redis_.unsubscribe(user.get_id());

    if (user.get_id() != -1)
    {
        user.set_state("offline");
        user_model_.update_state(user);
    }
}

//一对一聊天业务
void ChatService::one_chat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int receive_id = js["to"].get<int>();

    {
        lock_guard<mutex> lock(conn_mutex_);
        auto it = user_connection_map_.find(receive_id);
        if (it != user_connection_map_.end())
        {
            //用户在线，转发消息给receive用户
            it->second->send(js.dump());
            return;
        }
    }

    //用户在其他主机，publish消息到redis
    User user = user_model_.query(receive_id);
    if (user.get_state() == "online")
    {
        redis_.publish(receive_id, js.dump());
        return;
    }

    //  用户不在线，存储离线消息
    offline_message_model_.insert(receive_id, js.dump());
}

//添加好友业务
bool ChatService::add_friend(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int user_id = js["id"].get<int>();
    int friend_id = js["friendid"].get<int>();

    //存储好友信息
    friend_model_.insert(user_id, friend_id);
}

//创建 群组
bool ChatService::create_group(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int user_id = js["id"].get<int>();
    string name = js["groupname"];
    string desc = js["groupdesc"];

    //创建群组信息
    Group group(-1, name, desc);
    if (group_model_.create_group(group))
    {
        //存储群主信息
        if (group_model_.add_group(user_id, group.get_id(), "creator"))
        {
            return true;
        }
    }
    return false;
}

//加入群组
bool ChatService::add_group(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int user_id = js["id"].get<int>();
    int group_id = js["groupid"].get<int>();
    if (group_model_.add_group(user_id, group_id, "normal"))
    {
        return true;
    }
    return false;
}

//群聊业务
void ChatService::group_chat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int user_id = js["id"].get<int>();
    int group_id = js["groupid"].get<int>();

    vector<int> userid_vec = group_model_.query_group_users(user_id, group_id);

    lock_guard<mutex> lock(conn_mutex_);
    for (int id : userid_vec)
    {
        auto it = user_connection_map_.find(id);
        if (it != user_connection_map_.end())
        {
            //转发群消息
            it->second->send(js.dump());
        }
        else
        {
            //用户在其他服务器登录
            User user = user_model_.query(id);
            if (user.get_state() == "online")
            {
                redis_.publish(id, js.dump());
            }
            else
            {
                //转储离线消息
                offline_message_model_.insert(id, js.dump());
            }
        }
    }
}

//服务器异常，业务重置
void ChatService::reset()
{
    //把online用户设置为offline
    user_model_.reset_state();
}

//注销业务
void ChatService::loginout(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int user_id = js["id"].get<int>();

    {
        lock_guard<mutex> lock(conn_mutex_);
        auto it = user_connection_map_.find(user_id);
        if (it != user_connection_map_.end())
        {
            user_connection_map_.erase(it);
        }
    }

    //取消Redis订阅
    redis_.unsubscribe(user_id);

    // 更新用户的状态信息
    User user(user_id, "", "", "offline");
    user_model_.update_state(user);
}

//redis订阅消息触发的回调函数,这里channel其实就是id
void ChatService::redis_subscribe_message_handler(int channel, string message)
{
    //用户在线
    lock_guard<mutex> lock(conn_mutex_);
    auto it = user_connection_map_.find(channel);
    if (it != user_connection_map_.end())
    {
        it->second->send(message);
        return;
    }

    //转储离线
    offline_message_model_.insert(channel, message);
}
