#ifndef OFFLINEMESSAGEMODEL_H
#define OFFLINEMESSAGEMODEL_H

#include <string>
#include <vector>

using namespace std;

//关于offlinemessage表的操作
class OfflineMessageModel
{
public:
    //存储用户的离线消息
    bool insert(int id, string msg);

    //删除用户的离线消息
    bool remove(int id);

    //查询用户的聊天消息
    vector<string> query(int id);
};
#endif