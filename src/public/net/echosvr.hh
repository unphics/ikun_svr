#pragma once

#include "tcpsvr.hh"
#include "threadpool.hh"

namespace last::net {

class socket;
class connect;
class eventloop;
// class threadpool;

class echosvr {
public:
    echosvr(const std::string& ip, uint32_t port, uint32_t thd_count, uint32_t work_count);
    ~echosvr();
    void start();
    void handle_new_conn(std::shared_ptr<connect> conn);
    void handle_close(std::shared_ptr<connect> conn);
    void handle_error(std::shared_ptr<connect> conn);
    void handle_msg(std::shared_ptr<connect> conn, std::string msg);
    void handle_post_send(std::shared_ptr<connect> conn);
    void handle_time_out(eventloop* loop);
    void on_msg(std::shared_ptr<connect> conn, std::string msg);
    void shut_down();
private:
    tcpsvr _tcp;
    threadpool _wrok_pool;
};

}