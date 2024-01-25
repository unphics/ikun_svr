#pragma once

#include <thread>
#include <vector>
#include <mutex>
#include <functional>
#include <queue>
#include <condition_variable>
#include <atomic>

namespace last::net {

class threadpool {
public:
    threadpool(size_t count, const char* thd_name);
    ~threadpool();
    void add(std::function<void()> task);
    uint32_t size() const;
    void shut_down();
private:
    std::vector<std::thread> _thds;
    std::queue<std::function<void()>> _queue;
    std::mutex _mutex;
    std::condition_variable _cond;
    std::atomic_bool _stop;
    const char* _thd_name;
};

}
