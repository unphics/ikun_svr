#pragma once

#include <map>
#include <functional>
#include <queue>
#include <mutex>
#include <memory>
#include <sys/timerfd.h>
#include <atomic>

namespace last::net {

class poller;
class channel;
class connect;

class eventloop {
public:
    eventloop(bool main_loop, int time_tvl = 30, int time_out = 80); // 创建epoll对象
    ~eventloop(); // 销毁epoll对象
    void run(); // 运行事件循环
    poller* ep();
    void set_time_out_cb(const std::function<void(eventloop*)>& cb);
    bool is_in_loop_thd();
    void add(const std::function<void()>& task);
    void weakup();
    void handle_weakup();
    void handle_timer();
    void new_conn(std::shared_ptr<connect> conn);
    void set_timer_cb(const std::function<void(int)>& cb);
    void shut_down();
private:
    pid_t _thd_id;
    poller* _ep;
    std::function<void(eventloop*)> _time_out_cb;
    std::queue<std::function<void()>> _tasks; // 事件循环线程被eventfd唤醒后执行的任务队列
    std::mutex _mutex;
    int32_t _weakup_fd;
    std::unique_ptr<channel> _weak_chnl;
    int _timer_fd;
    std::unique_ptr<channel> _timer_chnl;
    bool _main_loop;
    std::map<int, std::shared_ptr<connect>> _conns;
    std::function<void(int)> _timer_cb;
    int _time_tvl;
    int _time_out;
    std::atomic_bool _shut_down;
};

}