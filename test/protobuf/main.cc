#include "test.pb.h"
#include <iostream>


void test1()
{
    // 封装login请求对象的数据
    fixbug::LoginRequest req;
    req.set_name("zhangsan");
    req.set_pwd("123456");

    // 对象数据序列化成char *
    std::string send_str;
    if(req.SerializeToString(&send_str))
    {
        std::cout << send_str.c_str() << std::endl;
    }

    // 从send_str反序列化一个login请求对象
    fixbug::LoginRequest reqB;
    if(reqB.ParseFromString(send_str))
    {
        std::cout << reqB.name() << std::endl;
        std::cout << reqB.pwd() << std::endl;
    }
}


void test2()
{
    fixbug::LoginResponse rsp;
    fixbug::ResultCode *rc = rsp.mutable_result();
    rc->set_errcode(1);
    rc->set_errmsg("登录处理失败！");
    rsp.set_success(false);

    std::string send_str;
    if(rsp.SerializeToString(&send_str))
    {
        std::cout << send_str.c_str() << std::endl;
    }

    fixbug::LoginResponse rspB;
    if(rspB.ParseFromString(send_str))
    {
        std::cout << rspB.result().errcode() << std::endl;
        std::cout << rspB.result().errmsg() << std::endl;
        if(rspB.success())
        {
            std::cout << "成功！" << std::endl;
        }
        else
        {
            std::cout << "失败！" << std::endl;
        }
    }
}


void test3()
{
    fixbug::GetFriendListsResponse rsp;
    fixbug::ResultCode *rc = rsp.mutable_result();
    rc->set_errcode(0);

    fixbug::User *user;
    user = rsp.add_friendlist();
    user->set_name("zhang san");
    user->set_age(18);
    user->set_sex(fixbug::User::M);

    user = rsp.add_friendlist();
    user->set_name("li si");
    user->set_age(21);
    user->set_sex(fixbug::User::F);

    user = rsp.add_friendlist();
    user->set_name("wang wu");
    user->set_age(19);
    user->set_sex(fixbug::User::M);

    // std::cout << rsp.friendlist_size() << std::endl;     // 3
    // std::cout << rsp.friendlist(0).name() << std::endl;
    // std::cout << rsp.friendlist(1).name() << std::endl;
    // std::cout << rsp.friendlist(2).name() << std::endl;

    // 序列化
    std::string send_str;
    if(rsp.SerializeToString(&send_str))
    {
        /*......*/
    }

    // 反序列化
    fixbug::GetFriendListsResponse rspB;
    std::cout << rspB.ParseFromString(send_str) << std::endl;
    std::cout << rspB.friendlist_size() << std::endl;
    std::cout << rspB.friendlist(0).name() << std::endl;
    std::cout << rspB.friendlist(1).name() << std::endl;
    std::cout << rspB.friendlist(2).name() << std::endl;
}

int main()
{
    // test1();
    // test2();
    test3();

    return 0;
}

// g++ main.cc test.pb.cc -lprotobuf -o demo