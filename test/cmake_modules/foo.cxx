module;
#include <iostream>
export module hello;
export struct hello {
    hello() {
        std::cout << "hello construct" << std::endl;
    }
    ~hello() {
        std::cout << "hello desstruct" << std::endl;
    }
    void hello_world() {
        std::cout << "hello world" << std::endl;
    }
}