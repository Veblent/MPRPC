#ifndef RPCPROVIDER_H
#define RPCPROVIDER_H

#include "google/protobuf/service.h"
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <string>
#include <functional>
#include <google/protobuf/descriptor.h>
#include <unordered_map>
#include <memory>



// 框架提供的专门服务发布rpc服务的网络对象类
class RpcProvider
{
public:
    // 这里是框架提供给外部使用的，可以发布rpc方法的函数接口
    void NotifyService(google::protobuf::Service *service);
    
    // 启动rpc服务节点，开始提供rpc远程调用服务
    void Run();

private:
    muduo::net::EventLoop _eventLoop;

    // 服务类型信息
    struct ServiceInfo
    {
        google::protobuf::Service *_service;    // 保存服务对象
        std::unordered_map<std::string, std::shared_ptr<const google::protobuf::MethodDescriptor>> _methodMap; // 保存该服务所有的方法
    };

    // 保存所有注册成功的服务信息
    std::unordered_map<std::string, std::shared_ptr<ServiceInfo>> _serviceMap;

    // 客户端请求连接的回调函数
    void onConnection(const muduo::net::TcpConnectionPtr&);

    // 客户端数据到来回调
    void onMessage(const muduo::net::TcpConnectionPtr&, muduo::net::Buffer*, muduo::Timestamp);

    // Closure的回调操作，用于序列化rpc响应数据的序列化以及网络发送
    void sendRpcResponse(const muduo::net::TcpConnectionPtr&, google::protobuf::Message *);
};


#endif