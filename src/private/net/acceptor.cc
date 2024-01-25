#include "net/acceptor.hh"

#include "essential_head.hh"
#include "net/eventloop.hh"
#include "net/channel.hh"
#include "net/socket.hh"
#include "net/connect.hh"

namespace last::net {

acceptor::acceptor(eventloop* loop, const std::string& ip, const uint32_t& port): _loop(loop) {
    this->_lsock = new last::net::socket(last::net::socket::create_nonb());
    this->_lsock->set_reuse_addr();
    this->_lsock->set_reuse_port();
    this->_lsock->set_tcp_no_delay();
    this->_lsock->set_keep_alive();

    last::net::inetaddr laddr(ip, port);

    this->_lsock->bind(laddr);
    this->_lsock->listen();

    this->_lchnl = new channel(this->_loop, this->_lsock->fd());
    this->_lchnl->set_read_cb(std::bind(&acceptor::new_conn, this));
    this->_lchnl->enable_read();
}
acceptor::~acceptor() {
    // delete this->_lsock;
    delete this->_lchnl;
}
void acceptor::new_conn() {
    print("[acceptor] : new conn !");
    inetaddr caddr;
    std::unique_ptr<socket> csock = std::make_unique<socket>(this->_lsock->accept(caddr));
    // socket* csock = new socket(this->_lsock->accept(caddr));
    csock->set_ip_port(caddr.ip(), caddr.port());
    if (this->new_conn_cb) {
        this->new_conn_cb(std::move(csock));
    }
}
void acceptor::set_new_conn_cb(const std::function<void(std::unique_ptr<socket>)>& cb) {
    this->new_conn_cb = cb;
}

}