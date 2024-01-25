#include "net/tcpsvr.hh"

#include "essential_head.hh"
#include "net/eventloop.hh"
#include "net/socket.hh"
#include "net/channel.hh"
#include "net/acceptor.hh"
#include "net/connect.hh"
#include "net/threadpool.hh"

#include <unistd.h> // close
#include <unistd.h> // syscall
#include <syscall.h> // SYS_gettid

namespace last::net {

tcpsvr::tcpsvr(const std::string& ip, uint32_t port, uint32_t thd_count): _thd_count(thd_count),
    _main_loop(new eventloop(true)), _acceptor(new acceptor(this->_main_loop.get(), ip, port)),
    _thd_pool(new threadpool(this->_thd_count, "io_thd")) {
    print("tcpsvr::tcpsvr main thd is", ::syscall(SYS_gettid));
    this->_main_loop->set_time_out_cb(std::bind(&tcpsvr::_on_time_out, this, std::placeholders::_1));
    this->_acceptor->set_new_conn_cb(std::bind(&tcpsvr::_on_new_conn, this, std::placeholders::_1));
    for (uint32_t i = 0; i < this->_thd_count; ++i) {
        this->_sub_loops.emplace_back(std::make_unique<eventloop>(false, 5, 10));
        this->_sub_loops[i]->set_time_out_cb(std::bind(&tcpsvr::_on_time_out, this, std::placeholders::_1));
        this->_sub_loops[i]->set_timer_cb(std::bind(&tcpsvr::remove_conn, this, std::placeholders::_1));
        this->_thd_pool->add(std::bind(&eventloop::run, this->_sub_loops[i].get()));
    }
}
tcpsvr::~tcpsvr() {
    delete this->_acceptor;
}
void tcpsvr::start() {
    this->_main_loop->run();
}
void tcpsvr::_on_new_conn(std::unique_ptr<socket> csock) {
    // printf("accept new client fd =", csock->fd(), ", ip =", csock->ip().c_str(), ", port =", csock->port());
    std::shared_ptr<connect> share_conn = std::make_shared<connect>(this->_sub_loops[csock->fd() % this->_thd_count].get(), std::move(csock));
    share_conn->set_close_cb(std::bind(&tcpsvr::_on_conn_close, this, std::placeholders::_1));
    share_conn->set_error_cb(std::bind(&tcpsvr::_on_conn_error, this, std::placeholders::_1));
    share_conn->set_msg_cb(std::bind(&tcpsvr::_on_msg, this, std::placeholders::_1, std::placeholders::_2));
    share_conn->set_send_cb(std::bind(&tcpsvr::_on_post_send, this, std::placeholders::_1));
    {
        std::lock_guard<std::mutex> guard(this->_mutex);
        this->_sub_loops[share_conn->fd() % this->_thd_count]->new_conn(share_conn);
    }
    this->_conns[share_conn->fd()] = share_conn;
    if (this->_new_conn_cb) {
        this->_new_conn_cb(share_conn);
    }
}
void tcpsvr::_on_conn_close(std::shared_ptr<connect> conn) {
    if (this->_conn_close_cb) {
        this->_conn_close_cb(conn);
    }
    print("client disconnected !!!");
    {
        std::lock_guard<std::mutex> guard(this->_mutex);
        this->_conns.erase(conn->fd());
    }
}
void tcpsvr::_on_conn_error(std::shared_ptr<connect> conn) {
    if (this->_conn_error_cb) {
        this->_conn_error_cb(conn);
    }
    {
        std::lock_guard<std::mutex> guard(this->_mutex);
        this->_conns.erase(conn->fd());
    }
}
void tcpsvr::_on_msg(std::shared_ptr<connect> conn, const std::string& msg) {
    if (this->_on_msg_cb) {
        this->_on_msg_cb(conn, msg);
    }
}
void tcpsvr::_on_post_send(std::shared_ptr<connect> conn) {
    // 根据业务的需求添加其他代码
    if (this->_post_send_cb) {
        this->_post_send_cb(conn);
    }
}
void tcpsvr::_on_time_out(eventloop* loop) {
    print("epoll wait time out");
    if (this->_time_out_cb) {
        this->_time_out_cb(loop);
    }
}

void tcpsvr::set_new_conn_cb(const std::function<void(std::shared_ptr<connect>)>& cb) {
    this->_new_conn_cb = cb;
}
void tcpsvr::set_conn_close_cb(const std::function<void(std::shared_ptr<connect>)>& cb) {
    this->_conn_close_cb = cb;
}
void tcpsvr::set_conn_error_cb(const std::function<void(std::shared_ptr<connect>)>& cb) {
    this->_conn_error_cb = cb;
}
void tcpsvr::set_on_msg_cb(const std::function<void(std::shared_ptr<connect>, const std::string&)>& cb) {
    this->_on_msg_cb = cb;
}
void tcpsvr::set_post_send_cb(const std::function<void(std::shared_ptr<connect>)>& cb) {
    this->_post_send_cb = cb;
}
void tcpsvr::set_time_out_cb(const std::function<void(eventloop*)>& cb) {
    this->_time_out_cb = cb;
}
void tcpsvr::remove_conn(uint32_t fd) {
    {
        std::lock_guard<std::mutex> guard(this->_mutex);
        this->_conns.erase(fd);
    }
}
void tcpsvr::shut_down() {
    this->_main_loop->shut_down();
    print("主事件循环已停止");
    for (auto it = this->_sub_loops.begin(); it != this->_sub_loops.end(); ++it) {
        it->get()->shut_down();
    }
    print("从事件循环已停止");
    this->_thd_pool->shut_down();
    print("IO线程池已停止");
}

}