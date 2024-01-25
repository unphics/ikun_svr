#include "net/inetaddr.hh"

#include <arpa/inet.h> // inet_addr
// #include <sock

namespace last::net {

inetaddr::inetaddr(const std::string& ip, uint32_t port) {
    this->_addr.sin_family = AF_INET; // ipv4
    this->_addr.sin_addr.s_addr = inet_addr(ip.c_str());
    this->_addr.sin_port = htons(port);
}
inetaddr::inetaddr(const struct sockaddr_in addr): _addr(addr) {}
inetaddr::inetaddr() {}
inetaddr::~inetaddr() {}
void inetaddr::set_addr(const struct sockaddr_in& addr) {
    this->_addr = addr;
}
const char* inetaddr::ip() const {
    return ::inet_ntoa(this->_addr.sin_addr);
}
uint16_t inetaddr::port() const {
    return ::ntohs(this->_addr.sin_port);
}
const struct sockaddr* inetaddr::addr() const {
    return (struct sockaddr*)&this->_addr;
}

}