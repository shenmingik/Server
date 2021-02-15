#ifndef PUBLIC_H
#define PUBLIC_H

/*
* service和client的公共文件
*/
enum EnMsgType
{
    LOGIN_MSG = 1,  //登录消息，绑定login
    LOGIN_MSG_ACK,  //登录响应消息
    REG_MSG,        //注册消息，绑定regist
    REG_MSG_ACK,    //注册响应消息
    ONE_CHAT_MSG,   //一对一聊天消息
    ADD_FRIEND_MSG, //添加好友消息

    CREATE_GROUP_MSG, //创建群聊
    ADD_GROUP_MSG,    //加入群聊
    GROUP_CHAT_MSG,   //群聊消息

    LOGINOUT_MSG,   //注销消息
};

#endif