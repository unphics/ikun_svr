#include <iostream>

#include <arpa/inet.h> // 包含了socket
#include <unistd.h>
#include <cstring>
#include <thread>
#include "client_test.pb.h"

void work(int idx) {
 // 1.创建通信的套接字
    int fd = socket(/*ipv4*/AF_INET, /*流式协议*/SOCK_STREAM, /*tcp*/0);
    if (fd == -1) {
        std::cout << "failed to create fd !!!" << std::endl;
        return ;
    }
    // 2.链接服务端
    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(8765); // 需要htons转换成大端
    inet_pton(AF_INET, "127.0.0.1", &saddr.sin_addr.s_addr);
    int ret = connect(fd, /*ip&port*/(struct sockaddr*)&saddr, sizeof(saddr));
    if (ret == -1) {
        std::cout << "failed to connet !!!" << std::endl;
        return ;
    }
    // 3. 与链接成功的客户端进行通信
    int num = 0;
    while (1) {
        // 发送数据
        {
            char buf[1020];
            // sprintf(buf, "client_%i:  hello, %d ...", idx, num++);

            client_test::req req;
            client_test::protocol_header header;
            header.set_protocol_name("req");
            req.mutable_header()->CopyFrom(header);
            req.set_id(123);
            num++;
            req.set_name("client_" + std::to_string(idx) + " hello, " + std::to_string(num));
            char tmp[1024];
            std::string byte_ser;
            req.SerializeToString(&byte_ser);

            // int len = strlen(buf);
            int len = req.ByteSize();
            memcpy(tmp, &len, 4); // 拼接报文头部
            memcpy(tmp + 4, byte_ser.c_str(), len); // 拼接报文内容
            send(fd, tmp, len + 4, 0);
        }

        {
            // 接收数据
            char buf[1020];
            ::memset(&buf, 0, sizeof(buf));
            int len = recv(fd, buf, sizeof(buf), 0);
            if (len > 0) {
                std::cout << "client_" << idx << " : server say: " << buf << std::endl;
            } else if (len == 0) {
                std::cout << "服务端已经断开了链接 !!!" << std::endl;
                break;
            } else {
                std::cout << "recv" << std::endl;
                break;
            }
        }

        sleep(1);
    }
    // 4.关闭文件描述符
    close(fd);
}

int main(int argc, char** argv) {

    for (int i = 0; i < 4; ++i) {
        printf("add new client to connect !!!\n");
        std::thread thd(work, i);
        thd.detach();
        sleep(3);
    }

    sleep(20);
    return 0;
}