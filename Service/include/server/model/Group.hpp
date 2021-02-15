#ifndef GROUP_H
#define GROUP_H

#include "GroupUser.hpp"
#include <string>
#include <vector>

using namespace std;

//Group表的ORM类
class Group
{
public:
    Group(int id = -1, string name = "", string desc = "")
    {
        id_ = id;
        name_ = name;
        desc_ = desc;
    }

    void set_id(int id)
    {
        id_ = id;
    }

    void set_name(string name)
    {
        name_ = name;
    }

    void set_desc(string desc)
    {
        desc_ = desc;
    }

    int get_id()
    {
        return id_;
    }

    string get_name()
    {
        return name_;
    }

    string get_desc()
    {
        return desc_;
    }

    vector<GroupUser> &get_User()
    {
        return user_;
    }

private:
    int id_;
    string name_;
    string desc_;
    vector<GroupUser> user_;
};

#endif