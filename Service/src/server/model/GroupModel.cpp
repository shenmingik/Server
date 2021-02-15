#include "GroupModel.hpp"
#include "MySQL.hpp"
#include<iostream>
using namespace std;

//创建群组
bool GroupModel::create_group(Group &group)
{
    //组装SQL语句
    char sql[1024] = {0};
    sprintf(sql, "insert into AllGroup(groupname,groupdesc) values('%s','%s');", group.get_name().c_str(), group.get_desc().c_str());
    //cout << sql << endl;

    MySQL mysql;
    if (mysql.connet())
    {
        if (mysql.update(sql))
        {
            group.set_id(mysql_insert_id(mysql.get_connection()));
            return true;
        }
    }
    return false;
}

//加入群组
bool GroupModel::add_group(int user_id, int group_id, string role)
{
    //组装SQL语句
    char sql[1024] = {0};
    sprintf(sql, "insert into GroupUser values(%d,%d,'%s');", group_id, user_id, role.c_str());
    cout << sql << endl;

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

//查询用户所在群组信息
vector<Group> GroupModel::query_group(int user_id)
{
    //组装SQL语句
    char sql[1024] = {0};
    sprintf(sql, "select  a.id,a.groupname,a.groupdesc from AllGroup a inner join GroupUser b on b.groupid=a.id  where b.userid=%d;", user_id);
    //cout << sql << endl;

    vector<Group> group_vec;
    MySQL mysql;
    if (mysql.connet())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                Group group;
                group.set_id(atoi(row[0]));
                group.set_name(row[1]);
                group.set_desc(row[2]);
                group_vec.push_back(group);
            }
            //释放资源，否则内存不断泄露
            mysql_free_result(res);
        }
    }

    //查询群组所有群员的信息
    for (Group &temp : group_vec)
    {
        sprintf(sql, "select  a.id,a.name,a.state,b.grouprole from User a inner join GroupUser b on b.userid=a.id  where b.groupid=%d;", temp.get_id());

        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                GroupUser group_user;
                group_user.set_id(atoi(row[0]));
                group_user.set_name(row[1]);
                group_user.set_state(row[2]);
                group_user.set_role(row[3]);
                temp.get_User().push_back(group_user);
            }
            //释放资源，否则内存不断泄露
            mysql_free_result(res);
        }
        return group_vec;
    }
}

//根据指定的groupid查询群组用户id列表，除userid自己，给该群用户群发消息
vector<int> GroupModel::query_group_users(int user_id, int group_id)
{
    //组装SQL语句
    char sql[1024] = {0};
    sprintf(sql, "select userid  from GroupUser where groupid=%d and userid != %d;", group_id, user_id);
    //cout << sql << endl;

    vector<int> id_vec;
    MySQL mysql;
    if (mysql.connet())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                id_vec.push_back(atoi(row[0]));
            }
            //释放资源，否则内存不断泄露
            mysql_free_result(res);
        }
    }
    return id_vec;
}
