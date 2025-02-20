#include "rpcprovider.h"
#include "mprpcapplication.h"
#include "rpcheader.pb.h"
#include "logger.h"
#include "zookeeperutil.h"
#include <sstream>

// 这里是框架提供给外部使用的，可以发布rpc方法的函数接口
void RpcProvider::NotifyService(google::protobuf::Service *service)
{
    // ServiceInfo *service_info = new ServiceInfo();
    std::shared_ptr<ServiceInfo> service_info(new ServiceInfo());

    service_info->_service = service;

    // 获取服务对象的描述信息
    const google::protobuf::ServiceDescriptor *pServiceDesc = service->GetDescriptor();

    // 获取服务的名字
    std::string serviceName = pServiceDesc->name();

    // 获取服务对象Service的方法的数量
    int methodCnt = pServiceDesc->method_count();

    LOG_INFO("Service Name: [%s]", serviceName.c_str());

    for (int i = 0; i < methodCnt; i++)
    {
        // 获取了服务对象对应第 i 个服务方法的描述（抽象描述）
        // const google::protobuf::MethodDescriptor *pMethodDesc = pServiceDesc->method(i);
        std::shared_ptr<const google::protobuf::MethodDescriptor> pMethodDesc(pServiceDesc->method(i));
        std::string methodName = pMethodDesc->name();
        service_info->_methodMap.insert({methodName, pMethodDesc});

        // std::cout << "Method Name: " << methodName << std::endl;
        LOG_INFO("Method Name: [%s]", methodName.c_str());
    }
    _serviceMap.insert({serviceName, service_info});
}

