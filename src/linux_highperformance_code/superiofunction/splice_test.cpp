/**
 * ==================================================
 *  @file splice_test.cpp
 *  @brief splice+select实现零拷贝echoserver
 *  @author ywj
 *  @date 2025-06-30 01:09
 *  @version 1.0
 *  @copyright Copyright (c) 2025 ywj. All Rights Reserved.
 * ==================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/types.h>

#define MAX_CLIENTS 1024
#define PORT        8080

int main(void) {
    int server_fd, client_fds[MAX_CLIENTS] = {0};
    fd_set readfds;
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);

    // 创建监听 socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // 设置地址可重用
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 绑定
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);
    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 监听
    if (listen(server_fd, SOMAXCONN) < 0) {
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Echo server started on port %d\n", PORT);

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        int max_fd = server_fd;

        // 添加已连接的客户端
        for (int i = 0; i < MAX_CLIENTS; ++i) {
            if (client_fds[i] > 0)
                FD_SET(client_fds[i], &readfds);
            if (client_fds[i] > max_fd)
                max_fd = client_fds[i];
        }

        // select 等待
        int activity = select(max_fd + 1, &readfds, NULL, NULL, NULL);
        if (activity < 0 && errno != EINTR) {
            perror("select error");
            continue;
        }

        // 新连接
        if (FD_ISSET(server_fd, &readfds)) {
            int new_fd = accept(server_fd, (struct sockaddr *)&addr, &addrlen);
            if (new_fd < 0) {
                perror("accept");
                continue;
            }

            // 设置非阻塞
            int flags = fcntl(new_fd, F_GETFL, 0);
            fcntl(new_fd, F_SETFL, flags | O_NONBLOCK);

            // 添加到客户端数组
            for (int i = 0; i < MAX_CLIENTS; ++i) {
                if (client_fds[i] == 0) {
                    client_fds[i] = new_fd;
                    break;
                }
            }
        }

        // 处理客户端数据
        for (int i = 0; i < MAX_CLIENTS; ++i) {
            int fd = client_fds[i];
            if (fd <= 0 || !FD_ISSET(fd, &readfds))
                continue;

            // 创建管道
            int pipefd[2];
            if (pipe(pipefd) < 0) {
                perror("pipe");
                close(fd);
                client_fds[i] = 0;
                continue;
            }

            // splice 数据从 socket 到 pipe
            ssize_t bytes_spliced;
            while ((bytes_spliced = splice(fd, NULL, pipefd[1], NULL, 32768, SPLICE_F_MOVE)) > 0) {
                // splice 数据从 pipe 回 socket
                splice(pipefd[0], NULL, fd, NULL, bytes_spliced, SPLICE_F_MOVE);
            }

            if (bytes_spliced < 0 && errno != EAGAIN) {
                // 客户端断开连接或出错
                close(fd);
                close(pipefd[0]);
                close(pipefd[1]);
                client_fds[i] = 0;
            } else if (bytes_spliced == 0) {
                // 对端关闭
                close(fd);
                close(pipefd[0]);
                close(pipefd[1]);
                client_fds[i] = 0;
            }
        }
    }

    close(server_fd);
    return 0;
}