#include <iostream>
#include "mprpcapplication.h"
#include "user.pb.h"
#include "logger.h"

int main(int argc, char **argv)
{
    // 整个程序启动以后，想使用mprpc框架来使用rpc服务调用，首先需要初始化框架（只初始化一次）
    MprpcApplication::Init(argc, argv);

    // 多态调用派生类重写的：基类RpcChannel* channel--->派生类 MprpcChannel
    // UserServiceRpc_Stub(::PROTOBUF_NAMESPACE_ID::RpcChannel* channel);
    fixbug::UserServiceRpc_Stub stub(new MprpcChannel());

    MprpcController controller;

    // rpc方法请求参数
    fixbug::LoginRequest login_request;
    login_request.set_name("zhang san");
    login_request.set_pwd("123456");
    // rpc请求的响应
    fixbug::LoginResponse login_response;
    /*  每个方法内部都是：RpcChannel->callMethod
        再在callMethod中处理所有rpc方法调用的参数的序列化以及网络发送
        阻塞等待远端返回调用的结果
    */
    // 发起rpc方法调用，同步rpc调用过程（阻塞等待）
    controller.Reset();
    stub.Login(&controller, &login_request, &login_response, nullptr);

    if (controller.Failed())
    {
        // std::cout << controller.ErrorText() << std::endl;
        LOG_ERR("[%s]", controller.ErrorText().c_str());
    }
    else
    {
        // 一次rpc调用完成
        if (login_response.ret().errcode() == 0) // 错误码为0 表示调用成功
        {
            // 查看是否登录成功
            std::cout << "RPC Login Response: " << login_response.success() << std::endl;
        }
        else
        {
            std::cout << "RPC Login Response Error: " << login_response.ret().errmsg() << std::endl;
        }
    }

    fixbug::RegisterRequest register_request;
    fixbug::RegisterResponse register_respone;
    register_request.set_name("li si");
    register_request.set_pwd("789456");

    controller.Reset();
    stub.Regster(&controller, &register_request, &register_respone, nullptr);

    if (controller.Failed())
    {
        std::cout << controller.ErrorText() << std::endl;
    }
    else
    {
        if (register_respone.ret().errcode() == 0)
        {
            std::cout << "RPC Register Response: " << register_respone.success() << std::endl;
        }
        else
        {
            std::cout << "RPC Register Response Error: " << register_respone.ret().errmsg() << std::endl;
        }
    }

    return 0;
}