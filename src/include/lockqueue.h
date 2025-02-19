#ifndef LOCKQUEUE_H
#define LOCKQUEUE_H
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

// 模板代码只能写在头文件当中
template<typename T>
class LockQueue
{
public:
    // 多个线程都会写日志queue缓冲区
    void push(const T &data)
    {
        std::lock_guard<std::mutex> lock(_mtx);
        _queue.push(data);
        _cndVar.notify_one();
    }

    // 只有一个线程读出queue缓冲区
    T pop()
    {
        std::unique_lock<std::mutex> lock(_mtx);
        _cndVar.wait(lock, [&](){return !_queue.empty();});
        T data = _queue.front();
        _queue.pop();
        return data;
    }

private:
    std::queue<T> _queue;
    std::mutex _mtx;
    std::condition_variable _cndVar;
};

#endif