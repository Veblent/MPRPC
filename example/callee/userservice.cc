#include <iostream>
#include <string>
#include "user.pb.h"
#include "mprpcapplication.h"
#include "rpcprovider.h"

/*
UserService原来是一个本地服务，
提供了两个进程内的本地方法：
Login 和 GetFriendLists
*/

// callee
class UserService : public fixbug::UserServiceRpc // rpc服务提供者
{
public:
    bool Login(std::string name, std::string pwd)
    {
        std::cout << "doing local service Login" << std::endl;
        std::cout << "name : " << name << " pwd : " << pwd << std::endl;

        return true;
    }

    bool Register(std::string name, std::string pwd)
    {
        std::cout << "doing local service Register" << std::endl;
        std::cout << "name : " << name << " pwd : " << pwd << std::endl;

        return true;
    }
    /*
        重写基类UserServiceRpc的虚函数 下面这些方法都是框架直接调用
        1. caller   ===>  Login(LoginRequest)   => muduo => callee
        2. callee接收到caller调用的Login(LoginRequest)方法后，交给我们下面重写的Login方法
    */
    void Login(::google::protobuf::RpcController *controller,
               const ::fixbug::LoginRequest *request,
               ::fixbug::LoginResponse *response,
               ::google::protobuf::Closure *done)
    {
        // 框架给业务上报了请求参数LoginRequest，应用获取相应数据做本地业务
        std::string name = request->name();
        std::string pwd = request->pwd();

        // 根据caller端发送的参数，调用本地业务
        bool ret = Login(name, pwd);

        // 根据本地业务执行结果，将errcode errmsg success写入response
        response->mutable_ret()->set_errcode(0);
        response->mutable_ret()->set_errmsg("No Error");
        response->set_success(ret);

        // 执行回调：执行response对象的序列化以及网络发送（由框架完成）
        done->Run();
    }

    void Regster(::google::protobuf::RpcController *controller,
                 const ::fixbug::RegisterRequest *request,
                 ::fixbug::RegisterResponse *response,
                 ::google::protobuf::Closure *done)
    {
        std::string name = request->name();
        std::string pwd = request->pwd();

        bool ret = Register(name, pwd);
        response->mutable_ret()->set_errcode(0);
        response->mutable_ret()->set_errmsg("");
        response->set_success(true);

        done->Run();
    }
};

int main(int argc, char **argv)
{
    // 调用框架的初始化操作 provider -i config.conf
    MprpcApplication::Init(argc, argv);

    // provider是一个rpc网络服务对象 把UserService对象发布到rpc节点上
    // 数据的序列化/反序列化、网络数据收发
    RpcProvider provider;
    provider.NotifyService(new UserService());

    // 启动一个rpc服务发布节点，Run以后，进程进入阻塞状态，等待远程的rpc调用请求
    provider.Run();

    return 0;
}