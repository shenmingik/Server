#ifndef FRIENDMODEL_H
#define FRIENDMODEL_H

#include <vector>
#include "User.hpp"

using namespace std;

//维护好友信息的结构方法
class FriendModel
{
public:
    //添加好友关系
    bool insert(int user_id, int friend_id);

    //返回用户好友列表
    vector<User> query(int user_id);
};

#endif