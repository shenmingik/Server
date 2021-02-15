#include <iostream>
#include <thread>
#include <string>
#include <vector>
#include <chrono>
#include <ctime>

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "json.hpp"

#include "Group.hpp"
#include "User.hpp"
#include "public.hpp"

using namespace std;
using json = nlohmann::json;

#define PASSWOED_LENGTH 50
#define NAME_LENGTH 50
#define BUFFER_SIZE 1024

//记录当前系统登录的用户信息
User g_current_user;
//记录当前登录用户的好友列表信息
vector<User> g_current_friends_list;
//记录当前登录用户的群组列表信息
vector<Group> g_current_group_list;

//显示当前登录成功用户的基本信息
void ShowCurrentUserData();
//接收用户收到消息的线程
void ReadTaskHandler(int client_fd);
//获取系统时间
string GetCurrentTime();
//主聊天页面程序
void MainMenu(int client_fd);

// "help" command handler
void Help(int fd = 0, string str = "");
// "chat" command handler
void Chat(int, string);
// "addfriend" command handler
void AddFriend(int, string);
// "creategroup" command handler
void CreateGroup(int, string);
// "addgroup" command handler
void AddGroup(int, string);
// "groupchat" command handler
void GroupChat(int, string);
// "loginout" command handler
void LoginOut(int, string);

//控制主菜单是否继续显示
bool g_is_menu_running = false;
// 系统支持的客户端命令列表
unordered_map<string, string> command_map = {
    {"help", "显示所有支持的命令，格式help"},
    {"chat", "一对一聊天，格式chat:friendid:message"},
    {"addfriend", "添加好友，格式addfriend:friendid"},
    {"creategroup", "创建群组，格式creategroup:groupname:groupdesc"},
    {"addgroup", "加入群组，格式addgroup:groupid"},
    {"groupchat", "群聊，格式groupchat:groupid:message"},
    {"loginout", "注销，格式loginout"}};

// 注册系统支持的客户端命令处理
unordered_map<string, function<void(int, string)>> command_handler_map = {
    {"help", Help},
    {"chat", Chat},
    {"addfriend", AddFriend},
    {"creategroup", CreateGroup},
    {"addgroup", AddGroup},
    {"groupchat", GroupChat},
    {"loginout", LoginOut}};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//main函数主要获取用户的输入，接收线程则是将用户收到的信息打印出来
