#ifndef last_actor_mgr
#define last_actor_mgr

#include "essential_head.hh"
#include "actor/actor.hh"
#include "def.hh"

#include <vector>
#include <map>
#include <memory>
#include <type_traits>
#include <thread>

namespace last::actor {

class task;
class actor;

class mgr {
public:
    mgr();
    template<typename T, typename = std::enable_if_t<std::is_base_of_v<actor, T>>>
    void create() {
        std::jthread jthd([this]() mutable {
            std::unique_ptr<T> unique = std::make_unique<T>();
            // if (unique->def() == def::actor) {
            //     return;
            // }
            ::last::actor::def def = unique->def();
            this->_actors[def].push_back(std::move(unique));
            this->_actors[def][this->_actors[def].size() - 1]->start();
        });
        jthd.detach();
    }
    void send(def def, std::unique_ptr<task> task);
private:
    std::map<def, std::vector<std::unique_ptr<actor>>> _actors;
};

}

#endif