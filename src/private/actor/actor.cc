#include "actor/actor.hh"

#include "essential_head.hh"
#include "actor/task.hh"


namespace last::actor {

actor::actor(): _stop(false), _def(def::actor) {
    
}
actor::~actor() {
    this->stop();
}
void actor::start() {
    print("actor start on thread:", ::syscall(SYS_gettid));
    this->session();
}
void actor::dispatch(std::unique_ptr<task> task) {
    {
        std::lock_guard<std::mutex> guard(this->_mutex);
        this->_queue.push(std::move(task));
    }
    this->_cond.notify_one();
}
void actor::stop() {
    if (this->_stop == true) {
        return;
    }
    this->_stop = true;
    this->_cond.notify_all();
}
uint32_t actor::load() {
    uint32_t load = 0;
    load += this->_queue.size();
    return load;
}
void actor::deal(std::unique_ptr<task> task) {
    print("[deal] thread", ::syscall(SYS_gettid),"deal task:", task->id);
    ::sleep(0.3);
}
void actor::session() {
    while (this->_stop == false) {
        std::unique_ptr<task> task;
        {
            std::unique_lock<std::mutex> lock(this->_mutex);
            this->_cond.wait(lock, [this] {
                return this->_stop == false && this->_queue.empty() == false;
            });
            if (this->_stop == true || this->_queue.empty() == true) {
                return;
            }
            task = std::move(this->_queue.front());
            this->_queue.pop();
        }
        this->deal(std::move(task));
    }
}
typename ::last::actor::def actor::def() {
    return this->_def;
}

}