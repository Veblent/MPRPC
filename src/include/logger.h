#ifndef LOGGER_H
#define LOGGER_H
#include "lockqueue.h"
#include <string>

//  日志系统
enum LogFlag
{
    INFO,       // 普通信息
    ERROR,      // 错误信息
};

class Logger
{
public:
    static Logger& GetInstance();

    void setLogFlag(LogFlag flag);
    void Log(std::string msg);
private:
    int _logFlag;
    LockQueue<std::string> _lckQueue;

    Logger();
    Logger(const Logger &) = delete;
    Logger(Logger &&) = delete;
    Logger& operator()(const Logger &) = delete;
};


// 定义宏
#define LOG_INFO(logmsgformat, ...)                     \
    do                                                  \
    {                                                   \
        Logger &logger = Logger::GetInstance();         \
        logger.setLogFlag(INFO);                        \
        char c[1024] = {0};                             \
        snprintf(c, 1024, logmsgformat, ##__VA_ARGS__);  \
        logger.Log(c);                                  \
    }while(0);


#define LOG_ERR(logerrformat, ...)                      \
    do                                                  \
    {                                                   \
        Logger &logger = Logger::GetInstance();         \
        logger.setLogFlag(ERROR);                       \
        char c[1024] = {0};                             \
        snprintf(c, 1024, logerrformat, ##__VA_ARGS__);  \
        logger.Log(c);                                  \
    }while(0);

#endif