//集成登录、注册功能
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

    //创建socket
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientfd == -1)
    {
        cerr << "create socket error" << endl;
        exit(-1);
    }

    //录入连接server服务器信息
    sockaddr_in server;
    memset(&server, 0, sizeof(sockaddr_in));

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(ip);

    //server与clientfd连接
    if (-1 == connect(clientfd, (sockaddr *)&server, sizeof(sockaddr_in)))
    {
        cout << "connect error" << endl;
        close(clientfd);
        exit(-1);
    }

    //主业务
    for (;;)
    {
        //显示首页面菜单 登录，注册，退出
        cout << "**********welcome**********" << endl;
        cout << "           1.  login" << endl;
        cout << "           2.  register" << endl;
        cout << "           3.  quit " << endl;
        cout << "please input your choice:";
        int choice = 0;
        cin >> choice;
        //处理读入 缓冲区的回车
        cin.get();
        switch (choice)
        {
        case 1:
        {
            int id = 0;
            char pwd[PASSWOED_LENGTH] = {0};
            cout << "please input id:";
            cin >> id;
            cin.get();

            cout << "please input password:";
            cin.getline(pwd, PASSWOED_LENGTH);

            json js;
            js["msgid"] = LOGIN_MSG;
            js["id"] = id;
            js["password"] = pwd;
            string request = js.dump();

            //发送
            int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
            if (len == -1)
            {
                cerr << "send reg msg error" << endl;
            }
            else
            {
                //接收服务器端反馈
                char buffer[BUFFER_SIZE] = {0};
                len = recv(clientfd, buffer, BUFFER_SIZE, 0);
                if (len == -1)
                {
                    cerr << "recv login response error" << endl;
                }
                else
                {
                    //反序列化
                    json response_js = json::parse(buffer);
                    //登录失败
                    if (response_js["errno"].get<int>() != 0)
                    {
                        cerr << response_js["errmsg"] << endl;
                    }
                    else
                    {
                        //登陆成功记录当前用户信息、好友信息、群组信息、离线消息
                        //记录当前用户信息
                        g_current_user.set_id(response_js["id"].get<int>());
                        g_current_user.set_name(response_js["name"]);

                        //记录当前用户的好友信息
                        if (response_js.contains("friends"))
                        {
                            vector<string> vec = response_js["friends"];
                            for (string &str : vec)
                            {
                                json friend_js = json::parse(str);
                                User user;
                                user.set_id(friend_js["id"].get<int>());
                                user.set_name(friend_js["name"]);
                                user.set_state(friend_js["state"]);
                                g_current_friends_list.push_back(user);
                            }
                        }

                        //群组信息
                        if (response_js.contains("groups"))
                        {
                            vector<string> vec = response_js["groups"];
                            for (string &str : vec)
                            {
                                json group_js = json::parse(str);
                                Group group;
                                group.set_id(group_js["id"].get<int>());
                                group.set_name(group_js["groupname"]);
                                group.set_desc(group_js["groupdesc"]);

                                vector<string> vec2 = group_js["users"];
                                for (string &user_str : vec2)
                                {
                                    GroupUser group_user;
                                    json group_user_js = json::parse(user_str);
                                    group_user.set_id(group_user_js["id"].get<int>());
                                    group_user.set_name(group_user_js["name"]);
                                    group_user.set_state(group_user_js["state"]);
                                    group_user.set_role(group_user_js["role"]);

                                    group.get_User().push_back(group_user);
                                }
                                g_current_group_list.push_back(group);
                            }
                        }

                        //显示登录用户的基本信息
                        ShowCurrentUserData();

                        //显示用户的离线消息
                        if (response_js.contains("offlinemsg"))
                        {
                            vector<string> vec = response_js["offlinemsg"];
                            for (string &str : vec)
                            {
                                json js = json::parse(str);
                                cout << js["time"].get<string>() << "[" << js["id"] << "]" << js["name"].get<string>() << " said: " << js["msg"].get<string>() << endl;
                            }
                        }

                        //登录成功
                        thread read_task(ReadTaskHandler, clientfd);
                        read_task.detach();

                        //进入主界面
                        g_is_menu_running = true;
                        MainMenu(clientfd);
                    }
                }
            }
        }
        break;
        case 2:
        {
            //注册
            char name[NAME_LENGTH] = {0};
            char pwd[PASSWOED_LENGTH] = {0};
            cout << "user name:" << endl;
            cin.getline(name, NAME_LENGTH);

            cout << "password:" << endl;
            cin.getline(pwd, PASSWOED_LENGTH);

            //序列化
            json js;
            js["msgid"] = REG_MSG;
            js["name"] = name;
            js["password"] = pwd;
            string request = js.dump();

            //发送
            int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
            if (len == -1)
            {
                cerr << "send reg msg error" << endl;
            }
            else
            {
                //接收服务器端反馈
                char buffer[BUFFER_SIZE] = {0};
                len = recv(clientfd, buffer, BUFFER_SIZE, 0);
                if (len == -1)
                {
                    cerr << "recv register response error" << endl;
                }
                else
                {
                    cout << "lens: " << len << "buffer :" << buffer << endl;
                    //反序列化
                    json response_js = json::parse(buffer);
                    //注册失败
                    if (response_js["errno"].get<int>() != 0)
                    {
                        cerr << name << " is already exist,register error!" << endl;
                    }
                    else
                    {
                        //注册成功
                        cout << name << "register success , and user id: " << response_js["id"] << " , please remeber it!!!" << endl;
                    }
                }
            }
        }
        break;
        case 3:
        {
            //退出
            close(clientfd);
            exit(0);
        }
        default:
            cerr << "invaild input!" << endl;
            break;
        }
    }
}

//显示当前登录成功用户的基本信息
void ShowCurrentUserData()
{
    cout << "--------------------login user--------------------" << endl;
    cout << "current login uer => id: " << g_current_user.get_id() << " name: " << g_current_user.get_name() << endl;
    cout << "-------------------friend  list-------------------" << endl;
    if (!g_current_friends_list.empty())
    {
        for (User &user : g_current_friends_list)
        {
            cout << user.get_id() << " " << user.get_name() << " " << user.get_state() << endl;
        }
    }
    cout << "--------------------group list--------------------" << endl;
    if (!g_current_group_list.empty())
    {
        for (Group &group : g_current_group_list)
        {
            cout << group.get_id() << " " << group.get_name() << " " << group.get_desc() << endl;

            //打印群员信息
            cout << "========group user========" << endl;
            for (GroupUser &group_user : group.get_User())
            {
                cout << group_user.get_id() << " " << group_user.get_name() << " " << group_user.get_state() << " " << group_user.get_role() << endl;
            }
        }
    }
    cout << "--------------------------------------------------" << endl;
}

//接收用户收到消息的线程
void ReadTaskHandler(int client_fd)
{
    for (;;)
    {
        char buffer[BUFFER_SIZE] = {0};
        int len = recv(client_fd, buffer, BUFFER_SIZE, 0);
        if (len == -1 || len == 0)
        {
            close(client_fd);
            exit(-1);
        }

        json js = json::parse(buffer);
        //一对一聊天
        if (js["msgid"].get<int>() == ONE_CHAT_MSG)
        {
            cout << js["time"].get<string>() << "[" << js["id"] << "]" << js["name"].get<string>() << " said: " << js["msg"].get<string>() << endl;
            continue;
        }
        else if (js["msgid"].get<int>() == GROUP_CHAT_MSG)
        {
            cout << "group msg: [" << js["groupid"] << "]";
            cout << js["time"].get<string>() << "[" << js["id"] << "]" << js["name"].get<string>() << " said: " << js["msg"].get<string>() << endl;
            continue;
        }
    }
}

