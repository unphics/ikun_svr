#ifndef last_log_logger
#define last_log_logger

#include <string>
#include <sstream>
#include <mutex>

namespace last::log {

class logger {
public:
    static logger*  get_inst();
    void init();
    std::string get_log_file_url();
    void push_log(std::ostringstream& oss);
private:
    logger();
    inline static logger* _singleton {nullptr};
    std::string _log_file_name;
    std::mutex _mutex;
};

}

#endif