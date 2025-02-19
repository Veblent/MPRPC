#ifndef MPRPCCONFIG_H
#define MPRPCCONFIG_H
#include <iostream>
#include <unordered_map>
#include <string>

// 框架读取配置文件
/*
    rpcserverIP
    rpcserverPORT
    zookeeperIP
    zookeeperPORT
*/

class MprpcConfig
{
public:
    // 加载并解析配置文件
    void LoadConfigFile(const char *config_file);

    // 查询并返回键对应的值
    std::string Load(std::string key);

private:
    std::unordered_map<std::string, std::string> _configMap;
};

#endif