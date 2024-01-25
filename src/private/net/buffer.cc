#include "net/buffer.hh"

#include <cstring>

namespace last::net {

buffer::buffer(uint16_t sep): _sep(sep) {}
buffer::~buffer() {}
void buffer::append(const char* data, size_t size) {
    this->_buf.append(data, size);
}
void buffer::append_with_sep(const char*data, size_t size) {
    if (this->_sep == 0) {
        this->_buf.append(data, size);    
    } else if (this->_sep == 1) {
        this->_buf.append((char*)&size, 4);
        this->_buf.append(data, size);
    }
    
}
size_t buffer::size() {
    return this->_buf.size();
}
const char* buffer::data() {
    return this->_buf.data();
}
void buffer::clear() {
    this->_buf.clear();
}
void buffer::erase(size_t pos, size_t size) {
    this->_buf.erase(pos, size);
}
bool buffer::pick_msg(std::string& str) {
    if (this->_buf.size() == 0) {
        return false;
    }
    if (this->_sep == 0) {
        str = this->_buf;
        this->_buf.clear();
    } else if (this->_sep == 1) {
        int len = 0;
        ::memcpy(&len, this->_buf.data(), 4);
        if (this->_buf.size() < len + 4) {
            return false;
        }
        str = this->_buf.substr(4, len);
        this->_buf.erase(0, len + 4);
    }
    return true;
}

}