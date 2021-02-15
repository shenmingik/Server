#include "FriendModel.hpp"
#include "MySQL.hpp"

//添加好友关系
bool FriendModel::insert(int user_id, int friend_id)
{
    //组装SQL语句
    char sql[1024] = {0};
    sprintf(sql, "insert into Friend values(%d,%d);", user_id, friend_id);
    //cout << sql << endl;

    MySQL mysql;
    if (mysql.connet())
    {
        if (mysql.update(sql))
        {
            return true;
        }
    }
    return false;
}

//返回用户好友列表
vector<User> FriendModel::query(int user_id)
{
    //组装SQL语句
    char sql[1024] = {0};
    sprintf(sql, "select  a.id,a.name,a.state from User a inner join Friend b on b.friendid = a.id  where b.userid=%d;", user_id);
    //cout << sql << endl;

    vector<User> vec;
    MySQL mysql;
    if (mysql.connet())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                User user;
                user.set_id(atoi(row[0]));
                user.set_name(row[1]);
                user.set_state(row[2]);
                vec.push_back(user);
            }
            //释放资源，否则内存不断泄露
            mysql_free_result(res);
        }
    }
    return vec;
}