#include "mprpcconfig.h"
#include "logger.h"
#include <string>
#include <cstring>
#include <algorithm>

/*
    rpcserverIP
    rpcserverPORT
    zookeeperIP
    zookeeperPORT
*/

// 加载并解析配置文件
void MprpcConfig::LoadConfigFile(const char *config_file)
{
    FILE *fp = fopen(config_file, "r");
    if (fp == nullptr)
    {
        // std::cout << config_file << " NOT EXISTS!" << std::endl;
        LOG_ERR("[%s] Not Exists!", config_file);
        exit(EXIT_FAILURE);
    }
    /*
        1. 注释
        2. 去掉开头多余的空格
        3. 正确的配置选项 =
    */
    char buf[128];
    while (!feof(fp))
    {
        memset(buf, 0, sizeof(buf));
        fgets(buf, sizeof(buf), fp);

        std::string src_buf(buf);
        
        // 去除所有空格 换行符 Tab等
        src_buf.erase(std::remove_if(src_buf.begin(), src_buf.end(), ::isspace), src_buf.end());

        // 判断#的注释 以及 空行
        if (src_buf.empty() || src_buf[0] == '#')
        {
            continue;
        }

        // 判断配置项
        int idx = src_buf.find('=');
        if (idx == std::string::npos)
        {
            // 配置项不合法
            continue;
        }
        std::string key = src_buf.substr(0, idx);
        std::string value = src_buf.substr(idx + 1, src_buf.size() - idx);
        _configMap.insert({key, value});
    }
}

// 查询并返回键对应的值
std::string MprpcConfig::Load(std::string key)
{
    auto it = _configMap.find(key);
    if (it == _configMap.end())
    {
        return "";
    }
    return it->second;
}
