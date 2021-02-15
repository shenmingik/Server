#ifndef USER_H
#define USER_H

#include <string>
using namespace std;

class User
{
public:
    User(int id = -1, string name = "", string password = "", string state = "offline")
    {
        id_ = id;
        name_ = name;
        password_ = password;
        state_ = state;
    }

    void set_id(int id)
    {
        id_ = id;
    }

    void set_name(string name)
    {
        name_ = name;
    }

    void set_password(string password)
    {
        password_ = password;
    }

    void set_state(string state)
    {
        state_ = state;
    }

    int get_id()
    {
        return id_;
    }

    string get_name()
    {
        return name_;
    }

    string get_password()
    {
        return password_;
    }

    string get_state()
    {
        return state_;
    }

private:
    int id_;
    string name_;
    string password_;
    string state_;
};
#endif