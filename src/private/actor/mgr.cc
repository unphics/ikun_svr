#include "actor/mgr.hh"

#include "actor/task.hh"

namespace last::actor {

mgr::mgr() {

}
void mgr::send(def def, std::unique_ptr<task> task) {
    uint32_t idx = 0;
    for (uint32_t i = 0; i < this->_actors[def].size(); ++i) {
        if (this->_actors[def][i]->load() < this->_actors[def][idx]->load()) {
            idx = i;
        }
    }
    this->_actors[def][idx]->dispatch(std::move(task));
}

}