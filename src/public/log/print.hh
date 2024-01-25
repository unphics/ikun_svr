#ifndef last_log_print
#define last_log_print

#include "logger.hh"

#include <iostream>
#include <chrono>
#include <sstream>

template<typename T>
void _rec_cout(std::ostringstream& oss, T t) {
    oss << " " << t;
}

template<typename T, typename... Args>
void _rec_cout(std::ostringstream& oss, T t, Args... args) {
    oss << " " << t;
    _rec_cout(oss, args...);
}

template<typename... Args>
void print(Args... args) {
    auto now = std::chrono::system_clock::now(); // 当前系统时间点
    std::time_t now_time = std::chrono::system_clock::to_time_t(now) - 50; // 转换为时间戳, 这个50不知道是啥, 可能是北京和珠海的时差?
    std::tm* local_time = std::localtime(&now_time);
    std::ostringstream oss;
    oss << "["
        << local_time->tm_year + 1900 << "/"
        << local_time->tm_mon + 1 << "/"
        << local_time->tm_mday << " "
        << local_time->tm_hour << ":"
        << local_time->tm_min << ":"
        << local_time->tm_sec
        <<"] Print :";
    _rec_cout(oss, args...);
    std::cout << oss.str() << std::endl;
    last::log::logger::get_inst()->push_log(oss);
}

#endif