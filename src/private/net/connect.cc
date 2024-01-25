#include "net/connect.hh"

#include "essential_head.hh"
#include "net/channel.hh"
#include "net/poller.hh"
#include "net/socket.hh"
#include "net/eventloop.hh"

#include <unistd.h>
#include <cstring>

namespace last::net {

connect::connect(eventloop* loop, std::unique_ptr<socket> csock): _loop(loop), _csock(std::move(csock)),
    _disconn(false), _cchnl(new channel(loop, this->_csock->fd())) {
    // this->_cchnl = std::make_unique<channel>(loop, this->_csock->fd());
    this->_cchnl->set_read_cb(std::bind(&connect::on_msg, this));
    this->_cchnl->set_close_cb(std::bind(&connect::on_close, this));
    this->_cchnl->set_error_cb(std::bind(&connect::on_error, this));
    this->_cchnl->set_write_cb(std::bind(&connect::writecb, this));
    this->_cchnl->use_et();
    this->_cchnl->enable_read();
}
connect::~connect() {
    print("connect::~connect()");
}
uint32_t connect::fd() const {
    return this->_csock->fd();
}
std::string connect::ip() const {
    return this->_csock->ip();
}
uint32_t connect::port() const {
    return this->_csock->port();
}
void connect::on_close() {
    print("client close");
    this->_disconn = true;
    this->_cchnl->remove();
    if (this->_close_cb) {
        this->_close_cb(this->shared_from_this());
    }
}
void connect::on_error() {
    this->_disconn = true;
    this->_cchnl->remove();
    if (this->_error_cb) {
        this->_error_cb(this->shared_from_this());
    }
}
void connect::set_close_cb(const std::function<void(std::shared_ptr<connect>)>& cb) {
    this->_close_cb = cb;
}
void connect::set_error_cb(const std::function<void(std::shared_ptr<connect>)>& cb) {
    this->_error_cb = cb;
}
void connect::set_msg_cb(const std::function<void(std::shared_ptr<connect>, const std::string&)>& cb) {
    this->_msg_cb = cb;
}
void connect::set_send_cb(const std::function<void(std::shared_ptr<connect>)>& cb) {
    this->_send_cb = cb;
}
void connect::on_msg() {
    char buf[1024];
    while (1) {
        ::memset(&buf, 0, sizeof(buf));
        int nread = ::read(this->fd(), buf, sizeof(buf));
        if (nread > 0) {
            int len = 0;
            ::memcpy(&len, buf, 4);
            this->_input_buf.append(buf, len + 4);
        } else if (nread == -1 && errno == EINTR) { // 读取数据时被信号中断, 继续读
            continue;
        } else if (nread == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) { // 全部数据已经读取完毕
            std::string msg;
            while (1) {
                if (this->_input_buf.pick_msg(msg) == false) {
                    break;
                }
                // printf("msg : %s, size: %i\n", msg.data(), msg.size());
                print("recv msg, size:", msg.size());
                this->_last_at_time = timestamp::now();
                // std::cout << "last at time: " << this->_last_at_time.to_str() << std::endl;
                if (this->_msg_cb) {
                    this->_msg_cb(this->shared_from_this(), msg);
                }
            }
            break;
        } else if (nread == 0) {
            // 客户端连接已经断开
            print("recv client disconnected");
            this->on_close();
            break;
        } else {
            print("recv 什么情况");
        }
    }
}
void connect::send(char* data, size_t size) {
    if (this->_disconn == true) {
        print("client has been disconnected !!!");
        return;
    }
    if (this->_loop->is_in_loop_thd()) {
        // 如果当前线程是io线程, 直接执行发送数据的操作
        this->send_in_loop(data, size);
    } else {
        // 如果当前线程不是io线程, 把发送数据的操作交给io线程执行
        // TODO 此处bind捕获的char*会丢失指向, 暂时这样写, 后面改成buffer对象
        char* a = new char[30];
        memcpy(a, data, 30);
        this->_loop->add(std::bind(&connect::send_in_loop, this, a, size));
    }
}
void connect::send_in_loop(char* data, size_t size) {
    print("send size:", size, ", info:", data);
    this->_output_buf.append(data, size);
    this->_cchnl->enable_write();
}
void connect::writecb() {
    int writen = ::send(this->fd(), this->_output_buf.data(), this->_output_buf.size(), 0);
    if (writen > 0) {
        this->_output_buf.erase(0, this->_output_buf.size());
    }
    if (this->_output_buf.size() == 0) {
        this->_cchnl->disenable_write();
        if (this->_send_cb) {
            this->_send_cb(this->shared_from_this());
        }
    }
}
bool connect::time_out(::time_t now, int judge) {
    return now - this->_last_at_time.to_int() > judge;
}

}