#ifndef ZOOKEEPERUTIL_H
#define ZOOKEEPERUTIL_H

#include <semaphore.h>
#include <zookeeper/zookeeper.h>
#include <string>

class ZKClient
{
public:
    ZKClient();
    ~ZKClient();

    // ZKClient启动zkserver
    void Start();
    
    // 在zkserver上根据指定的path创建znode节点（state=0表示是临时性节点）
    void Create(const char *path, const char *data, int datalen, int state=0);
    
    // 根据指定的znode节点路径，获取znode节点的值
    std::string GetData(const char *path);
private:
    // ZK的客户端句柄
    zhandle_t *_zkhandle;
};

#endif