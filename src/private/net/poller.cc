#include "net/poller.hh"

#include "essential_head.hh"
#include "net/channel.hh"

#include <iostream> // print
#include <unistd.h> // close
#include <cstring> // memset

namespace last::net {

poller::poller() {
    if (this->_epfd = ::epoll_create(1), this->_epfd == -1) {
        print("failed to create epfd !!!");
        ::exit(-1);
    }
}
poller::~poller() {
    ::close(this->_epfd);
}
void poller::update(channel* chnl) {
    epoll_event ev;
    ev.data.ptr = chnl;
    ev.events = chnl->evs();
    if (chnl->in_ep()) {
        if (::epoll_ctl(this->_epfd, EPOLL_CTL_MOD, chnl->fd(), &ev) == -1) {
            print("failed to add ev by ep ctl !!!");
            ::exit(-1);
        }
    } else {
        if (::epoll_ctl(this->_epfd, EPOLL_CTL_ADD, chnl->fd(), &ev) == -1) {
            print("failed to add ev by ep ctl !!!");
            ::exit(-1);
        }
        chnl->set_in_ep();
    }
}
void poller::remove(channel* chnl) {
    if (chnl->in_ep()) {
        if (::epoll_ctl(this->_epfd, EPOLL_CTL_DEL, chnl->fd(), 0) == -1) {
            print("failed to del ev by ep ctl !!!");
            ::exit(-1);
        }
    }
}
std::vector<channel*> poller::loop(int32_t time_out) {
    std::vector<channel*> evs; // 存放epoll_wait返回的事件
    memset(this->_evs, 0, sizeof(this->_evs));
    int infds = ::epoll_wait(this->_epfd, this->_evs, MAX_EVS,time_out); // 等待监视的fd有事件发生
    if (infds < 0) {
        print("failed to exec epoll wait !!!"); // 返回失败
        ::exit(-1);
    }
    if (infds == 0) {
        print("epoll wait time out !!!"); // 超时
        return evs;
    }
    for (int16_t i = 0; i < infds; ++i) {
        channel* chnl = (channel*)this->_evs[i].data.ptr;
        chnl->set_revs(this->_evs->events);
        evs.push_back(chnl);
    }
    return evs;
}
uint32_t poller::fd() const {
    return this->_epfd;
}

}