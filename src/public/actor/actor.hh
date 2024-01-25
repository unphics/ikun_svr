#ifndef last_actor_actor
#define last_actor_actor

#include "def.hh"

#include <queue>
#include <condition_variable>
#include <mutex>
#include <functional>
#include <atomic>
#include <memory>

namespace last::actor {

class task;

class actor {
public:
    actor();
    virtual ~actor();
    virtual void start();
    virtual void dispatch(std::unique_ptr<task> task);
    virtual void stop();
    uint32_t load(); // 负载程度
    virtual void deal(std::unique_ptr<task> task);
    typename ::last::actor::def def();
private:
    virtual void session();
    std::atomic_bool _stop;
    std::queue<std::unique_ptr<task>> _queue;
    std::condition_variable _cond;
    std::mutex _mutex;
    typename ::last::actor::def _def = def::actor;
};

}

#endif