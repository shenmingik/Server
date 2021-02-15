#include <iostream>
#include "UserModel.hpp"
#include "MySQL.hpp"

using namespace std;

//增加User
bool UserModel::insert(User &user)
{
    //组装SQL语句
    char sql[1024] = {0};
    sprintf(sql, "insert into User(name,password,state) values('%s','%s','%s')",
            user.get_name().c_str(), user.get_password().c_str(), user.get_state().c_str());

    MySQL mysql;
    if (mysql.connet())
    {
        if (mysql.update(sql))
        {
            //获取自增的用户ID
            user.set_id(mysql_insert_id(mysql.get_connection()));
            return true;
        }
    }
    return false;
}

//根据ID查询user信息，失败返回一个默认构造User，id = -1
User UserModel::query(int id)
{
    //组装SQL语句
    char sql[1024] = {0};
    sprintf(sql, "select * from User where id=%d", id);

    MySQL mysql;
    if (mysql.connet())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row = mysql_fetch_row(res);
            if (row != nullptr)
            {
                User user;
                user.set_id(atoi(row[0]));
                user.set_name(row[1]);
                user.set_password(row[2]);
                user.set_state(row[3]);

                //释放资源，否则内存不断泄露
                mysql_free_result(res);

                return user;
            }
        }
    }
    return User();
}

//更新状态操作
bool UserModel::update_state(User user)
{
    //组装SQL语句
    char sql[1024] = {0};
    sprintf(sql, "update User set state='%s'  where id=%d;", user.get_state().c_str(), user.get_id());
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

//重置用户状态信息
bool UserModel::reset_state()
{
     //组装SQL语句
    char sql[1024] = {0};
    sprintf(sql, "update User set state='offline'  where state='online';");
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
