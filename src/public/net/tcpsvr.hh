#pragma once

#include <string>
#include <cstdint>
#include <map>
#include <functional>
#include <memory>
#include <mutex>

namespace last::net {

class socket;
class acceptor;
class connect;
class eventloop;
class threadpool;

class tcpsvr {
public:
    tcpsvr(const std::string& ip, uint32_t port, uint32_t thd_count);
    ~tcpsvr();
    void start();
    void shut_down();
    void set_new_conn_cb(const std::function<void(std::shared_ptr<connect>)>& cb);
    void set_conn_close_cb(const std::function<void(std::shared_ptr<connect>)>& cb);
    void set_conn_error_cb(const std::function<void(std::shared_ptr<connect>)>& cb);
    void set_on_msg_cb(const std::function<void(std::shared_ptr<connect>, const std::string&)>& cb);
    void set_post_send_cb(const std::function<void(std::shared_ptr<connect>)>& cb);
    void set_time_out_cb(const std::function<void(eventloop*)>& cb);
    void remove_conn(uint32_t fd);
private:
    std::unique_ptr<eventloop> _main_loop;
    acceptor* _acceptor;
    std::map<int, std::shared_ptr<connect>> _conns;
    std::vector<std::unique_ptr<eventloop>> _sub_loops;
    uint32_t _thd_count;
    threadpool* _thd_pool;
    std::mutex _mutex;

    void _on_new_conn(std::unique_ptr<socket> csock);
    void _on_conn_close(std::shared_ptr<connect> conn);
    void _on_conn_error(std::shared_ptr<connect> conn);
    void _on_msg(std::shared_ptr<connect> conn, const std::string& msg);
    void _on_post_send(std::shared_ptr<connect> conn);
    void _on_time_out(eventloop* loop);

    std::function<void(std::shared_ptr<connect>)> _new_conn_cb;
    std::function<void(std::shared_ptr<connect>)> _conn_close_cb;
    std::function<void(std::shared_ptr<connect>)> _conn_error_cb;
    std::function<void(std::shared_ptr<connect>, const std::string&)> _on_msg_cb;
    std::function<void(std::shared_ptr<connect>)> _post_send_cb;
    std::function<void(eventloop*)> _time_out_cb;
};

}