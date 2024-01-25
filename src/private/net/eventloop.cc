#include "net/eventloop.hh"

#include "essential_head.hh"
#include "net/poller.hh"
#include "net/channel.hh"
#include "net/connect.hh"

#include <unistd.h> // syscall
#include <syscall.h> // SYS_gettid
#include <sys/eventfd.h>
#include <iostream>
#include <cstring> // strerro
#include <time.h>

int create_timer_fd(int sec) {
    int tfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);
    struct ::itimerspec time_out;
    memset(&time_out, 0, sizeof(struct ::itimerspec));
    time_out.it_value.tv_sec = sec;
    time_out.it_value.tv_nsec = 0;
    ::timerfd_settime(tfd, 0, &time_out, 0);
    return tfd;
}

namespace last::net {

eventloop::eventloop(bool main_loop, int time_tvl, int time_out):_main_loop(main_loop), _time_tvl(time_tvl),
    _time_out(time_out), _shut_down(false), _ep(new poller), _weakup_fd(::eventfd(0, EFD_NONBLOCK)), 
    _weak_chnl(new channel(this, this->_weakup_fd)), _timer_fd(create_timer_fd(time_tvl)),
    _timer_chnl(new channel(this, this->_timer_fd)) {
    this->_weak_chnl->set_read_cb(std::bind(&eventloop::handle_weakup, this));
    this->_weak_chnl->enable_read();
    this->_weak_chnl->use_et();
    this->_timer_chnl->use_et();
    this->_timer_chnl->set_read_cb(std::bind(&eventloop::handle_timer, this));
    this->_timer_chnl->enable_read();
}
eventloop::~eventloop() {
    delete this->_ep;
}
void eventloop::run() {
    print("eventloop::run() thd is ", ::syscall(SYS_gettid));
    this->_thd_id = ::syscall(SYS_gettid);
    while (this->_shut_down == false) {
        std::vector<last::net::channel*> chnls = this->_ep->loop();
        if (chnls.size() == 0) {
            if (this->_time_out_cb) {
                this->_time_out_cb(this);
            }
        } else {
            for (auto& chnl : chnls) {
                chnl->handle_event();
            } 
        }
    }
}
poller* eventloop::ep() {
    return this->_ep;
}
void eventloop::set_time_out_cb(const std::function<void(eventloop*)>& cb) {
    this->_time_out_cb = cb;
}
bool eventloop::is_in_loop_thd() {
    return this->_thd_id == ::syscall(SYS_gettid);
}
void eventloop::add(const std::function<void()>& task)  {
    {
        std::lock_guard<std::mutex> guard(this->_mutex);
        this->_tasks.push(task);
    }
    // 唤醒事件循环
    this->weakup();
}
void eventloop::weakup() {
    uint64_t val = 1; // 随便写啥都行
    int ret = ::write(this->_weakup_fd, &val, sizeof(val));
}
void eventloop::handle_weakup() {
    // printf("eventloop::handle_weakup: thd id is %d\n", ::syscall(SYS_gettid));
    uint64_t val;
    ::read(this->_weakup_fd, &val, sizeof(val)); // 从evfd中读取出数据, 如果不读取, 在水平触发模式下读读事件会一直触发
    std::function<void()> fn;
    std::lock_guard<std::mutex> guard(this->_mutex);
    while (this->_tasks.size() > 0) {
        fn = std::move(this->_tasks.front());
        this->_tasks.pop();
        fn();
    }
}
void eventloop::handle_timer() {
    // 重新计时
    // int tfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);
    struct ::itimerspec time_out;
    memset(&time_out, 0, sizeof(struct ::itimerspec));
    time_out.it_value.tv_sec = this->_time_tvl;
    time_out.it_value.tv_nsec = 0;
    ::timerfd_settime(this->_timer_fd, 0, &time_out, 0);
    if (this->_main_loop == true) {
        // print("主事件循环的闹钟到了");
    } else {
        // print("从事件循环的闹钟到了");
        ::time_t now = ::time(0);
        for (auto it = this->_conns.begin(); it != this->_conns.end(); ++it) {
            if (it->second->time_out(now, this->_time_out)) {
                this->_timer_cb(it->first);
                {
                    std::lock_guard<std::mutex> guard(this->_mutex);
                    it = this->_conns.erase(it);
                }
            }
        }
    }
}
void eventloop::new_conn(std::shared_ptr<connect> conn) {
    this->_conns[conn->fd()] = conn;
}
void eventloop::set_timer_cb(const std::function<void(int)>& cb) {
    this->_timer_cb = cb;
}
void eventloop::shut_down() {
    this->_shut_down = true;
    this->weakup(); // 有可能正在阻塞
}

}