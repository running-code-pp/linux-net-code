/**
* ==================================================
 *  @file writev_readv_test.cpp
 *  @brief epoll+sendfile文件下载服务器
 *  @author ywj
 *  @date 2025-06-27 21:41
 *  @version 1.0
 *  @copyright Copyright (c) 2025 ywj. All Rights Reserved.
 * ==================================================
 */

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <fcntl.h>

#define PORT 8080
#define MAX_EVENTS 1024
#define BUFFER_SIZE 1024

// 设置非阻塞
void set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL);
    if (flags == -1) flags = 0;
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int main() {
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == -1) {
        perror("socket");
        return 1;
    }

    // 设置地址可重用
    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("bind");
        close(listen_fd);
        return 1;
    }

    if (listen(listen_fd, SOMAXCONN) == -1) {
        perror("listen");
        close(listen_fd);
        return 1;
    }

    int epfd = epoll_create1(0);
    if (epfd == -1) {
        perror("epoll_create1");
        close(listen_fd);
        return 1;
    }

    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;  // ET 边缘触发
    ev.data.fd = listen_fd;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, listen_fd, &ev) == -1) {
        perror("epoll_ctl: listen_fd");
        close(listen_fd);
        close(epfd);
        return 1;
    }

    struct epoll_event events[MAX_EVENTS];

    std::cout << "Server is running on port " << PORT << std::endl;

    while (true) {
        int n_events = epoll_wait(epfd, events, MAX_EVENTS, -1);
        for (int i = 0; i < n_events; ++i) {
            if (events[i].data.fd == listen_fd) {
                // 新连接
                struct sockaddr_in client_addr;
                socklen_t client_len = sizeof(client_addr);
                int conn_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &client_len);
                if (conn_fd != -1) {
                    set_nonblocking(conn_fd);
                    struct epoll_event conn_ev;
                    conn_ev.events = EPOLLIN | EPOLLET;
                    conn_ev.data.fd = conn_fd;
                    if (epoll_ctl(epfd, EPOLL_CTL_ADD, conn_fd, &conn_ev) == -1) {
                        perror("epoll_ctl: conn_fd");
                        close(conn_fd);
                    }
                }
            } else {
                // 数据就绪
                int conn_fd = events[i].data.fd;
                char buffer[BUFFER_SIZE];
                ssize_t bytes_read = read(conn_fd, buffer, sizeof(buffer) - 1);

                if (bytes_read <= 0) {
                    // 客户端关闭或出错
                    close(conn_fd);
                    epoll_ctl(epfd, EPOLL_CTL_DEL, conn_fd, nullptr);
                    continue;
                }

                buffer[bytes_read] = '\0';
                std::cout << "Received path: " << buffer << std::endl;

                // 打开文件
                int file_fd = open(buffer, O_RDONLY);
                if (file_fd == -1) {
                    const char* msg = "File not found\n";
                    write(conn_fd, msg, strlen(msg));
                    close(conn_fd);
                    epoll_ctl(epfd, EPOLL_CTL_DEL, conn_fd, nullptr);
                    continue;
                }

                // 获取文件大小
                struct stat stat_buf;
                fstat(file_fd, &stat_buf);
                off_t offset = 0;
                ssize_t sent_bytes = sendfile(conn_fd, file_fd, &offset, stat_buf.st_size);
                if (sent_bytes == -1) {
                    perror("sendfile");
                }

                close(file_fd);
                close(conn_fd);
                epoll_ctl(epfd, EPOLL_CTL_DEL, conn_fd, nullptr);
            }
        }
    }

    close(listen_fd);
    close(epfd);
    return 0;
}