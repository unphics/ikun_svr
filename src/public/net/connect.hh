#pragma once

#include "buffer.hh"
#include "timestamp.hh"

#include <string>
#include <functional>
#include <memory>
#include <atomic>

namespace last::net {

class eventloop;
class socket;
class channel;

class connect;
class connect : public std::enable_shared_from_this<connect> {
public:
    connect(eventloop* loop, std::unique_ptr<socket> csock);
    ~connect();
    uint32_t fd() const;
    std::string ip() const;
    uint32_t port() const;
    void on_close();
    void on_error();
    void set_close_cb(const std::function<void(std::shared_ptr<connect>)>& cb);
    void set_error_cb(const std::function<void(std::shared_ptr<connect>)>& cb);
    void set_msg_cb(const std::function<void(std::shared_ptr<connect>, const std::string&)>& cb);
    void set_send_cb(const std::function<void(std::shared_ptr<connect>)>& cb); // 发送完成后的回调
    void on_msg();
    void writecb();
    void send(char* data, size_t size); // 发送数据, 不管在任何线程中, 都是调用此函数发送数据
    void send_in_loop(char* data, size_t size); // 发送数据, 如果当前线程是io线程, 直接调用此函数, 如果是工作线程, 将把此函数传给io线程
    bool time_out(::time_t now, int judge = 10); // 判断tcp连接是否超时(空闲太久)
private:
    eventloop* _loop;
    std::unique_ptr<socket> _csock;
    std::unique_ptr<channel> _cchnl;
    std::function<void(std::shared_ptr<connect>)> _close_cb;
    std::function<void(std::shared_ptr<connect>)> _error_cb;
    std::function<void(std::shared_ptr<connect>, const std::string&)> _msg_cb;
    std::function<void(std::shared_ptr<connect>)> _send_cb;
    buffer _input_buf; // 接收缓冲区
    buffer _output_buf; // 发送缓冲区
    std::atomic_bool _disconn;
    timestamp _last_at_time;
};

}