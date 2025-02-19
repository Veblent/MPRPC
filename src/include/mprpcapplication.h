#ifndef MPRPCAPPLICATION_H
#define MPRPCAPPLICATION_H

#include "mprpcconfig.h"
#include "mprpcchannel.h"
#include "mprpccontroller.h"

// mprpc框架的基础类，负责框架的一些初始化工作（单例模式）
class MprpcApplication
{
public:
    static void Init(int argc, char **argv);
    static MprpcApplication& GetInstance();
    static MprpcConfig& GetConfig();

private:
    static bool isInited ;
    static MprpcConfig _config;
    MprpcApplication();
    MprpcApplication(const MprpcApplication&) = delete;
    MprpcApplication(MprpcApplication&&) = delete;
};

#endif