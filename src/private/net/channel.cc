#include "net/channel.hh"

#include "net/poller.hh"
#include "net/eventloop.hh"
#include "net/connect.hh"

#include <sys/epoll.h>


namespace last::net {
// typedef int(*a)(int, int);
// using b = int(*)(int, int);
// int c[1];
// int* d[1];
// int* (*e);
// int(*(*v)[10])(int,int);
// int(*(*w()))(int,int);

channel::channel(eventloop* loop, uint32_t fd, bool is_listen): _loop(loop), _fd(fd) {}
channel::~channel() {
    // ep和fd都不属于channel
}
uint32_t channel::fd() const {
    return this->_fd;
}
void channel::use_et() {
    this->_evs |= EPOLLET;
}
void channel::enable_read() {
    this->_evs |= EPOLLIN;
    this->_loop->ep()->update(this);
}
void channel::disenable_read() {
    this->_evs &= ~EPOLLIN;
    this->_loop->ep()->update(this);
}
void channel::enable_write() {
    this->_evs |= EPOLLOUT;
    this->_loop->ep()->update(this);
}
void channel::disenable_write() {
    this->_evs &= ~EPOLLOUT;
    this->_loop->ep()->update(this);
}
void channel::disable_all() {
    this->_evs = 0;
    this->_loop->ep()->update(this);
}
void channel::remove() {
    this->disable_all();
    this->_loop->ep()->remove(this);
}
void channel::set_in_ep() {
    this->_in_ep = true;
}
void channel::set_revs(uint32_t ev) {
    this->_revs = ev;
}
bool channel::in_ep() const {
    return this->_in_ep;
}
uint32_t channel::evs() const {
    return this->_evs;
}
uint32_t channel::revs() const {
    return this->_revs;
}
void channel::handle_event() {
    if (this->_revs & EPOLLRDHUP) { // 对方已关闭, 有些系统检测不到, 可以使用EPOLLOIN, recv返回0
        // 如果停车票连接断开了, 就在eventloop中删除channel
        // this->remove();
        if (this->_close_cb) {
            this->_close_cb();
        }
    } else if (this->_revs & (EPOLLIN | EPOLLPRI)) {
        if (this->_read_cb) {
            this->_read_cb();
        }
    } else if (this->_revs & EPOLLOUT) { // 有数据要写, 暂时没代码
        if (this->_write_cb) {
            this->_write_cb();
        }
    } else { // 其他事件, 都视为错误
        // this->remove();
        if (this->_error_cb) {
            this->_error_cb();
        }
    }
}
void channel::set_read_cb(const std::function<void()>& read_cb) {
    this->_read_cb = read_cb;
}
void channel::set_write_cb(const std::function<void()>& cb) {
    this->_write_cb = cb;
}
void channel::set_close_cb(const std::function<void()>& cb) {
    this->_close_cb = cb;
}
void channel::set_error_cb(const std::function<void()>& cb) {
    this->_error_cb = cb;
}

}