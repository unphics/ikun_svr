#pragma once

#include <netinet/in.h>
#include <string>
#include <iostream>

namespace last::net {

class inetaddr {
public:
    inetaddr(const std::string& ip, uint32_t port); // 监听的fd就用这个构造函数
    inetaddr(const struct sockaddr_in addr); // 客户端连上来的fd, 就用这个构造函数
    inetaddr();
    ~inetaddr();
    void set_addr(const struct sockaddr_in& addr);
    const char* ip() const; // 返回字符串表示的地址 如192.168.1.1
    uint16_t port() const; // 返回整数表示的端口 如9999
    const struct sockaddr* addr() const; // 返回_addr成员的地址, 转换成了struct sockaddr
private:
    struct sockaddr_in _addr; // 地址协议的结构体
};

}