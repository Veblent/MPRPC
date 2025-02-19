#include <iostream>
#include <string>
#include "friend.pb.h"
#include "mprpcapplication.h"
#include "rpcprovider.h"
#include "logger.h"
#include <vector>

class FriendService : public fixbug::FriendServiceRpc
{
public:
    std::vector<std::string> GetFriendList(uint32_t userId)
    {
        std::vector<std::string> friendVec;
        std::cout << "do GetFriendList Service! userID = " << userId << std::endl;
        friendVec.push_back("user 01");
        friendVec.push_back("user 02");
        friendVec.push_back("user 03");
        friendVec.push_back("user 04");
        friendVec.push_back("user 05");

        return friendVec;
    }

    // 重写基类方法
    void GetFriendList(::google::protobuf::RpcController *controller,
                       const ::fixbug::GetFriendListReuqset *request,
                       ::fixbug::GetFriendListResponse *response,
                       ::google::protobuf::Closure *done)
    {
        uint32_t userId = request->userid();

        std::vector<std::string> friendVec =  GetFriendList(userId);
        response->mutable_ret()->set_errcode(0);
        response->mutable_ret()->set_errmsg("");
        
        for(std::string &friendName : friendVec)
        {
            std::string *p = response->add_friendslist();
            *p = friendName;
        }

        done->Run();
    }
};


int main(int argc, char **argv)
{
    // LOG_INFO("first log message!");
    // LOG_ERR("%s:%s:%d", __FILE__, __FUNCTION__, __LINE__);

    // 调用框架的初始化操作 provider -i config.conf
    MprpcApplication::Init(argc, argv);

    // provider是一个rpc网络服务对象 把UserService对象发布到rpc节点上
    // 数据的序列化/反序列化、网络数据收发
    RpcProvider provider;
    provider.NotifyService(new FriendService());

    // 启动一个rpc服务发布节点，Run以后，进程进入阻塞状态，等待远程的rpc调用请求
    provider.Run();

    return 0;
}