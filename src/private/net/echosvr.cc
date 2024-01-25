#include "net/echosvr.hh"

#include "essential_head.hh"
#include "net/connect.hh"

#include "client_test.pb.h"

#include <unistd.h> // syscall
#include <syscall.h> // SYS_gettid

namespace last::net {

echosvr::echosvr(const std::string& ip, uint32_t port, uint32_t thd_count, uint32_t work_count): _tcp(ip, port, thd_count), _wrok_pool(work_count, "work_thd") {
    this->_tcp.set_new_conn_cb(std::bind(&echosvr::handle_new_conn, this, std::placeholders::_1));
    this->_tcp.set_conn_close_cb(std::bind(&echosvr::handle_close, this, std::placeholders::_1));
    this->_tcp.set_conn_error_cb(std::bind(&echosvr::handle_error, this, std::placeholders::_1));
    this->_tcp.set_on_msg_cb(std::bind(&echosvr::handle_msg, this, std::placeholders::_1, std::placeholders::_2));
    this->_tcp.set_post_send_cb(std::bind(&echosvr::handle_post_send, this, std::placeholders::_1));
    this->_tcp.set_time_out_cb(std::bind(&echosvr::handle_time_out, this, std::placeholders::_1));
}
echosvr::~echosvr() {

}
void echosvr::start() {
    this->_tcp.start();
}
void echosvr::handle_new_conn(std::shared_ptr<connect> conn) {
    // printf("echosvr::handle_new_conn thd is %d\n", ::syscall(SYS_gettid));
}
void echosvr::handle_close(std::shared_ptr<connect> conn) {

}
void echosvr::handle_error(std::shared_ptr<connect> conn) {

}
void echosvr::handle_msg(std::shared_ptr<connect> conn, std::string msg) {
    if (this->_wrok_pool.size() == 0) {
        // 如果没有工作线程, 则直接在UI线程中计算
        this->on_msg(conn, msg);
    } else {
        // 把业务添加到业务线程池中
        this->_wrok_pool.add(std::bind(&echosvr::on_msg, this, conn, msg));
    }
}
void echosvr::handle_post_send(std::shared_ptr<connect> conn) {

}
void echosvr::handle_time_out(eventloop* loop) {

}
void echosvr::on_msg(std::shared_ptr<connect> conn, std::string msg) {
    client_test::req req;
    req.ParseFromString(msg);
    std::string recv(std::to_string(req.id()) + " " + req.name());
    // print("thread ", ::syscall(SYS_getgid), ": deal with : ", req.header().protocol_name() + " : " + recv.c_str());
    recv = "reply: " + recv;
    conn->send(recv.data(), recv.size());
    // std::string a("qqqqqqq");
    // conn->send(a.data(), a.size());
}
void echosvr::shut_down() {
    // 停止工作线程
    this->_wrok_pool.shut_down();
    print("work thread has shut down");
    // 停止IO线程
    this->_tcp.shut_down();
}

}