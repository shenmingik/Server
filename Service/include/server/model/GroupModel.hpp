#ifndef GROUPMODEL_H
#define GROUPMODEL_H

#include "Group.hpp"
#include <vector>
#include <string>

using namespace std;

class GroupModel
{
public:
    //创建群组
    bool create_group(Group &group);

    //加入群组
    bool add_group(int user_id, int group_id, string role);

    //查询用户所在群组信息
    vector<Group> query_group(int user_id);

    //根据指定的groupid查询群组用户id列表，除userid自己，给该群用户群发消息
    vector<int> query_group_users(int user_id, int group_id);
};

#endif