//获取系统时间
string GetCurrentTime()
{
    auto tt = chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    struct tm *ptm = localtime(&tt);
    char date[60] = {0};
    sprintf(date, "%d-%02d-%02d %02d:%02d:%02d",
            (int)ptm->tm_year + 1900, (int)ptm->tm_mon + 1, (int)ptm->tm_mday,
            (int)ptm->tm_hour, (int)ptm->tm_min, (int)ptm->tm_sec);
    return string(date);
}

//主聊天页面程序，开闭原则
void MainMenu(int clientfd)
{
    Help();
    char buffer[BUFFER_SIZE] = {0};
    while (g_is_menu_running)
    {
        cin.getline(buffer, BUFFER_SIZE);
        string command_buf(buffer);
        //存储命令
        string command;
        int index = command_buf.find(":");
        if (index == -1)
        {
            //help或者loginout
            command = command_buf;
        }
        else
        {
            //其他命令
            command = command_buf.substr(0, index);
        }

        auto it = command_handler_map.find(command);
        if (it == command_handler_map.end())
        {
            cerr << "invaild input command" << endl;
            continue;
        }

        //调用命令
        it->second(clientfd, command_buf.substr(index + 1, command_buf.size() - index));
    }
}

//打印系统支持的所有命令
void Help(int, string)
{
    cout << "--------command list--------" << endl;
    for (auto &it : command_map)
    {
        cout << it.first << " : " << it.second << endl;
    }
    cout << endl;
}

//一对一聊天
void Chat(int clientfd, string str)
{
    int index = str.find(":");
    if (index == -1)
    {
        cerr << "chat command invaild" << endl;
    }

    int friend_id = atoi(str.substr(0, index).c_str());
    string message = str.substr(index + 1, str.size() - index);

    json js;
    js["msgid"] = ONE_CHAT_MSG;
    js["id"] = g_current_user.get_id();
    js["name"] = g_current_user.get_name();
    js["to"] = friend_id;
    js["msg"] = message;
    js["time"] = GetCurrentTime();

    string request = js.dump();

    int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
    if (len == -1)
    {
        cerr << "send chat msg error" << endl;
    }
}

//添加好友
void AddFriend(int clientfd, string str)
{
    int friend_id = atoi(str.c_str());

    json js;
    js["msgid"] = ADD_FRIEND_MSG;
    js["id"] = g_current_user.get_id();
    js["friendid"] = friend_id;

    string request = js.dump();

    int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
    if (len == -1)
    {
        cerr << "send addfriend msg error" << endl;
    }
}

//创建群聊
void CreateGroup(int clientfd, string str)
{
    int index = str.find(":");
    if (index == -1)
    {
        cerr << "creategroup command invalid!" << endl;
        return;
    }

    string group_name = str.substr(0, index);
    string group_desc = str.substr(index + 1, str.size() - index);

    json js;
    js["msgid"] = CREATE_GROUP_MSG;
    js["id"] = g_current_user.get_id();
    js["groupname"] = group_name;
    js["groupdesc"] = group_desc;

    string request = js.dump();

    int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
    if (len == -1)
    {
        cerr << "send creategroup msg error" << endl;
    }
}

//加入群聊
void AddGroup(int clientfd, string str)
{
    int group_id = atoi(str.c_str());

    json js;
    js["msgid"] = ADD_GROUP_MSG;
    js["id"] = g_current_user.get_id();
    js["groupid"] = group_id;

    string request = js.dump();

    int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
    if (len == -1)
    {
        cerr << "send addgroup msg error" << endl;
    }
}

//群聊消息
void GroupChat(int clientfd, string str)
{
    int index = str.find(":");
    if (index == -1)
    {
        cerr << "creategroup command invalid!" << endl;
        return;
    }

    int group_id = atoi(str.substr(0, index).c_str());
    string message = str.substr(index + 1, str.size() - index);

    json js;
    js["msgid"] = GROUP_CHAT_MSG;
    js["id"] = g_current_user.get_id();
    js["name"] = g_current_user.get_name();
    js["groupid"] = group_id;
    js["msg"] = message;
    js["time"] = GetCurrentTime();

    string request = js.dump();

    int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
    if (len == -1)
    {
        cerr << "send grouochat msg error" << endl;
    }
}

//注销
void LoginOut(int clientfd, string)
{
    json js;
    js["msgid"] = LOGINOUT_MSG;
    js["id"] = g_current_user.get_id();
    string buffer = js.dump();

    string request = js.dump();

    int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
    if (len == -1)
    {
        cerr << "send grouochat msg error" << endl;
    }
    else
    {
        g_is_menu_running = false;
        g_current_friends_list.clear();
        g_current_group_list.clear();
    }
}
