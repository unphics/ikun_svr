#pragma once

#include <string>

namespace last::net {

class buffer {
public:
    buffer(uint16_t sep = 1);
    ~buffer();
    void append(const char* data, size_t size);
    void append_with_sep(const char*data, size_t size);
    size_t size();
    const char* data();
    void clear();
    void erase(size_t pos, size_t size); // 从_buf的pos开始, 删除size个字节, pos从0开始
    bool pick_msg(std::string& str);
private:
    std::string _buf;
    const uint16_t _sep; // 报文的分隔符: 0-无分隔符(固定长度,视频会议); 1-四字节的报头; 2-"\r\n\r\n"分隔符(http协议)
};

}