#include "net/socket.hh"

#include "essential_head.hh"

#include <arpa/inet.h> // socket
#include <iostream> // print
#include <netinet/tcp.h>  // TCP_NODELAY
#include <unistd.h> // close
#include <cstring> // strerror
#include <cstdlib> // exit 和 abort

namespace last::net {

socket::socket(const uint32_t& fd): _fd(fd) {
    if (this->_fd < 0) {
        print("failed to create socket fd !!!");
        printf("file: %s, function: %s, line: %d, errno: %s\n", __FILE__, __FUNCTION__, __LINE__, strerror(errno));
        ::exit(-1);
    }
}
socket::~socket() {
    ::close(this->_fd);
}
uint32_t socket::create() {
    return ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
}
uint32_t socket::create_nonb() {
    return ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
}
bool socket::bind(const inetaddr& addr) {
    if (::bind(this->_fd, addr.addr(), sizeof(struct ::sockaddr)) == -1) {
        print("failed to bind socket !!!");
        return false;
    }
    this->set_ip_port(addr.ip(), addr.port());
    return true;
}
bool socket::listen(int num) {
    if (::listen(this->_fd, num) < 0) {
        print("failed to listen socket !!!");
        return false;
    }
    return true;
}
uint32_t socket::accept(inetaddr& out_addr) {
    struct sockaddr_in caddr;
    socklen_t len = sizeof(caddr);
    int cfd = ::accept4(this->_fd, (struct sockaddr*)&caddr, &len, SOCK_NONBLOCK);
    out_addr.set_addr(caddr);
    this->_ip = out_addr.ip();
    this->_port = out_addr.port();
    return cfd;
}
uint32_t socket::fd() const {
    return this->_fd;
}
void socket::set_reuse_addr(bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(this->_fd, SOL_SOCKET, SO_REUSEADDR, &optval, static_cast<socklen_t>(sizeof(optval))); // 必须
}
void socket::set_reuse_port(bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(this->_fd, SOL_SOCKET, TCP_NODELAY, &optval, static_cast<socklen_t>(sizeof(optval))); // 必须
}
void socket::set_tcp_no_delay(bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(this->_fd, SOL_SOCKET, SO_REUSEPORT, &optval, static_cast<socklen_t>(sizeof(optval))); // 有用, 但是在reactor中意义不大
}
void socket::set_keep_alive(bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(this->_fd, SOL_SOCKET, SO_KEEPALIVE, &optval, static_cast<socklen_t>(sizeof(optval))); // 可能有用, 但是建议自己做心跳
}
std::string socket::ip() const {
    return this->_ip;
}
uint32_t socket::port() const {
    return this->_port;
}
void socket::set_ip_port(const std::string& ip, uint32_t port) {
    this->_ip = ip;
    this->_port = port;
}

}