#ifndef GROUPUSER_H
#define GROUPUSER_H

#include "User.hpp"

class GroupUser : public User
{
public:
    void set_role(string role)
    {
        role_ = role;
    }

    string get_role()
    {
        return role_;
    }

private:
    string role_;
};
#endif