// 启动rpc服务节点，开始提供rpc远程调用服务
void RpcProvider::Run()
{
    std::string ip = MprpcApplication::GetConfig().Load("rpcserverIP");
    uint16_t port = atoi(MprpcApplication::GetConfig().Load("rpcserverPORT").c_str());
    muduo::net::InetAddress address(ip, port);

    // 创建TcpServer对象
    muduo::net::TcpServer server(&_eventLoop, address, "RpcServer");

    // 绑定连接回调和消息读写回调方法 分离网络代码和业务代码
    server.setConnectionCallback(std::bind(&RpcProvider::onConnection, this, std::placeholders::_1));
    server.setMessageCallback(std::bind(&RpcProvider::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    // 设置muduo库的线程数量
    server.setThreadNum(4);


    // 把当前rpc节点上要发布的服务全部注册到zk上面，让rpc client可以从zk上发现服务
    // session timeout   30s     zkclient 网络I/O线程  1/3 * timeout 时间发送ping消息
    ZKClient zkCli;
    zkCli.Start();
    // service_name为永久性节点    method_name为临时性节点
    // 抓包测试心跳消息：sudo tcpdump -i lo port 2181
    for (auto &sp : _serviceMap) 
    {
        // /service_name   /UserServiceRpc
        std::string service_path = "/" + sp.first;

        zkCli.Create(service_path.c_str(), nullptr, 0);
        for (auto &mp : sp.second->_methodMap)
        {
            // /service_name/method_name   /UserServiceRpc/Login 存储当前这个rpc服务节点主机的ip和port
            std::string method_path = service_path + "/" + mp.first;
            std::stringstream method_path_data;
            method_path_data << ip << ":" << port;

            // ZOO_EPHEMERAL表示znode是一个临时性节点
            zkCli.Create(method_path.c_str(), method_path_data.str().c_str(), method_path_data.str().size(), ZOO_EPHEMERAL);
        }
    }

    LOG_INFO("Rpc Server Start at IP: [%s]\tPort: [%d]", ip.c_str(), port);

    // 启动网络服务
    server.start();
    _eventLoop.loop();
}

// 客户端请求连接事件回调
void RpcProvider::onConnection(const muduo::net::TcpConnectionPtr &conn)
{
    if (!conn->connected())
    {
        // 和RPC Client 的连接断开
        conn->shutdown();
    }
}

/*
    在框架内部，Caller调用Rpc方法时，应该和Callee协商好 通信的数据格式：
    1. serviceName
    2. methodName
    3. args
    因此也需要利用protobuf来定义通信的消息格式

    headerSize = 4B
    根据headerSize读取header内容
*/

// 客户读写事件回调
// 如果远程有一个RPC服务的调用请求，那么onMessage方法就会响应
void RpcProvider::onMessage(const muduo::net::TcpConnectionPtr &conn,
                            muduo::net::Buffer *buffer,
                            muduo::Timestamp time)
{
    // 网络上接收的远程rpc调用请求的字符流
    // 如：Login Args
    std::string recvBuf = buffer->retrieveAllAsString();

    // 从字符流中读取前4B内容
    uint32_t headerSize = 0;
    recvBuf.copy((char *)&headerSize, 4, 0);

    // 根据headerSize读取数据头的原始字符流
    std::string rpcHeaderStr = recvBuf.substr(4, headerSize);

    // 反序列化得到rpc请求的详细信息
    std::string serviceName;
    std::string methodName;
    uint32_t argSize;
    mprpc::RpcHeader rpcHeader;
    if (rpcHeader.ParseFromString(rpcHeaderStr))
    {
        // 数据头反序列化成功
        serviceName = rpcHeader.servicename();
        methodName = rpcHeader.methodname();
        argSize = rpcHeader.argszie();
    }
    else
    {
        // 数据头反序列化失败
        LOG_ERR("rpcHeader: [%s] Parse Error!", rpcHeaderStr.c_str());
        return;
    }

    std::string argStr = recvBuf.substr(4 + headerSize, argSize);

    std::cout << "================================" << std::endl;
    std::cout << "headerSize:\t" << headerSize << std::endl;
    std::cout << "rpcHeaderStr:\t" << rpcHeaderStr << std::endl;
    std::cout << "serviceName:\t" << serviceName << std::endl;
    std::cout << "methodName:\t" << methodName << std::endl;
    std::cout << "argSize:\t" << argSize << std::endl;
    std::cout << "argStr:\t" << argStr << std::endl;
    std::cout << "================================" << std::endl;

    // 根据判断解析得到的serviceName，检查是否存在对应的Service对象
    auto s_it = _serviceMap.find(serviceName);
    if (s_it == _serviceMap.end())
    {
        LOG_ERR("[%s] Is Not Exist!", serviceName.c_str());
        return;
    }

    // 根据判断解析得到的methodName，检查是否存在对应的Method对象
    auto m_it = s_it->second->_methodMap.find(methodName);
    if (m_it == s_it->second->_methodMap.end())
    {
        LOG_ERR("[%s]-[%s] Is Not Exist!", serviceName.c_str(), methodName.c_str());
        return;
    }
    

    // 获取Service对象和Method对象
    google::protobuf::Service *service = s_it->second->_service;
    auto method = m_it->second;

    // 生产rpc方法调用的请求request和响应respone参数
    google::protobuf::Message *request = service->GetRequestPrototype(method.get()).New();

    if (!request->ParseFromString(argStr))
    {
        LOG_ERR("Request Parse Error: [%s]", argStr.c_str());
        return;
    }
    google::protobuf::Message *response = service->GetResponsePrototype(method.get()).New();
    // 给方法调用
    // Closure* NewCallback(Class* object, void(Class::*method)(Arg1, Arg2), Arg1 arg1, Arg2 arg2)
    google::protobuf::Closure * done = \
        google::protobuf::NewCallback<RpcProvider, const muduo::net::TcpConnectionPtr&, google::protobuf::Message*> \
        (this, &RpcProvider::sendRpcResponse, conn, response);

    // 根据rpc请求，调用本地服务方法
    // CallMethod(const MethodDescriptor *method,
    //            RpcController *controller, 
    //            const Message *request,
    //            Message *response, Closure *done) = 0;
    service->CallMethod(method.get(), nullptr, request, response, done);
}


void RpcProvider::sendRpcResponse(const muduo::net::TcpConnectionPtr& conn, google::protobuf::Message *response)
{
    std::string response_str;
    if(response->SerializeToString(&response_str))
    {
        conn->send(response_str);
    }
    else
    {
        LOG_ERR("Serialize Response_str Error!");
    }
    conn->shutdown();   // 模拟http的短连接，有rpcprovide主动断开连接
}