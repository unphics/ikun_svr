#pragma once

#include <sys/epoll.h>
#include <fcntl.h>
#include <vector>

#define MAX_EVS 128

namespace last::net {

class channel;

class poller {
public:
    poller();
    ~poller();
    void update(channel* chnl);
    void remove(channel* chnl);
    std::vector<channel*> loop(int32_t time_out = -1); // 运行epoll_wait,等待事件的发生, 已发生的事件用vector返回
    uint32_t fd() const;
private:
    uint32_t _epfd = -1;
    epoll_event _evs[MAX_EVS];
};

}