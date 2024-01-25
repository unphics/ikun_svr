#pragma once

#include "inetaddr.hh"
#include <cstdint>

namespace last::net {

class socket {
public:
    socket(const uint32_t& fd);
    ~socket();
    static uint32_t create();
    static uint32_t create_nonb();
    bool bind(const inetaddr& addr);
    bool listen(int num = 128);
    uint32_t accept(inetaddr& out_addr);
    void set_reuse_addr(bool on = true);
    void set_reuse_port(bool on = true);
    void set_tcp_no_delay(bool on = true);
    void set_keep_alive(bool on = true);
    uint32_t fd() const;
    std::string ip() const;
    uint32_t port() const;
    void set_ip_port(const std::string& ip, uint32_t port);
private:
    const uint32_t _fd;
    std::string _ip;
    uint32_t _port;
};

}