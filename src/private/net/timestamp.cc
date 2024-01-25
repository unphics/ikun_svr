#include "net/timestamp.hh"

namespace last::net {

timestamp::timestamp() {
    this->_sec_since_epoch = ::time(0);
}
timestamp::timestamp(int64_t sec_since_epoch): _sec_since_epoch(sec_since_epoch) {

}
timestamp::~timestamp() {

}
timestamp timestamp::now() {
    return timestamp();
}
time_t timestamp::to_int() const {
    return this->_sec_since_epoch;
}
std::string timestamp::to_str() const {
    char buf[32] = {0};
    ::tm *tm_time = ::localtime(&this->_sec_since_epoch);
    snprintf(buf, 20, "%4d-%02d-%02d %02d:%02d:%02d",
        tm_time->tm_year + 1900,
        tm_time->tm_mon + 1,
        tm_time->tm_mday,
        tm_time->tm_hour,
        tm_time->tm_min,
        tm_time->tm_sec);
    return buf;
}

}