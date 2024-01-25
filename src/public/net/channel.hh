#pragma once

#include <cstdint>
#include <functional>
#include <memory>

namespace last::net {

class poller;
class socket;
class eventloop;

class channel {
public:
    channel(eventloop* loop, uint32_t fd, bool is_listen = false);
    ~channel();
    uint32_t fd() const;
    void use_et(); // 采用边沿触发模式
    void enable_read(); // 让epoll_wait监视fd的读事件
    void disenable_read(); // 取消读事件
    void enable_write(); // 注册写事件
    void disenable_write(); // 取消写事件
    void disable_all();
    void remove(); // 从事件循环中删除channel
    void set_in_ep();
    void set_revs(uint32_t ev);
    bool in_ep() const;
    uint32_t evs() const;
    uint32_t revs() const;
    void handle_event();
    void set_read_cb(const std::function<void()>& cb);
    void set_write_cb(const std::function<void()>& cb);
    void set_close_cb(const std::function<void()>& cb);
    void set_error_cb(const std::function<void()>& cb);
private:
    uint32_t _fd = -1;
    bool _in_ep = false; // 是否已经被添加到ep上, 未添加则调用epoll_ctl的时候使用EPOLL_CTL_ADD否则用EPOLL_CTL_MOD
    uint32_t _evs = 0; // fd需要监视的事件, lfd和cfd都需要监视EPOLLIN, cfd还需要监视EPOLLOUT
    uint32_t _revs = 0; // fd已经发生的事件
    std::function<void()> _read_cb;
    std::function<void()> _close_cb;
    std::function<void()> _error_cb;
    std::function<void()> _write_cb;
    eventloop* _loop;
};

}