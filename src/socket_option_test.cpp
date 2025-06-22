/**
 * ==================================================
 *  @file udp_client.cpp
 *  @brief 常用socket选项设置的示例代码
 *  @author ywj
 *  @date 2025-06-18 22:36
 *  @version 1.0
 *  @copyright Copyright (c) 2025 ywj. All Rights Reserved.
 * ==================================================
 */
 
 //注意：设置端口复用/地址复用的时候要在bind之前设置
 //      设置接受缓冲区/发送缓冲区的时候要在listen之前设置
 #include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main() {
    int sockfd;
    struct sockaddr_in addr;

    // 创建 TCP 套接字
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    // 设置 SO_REUSEADDR 和 SO_REUSEPORT
    int enable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0) {
        perror("setsockopt(SO_REUSEADDR) failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(enable)) < 0) {
        perror("setsockopt(SO_REUSEPORT) failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // 设置接收缓冲区大小（单位：字节）
    int rcvbuf = 256 * 1024; // 256 KB
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &rcvbuf, sizeof(rcvbuf)) < 0) {
        perror("setsockopt(SO_RCVBUF) failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // 设置发送缓冲区大小（单位：字节）
    int sndbuf = 256 * 1024; // 256 KB
    if (setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &sndbuf, sizeof(sndbuf)) < 0) {
        perror("setsockopt(SO_SNDBUF) failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // 绑定地址与端口
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("192.168.0.100"); // 替换为你的 IP
    addr.sin_port = htons(8888);                       // 指定监听端口

    if (bind(sockfd, (const struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Socket configured successfully: address/port reuse enabled, buffer size set.\n");

    // 监听（仅用于 TCP）
    listen(sockfd, 5);

    printf("Listening on port 8888...\n");

    // 程序保持运行（演示用）
    pause();

    close(sockfd);
    return 0;
}