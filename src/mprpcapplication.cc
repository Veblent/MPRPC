#include "mprpcapplication.h"
#include <iostream>
#include <unistd.h>
#include <string>
#include <stdio.h>


void ShowArgsHelp()
{
    std::cout << "Format : command -i [config file]" << std::endl;
}

bool MprpcApplication::isInited = false;
MprpcConfig MprpcApplication::_config = MprpcConfig();

MprpcApplication::MprpcApplication(){}

void MprpcApplication::Init(int argc, char **argv)
{
    if(isInited) return;
    if(argc < 2)
    {
        ShowArgsHelp();
        exit(EXIT_FAILURE);
    }

    int c = 0;
    std::string config_file;
    while((c = getopt(argc, argv, "i:")) != -1)
    {
        switch (c)
        {
        case 'i':
            config_file = optarg;
            break;
        case '?':
            ShowArgsHelp();
            exit(EXIT_FAILURE);
        case ':':
            ShowArgsHelp();
            exit(EXIT_FAILURE);
        default:
            break;
        }
    }

    // 开始加载配置文件
    _config.LoadConfigFile(config_file.c_str());
    isInited = true;
}

MprpcApplication& MprpcApplication::GetInstance()
{
    static MprpcApplication app;
    return app;
}

MprpcConfig& MprpcApplication::GetConfig()
{
    return _config;
}