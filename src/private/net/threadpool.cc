#include "net/threadpool.hh"

#include "essential_head.hh"
#include <unistd.h> // syscall
#include <syscall.h> // SYS_gettid

namespace last::net {

threadpool::threadpool(size_t count, const char* thd_name): _stop(false), _thd_name(thd_name) {
    for (size_t i = 0; i < count; ++i) {
        this->_thds.emplace_back([this]{
            print("create :", this->_thd_name, ", thread:", ::syscall(SYS_gettid));
            while (this->_stop == false) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(this->_mutex);
                    this->_cond.wait(lock, [this] {
                        return ((this->_stop == true) || (this->_queue.empty() == false));
                    });
                    // 在线程停止之前,如果队列中还有任务, 执行完再退出
                    if ((this->_stop == true) || (this->_queue.empty() == true)) {
                        return;
                    }
                    task = std::move(this->_queue.front());
                    this->_queue.pop();
                }
                // print(this->_thd_name, ::syscall(SYS_gettid), "exec task");
                task();
            }
        });
    }
}
threadpool::~threadpool() {
    this->shut_down();
}
void threadpool::add(std::function<void()> task) {
    {
        std::lock_guard<std::mutex> lock(this->_mutex);
        this->_queue.push(task);
    }
    this->_cond.notify_one();
}
uint32_t threadpool::size() const {
    return this->_thds.size();
}
void threadpool::shut_down() {
    if (this->_stop == true) {
        return;
    }
    this->_stop = true;
    this->_cond.notify_all();
    for (std::thread& thd : this->_thds) {
        thd.join();
    }
}

}