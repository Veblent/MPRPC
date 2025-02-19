#include <iostream>
#include "mprpcapplication.h"
#include "friend.pb.h"

int main(int argc, char **argv)
{
    MprpcApplication::Init(argc, argv);

    fixbug::FriendServiceRpc_Stub stub(new MprpcChannel());

    MprpcController controller;
    fixbug::GetFriendListReuqset request;
    fixbug::GetFriendListResponse response;
    request.set_userid(123);
    controller.Reset();
    stub.GetFriendList(&controller, &request, &response, nullptr);

    if (controller.Failed())
    {
        std::cout << controller.ErrorText() << std::endl;
    }
    else
    {
        if (response.ret().errcode() == 0)
        {
            int num = response.friendslist_size();
            for (int i = 0; i < num; i++)
            {
                std::cout << response.friendslist(i) << std::endl;
            }
        }
        else
        {
            std::cout << "RPC Login Response Error: " << response.ret().errmsg() << std::endl;
        }
    }
    return 0;
}