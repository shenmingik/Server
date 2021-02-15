#include "OfflineMessageModel.hpp"
#include "MySQL.hpp"

//存储用户的离线消息
bool OfflineMessageModel::insert(int id, string msg)
{
    //组装SQL语句
    char sql[1024] = {0};
    sprintf(sql, "insert into OfflineMessage values(%d,'%s')", id, msg.c_str());

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

//删除用户的离线消息
bool OfflineMessageModel::remove(int id)
{
    //组装SQL语句
    char sql[1024] = {0};
    sprintf(sql, "delete  from OfflineMessage where userid=%d", id);

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

//查询用户的聊天消息
vector<string> OfflineMessageModel::query(int id)
{
    //组装SQL语句
    char sql[1024] = {0};
    sprintf(sql, "select message from OfflineMessage where userid=%d", id);

    vector<string> vec;
    MySQL mysql;
    if (mysql.connet())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                vec.push_back(row[0]);
            }
            //释放资源，否则内存不断泄露
            mysql_free_result(res);
        }
    }
    return vec;
}

