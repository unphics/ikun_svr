#pragma once

#include <string>
#include <functional>
#include <memory>

namespace last::net {

class eventloop;
class socket;
class channel;

class acceptor {
public:
    acceptor(eventloop* loop, const std::string& ip, const uint32_t& port);
    ~acceptor();
    void new_conn();
    void set_new_conn_cb(const std::function<void(std::unique_ptr<socket>)>& cb);
private:
    eventloop* _loop; // acceptor对事件循环没有所有权, 不能使用移动语义, 只能使用常引用
    socket* _lsock;
    channel* _lchnl;
    std::function<void(std::unique_ptr<socket>)> new_conn_cb;
};

}