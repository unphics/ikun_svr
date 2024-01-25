#ifndef last_actor_task
#define last_actor_task

#include <cstdint>

namespace last::actor {

class task {
public:
    task() = default;
    virtual ~task() = default;
    uint64_t id;
private:
};

}

#endif