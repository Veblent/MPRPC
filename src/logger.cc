#include "logger.h"
#include <thread>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iostream>

Logger::Logger()
{
    // 启动专门的写日志线程
    std::thread wirteLogTask([&](){
        while(true)
        {
            // 获取当天日期，获取日志信息，写入相应的日志文件 a+
            std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
            std::time_t tt = std::chrono::system_clock::to_time_t(now);
            tm *nowtm = localtime(&tt);

            std::stringstream filename;
            filename << "./log/" << nowtm->tm_year + 1900 << "-" << nowtm->tm_mon + 1 << "-" << nowtm->tm_mday << "-log.log";

            FILE *fp = fopen(filename.str().c_str(), "a+");
            if(fp == nullptr)
            {
                std::cout << "Logger File: " << filename.str() << " Open Error" << std::endl;
                exit(EXIT_FAILURE);
            }
            
            // _lckQueue实现了线程安全，此处会阻塞等待缓冲区有数据写入
            std::string msg = _lckQueue.pop();
            std::stringstream time_msg;
            time_msg << nowtm->tm_hour << ":" << nowtm->tm_min << ":" << nowtm->tm_sec << "\t" \
                     << "[" << (_logFlag == INFO ? "INFO" : "ERROR") << "]\t" << msg << "\n";

            fputs(time_msg.str().c_str(), fp);
            fclose(fp);
        }
    });
    // 设置线程分离属性
    wirteLogTask.detach();
}


Logger& Logger::GetInstance()
{
    static Logger logger;
    return logger;
}


void Logger::setLogFlag(LogFlag flag)
{
    _logFlag = flag;
}

// 把日志信息写入LockQueue缓冲区当中
void Logger::Log(std::string msg)
{
    _lckQueue.push(msg);
}