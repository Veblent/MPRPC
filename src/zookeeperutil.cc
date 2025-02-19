#include "zookeeperutil.h"
#include "mprpcapplication.h"
#include "logger.h"
#include <iostream>

// 全局的watcher观察器：zkserver给zkclient的通知
void global_watcher(zhandle_t *zh, int type, int state, const char *path, void *watcherCtx)
{
    // 回调的消息类型是和会话相关的消息类型
    if(type == ZOO_SESSION_EVENT)
    {
        // zkclient和zkserver连接成功
        if(state == ZOO_CONNECTED_STATE)
        {
            sem_t *sem = (sem_t *)zoo_get_context(zh);
            sem_post(sem);
        }
    }
}


ZKClient::ZKClient()
{
    _zkhandle = nullptr;
}


ZKClient::~ZKClient()
{
    if(_zkhandle != nullptr)
    {
        zookeeper_close(_zkhandle);
    }
}

// ZKClient启动zkserver
void ZKClient::Start()
{
    // std::string host = MprpcApplication::GetConfig().Load("zookeeperip");
    // std::string port = MprpcApplication::GetConfig().Load("zookeeperport");

    std::string host = "127.0.0.1";
    std::string port = "2181";

    std::string connStr = host + ":" + port;
    
    /*
        zookeeper_mt: 多线程版本
        zookeeper的API客户端程序提供了三个线程
            1. API调用线程
            2. 网络I/O线程（pthread_create; poll）
            3. watcher回调线程 pthread_create
    */

    // zkclient请求zkserver是异步的，zkclient并不会在此阻塞等待zkserver响应
    _zkhandle = zookeeper_init(connStr.c_str(), global_watcher, 30000, nullptr, nullptr, 0);
    
    // _zkhandle！=nullptr仅代表_zkhandle初始化成功；而不是表示连接zkserver成功
    if(_zkhandle == nullptr) 
    {
        LOG_ERR("zookeeper_init Error!");
        exit(EXIT_FAILURE);
    }

    sem_t sem;
    sem_init(&sem, 0, 0);

    // 给句柄的上下文绑定信号量，连接成功后回调函数global_watcher将信号量++
    zoo_set_context(_zkhandle, &sem);

    sem_wait(&sem);     // 阻塞等待连接是否成功
    LOG_INFO("zookeeper_init Successed!");
    
}

// 在zkserver上根据指定的path创建znode节点（state=0表示是临时性节点）
void ZKClient::Create(const char *path, const char *data, int datalen, int state)
{
    char path_buffer[128];
    int bufferLen = sizeof(path_buffer);
    int flag;

    // 同步判断：检查节点是否存在
    flag = zoo_exists(_zkhandle, path, 0, nullptr);
    LOG_INFO("ZKClient tend to Create Node,  flag = [%d]", flag);
    if(flag == ZNONODE)
    {
        // 节点不存在：创建指定path的znode节点
        flag = zoo_create(_zkhandle, path, data, datalen, &ZOO_OPEN_ACL_UNSAFE,\
            state, path_buffer, bufferLen);
        if(flag == ZOK)
        {
            LOG_INFO("ZNode Create Successed... path: [%s]", path);
        }
        else
        {
            LOG_ERR("flag: [%d]; ZNode Create Falied... path: [%s]", flag, path);
            exit(EXIT_FAILURE);
        }
    }
}

// 根据指定的znode节点路径，获取znode节点的值
std::string ZKClient::GetData(const char *path)
{
    char buf[64];
    int bufLen = sizeof(buf);
    int flag = zoo_get(_zkhandle, path, 0, buf, &bufLen, nullptr);
    if(flag != ZOK)
    {
        LOG_ERR("flag: [%d]; Get ZNode Falied... path: [%s]", flag, path);
        return "";
    }
    else
    {
        return buf;
    }
}