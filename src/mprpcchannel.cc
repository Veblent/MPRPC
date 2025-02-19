#include "mprpcchannel.h"
#include "mprpcapplication.h"
#include "mprpccontroller.h"
#include "rpcheader.pb.h"
#include "zookeeperutil.h"
#include "logger.h"
#include <string>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <sstream>
#include <sys/socket.h>
#include <sys/types.h>
#include <error.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

/*
    1. rpc请求的数据组装、数据的序列化
    2. 发送rpc请求，等待远端响应
    3. 接受rpc响应，反序列化
*/
void MprpcChannel::CallMethod(const google::protobuf::MethodDescriptor *method,
                            google::protobuf::RpcController *controller,
                            const google::protobuf::Message *request,
                            google::protobuf::Message *response,
                            google::protobuf::Closure *done)
{
    const google::protobuf::ServiceDescriptor* serviceDesc = method->service();
    std::string serviceName = serviceDesc->name();
    std::string methodName = method->name();

    // 获取调用参数的序列化字符串的长度
    std::string argStr;
    uint32_t argSize = 0;
    if(request->SerializeToString(&argStr))
    {
        argSize = argStr.size();
    }
    else
    {
        controller->SetFailed("Serialize Request Error!");
        return ;
    }

    mprpc::RpcHeader rpcHeader;
    rpcHeader.set_servicename(serviceName);
    rpcHeader.set_methodname(methodName);
    rpcHeader.set_argszie(argSize);
    
    std::string rpcHeaderStr;
    uint32_t headerSize = 0;
    if(rpcHeader.SerializeToString(&rpcHeaderStr))
    {
        headerSize = rpcHeaderStr.size();
    }
    else
    {
        controller->SetFailed("Serialize rpcHeader Error!");
        return ;
    }

    // 组织待发送的rpc请求的字符串
    std::string sendRpcStr;
    sendRpcStr.insert(0, std::string((char*)&headerSize, 4));       // header size
    sendRpcStr += rpcHeaderStr;                                     // header
    sendRpcStr += argStr;                                         // args

    std::cout << "================================" << std::endl;
    std::cout << "headerSize:\t" << headerSize << std::endl;
    std::cout << "rpcHeaderStr:\t" << rpcHeaderStr << std::endl;
    std::cout << "serviceName:\t" << serviceName << std::endl;
    std::cout << "methodName:\t" << methodName << std::endl;
    std::cout << "argSize:\t" << argSize << std::endl;
    std::cout << "argStr:\t" << argStr << std::endl;
    std::cout << "================================" << std::endl;

    // 使用tcp编程，发送rpc请求
    int cfd = socket(AF_INET, SOCK_STREAM, 0);
    if(cfd == -1)
    {
        std::stringstream ss;
        ss << "Socket Error, Errno: " << errno;
        controller->SetFailed(ss.str());
        exit(EXIT_FAILURE);
    }

    // std::string ip = MprpcApplication::GetConfig().Load("rpcserverIP");
    // uint16_t port = atoi(MprpcApplication::GetConfig().Load("rpcserverPORT").c_str());

    ZKClient zkCli;
    zkCli.Start();
    //  /UserServiceRpc/Login
    std::string method_path = "/" + serviceName + "/" + methodName;
    // 127.0.0.1:8000
    std::string host_data = zkCli.GetData(method_path.c_str());
    LOG_INFO("Search from Zookeeper: [%s]-[%s]", method_path.c_str(), host_data.c_str());
    if (host_data == "")
    {
        controller->SetFailed(method_path + " is not exist!");
        return;
    }
    int idx = host_data.find(":");
    if (idx == -1)
    {
        controller->SetFailed(method_path + " address is invalid!");
        return;
    }
    std::string ip = host_data.substr(0, idx);
    uint16_t port = atoi(host_data.substr(idx+1, host_data.size()-idx).c_str());

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr(ip.c_str());

    if(connect(cfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
    {
        std::stringstream ss;
        ss << "Connect Error, Errno: " << errno;
        controller->SetFailed(ss.str());
        close(cfd);
        exit(EXIT_FAILURE);
    }

    // 发送rpc请求
    if(send(cfd, sendRpcStr.c_str(), sendRpcStr.size(), 0) == -1)
    {
        std::stringstream ss;
        ss << "Send Error, Errno: " << errno;
        controller->SetFailed(ss.str());
        close(cfd);
        return ;
    }

    // 接受rpc请求的响应
    char recvBuf[1024] = {0};
    int recvSize;
    // recv阻塞等待rpc服务器返回响应结果
    if((recvSize = recv(cfd, recvBuf, 1024, 0)) == -1)
    {
        std::stringstream ss;
        ss << "Recv Error, Errno: " << errno;
        controller->SetFailed(ss.str());
        close(cfd);
        return ;
    }


    if(!response->ParseFromArray(recvBuf, recvSize))
    {
        std::stringstream ss;
        ss << "Parse Error, recvBuf: " << recvBuf;
        controller->SetFailed(ss.str());
        close(cfd);
        return ;
    }

    close(cfd);
}