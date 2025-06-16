/**
* ==================================================
 *  @file concurrent_echo_server.cpp
 *  @brief 回显客户端
 *  @author ywj
 *  @date 2025-06-16 下午4:17
 *  @version 1.0
 *  @copyright Copyright (c) 2025 ywj. All Rights Reserved.
 * ==================================================
 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory>
#include <cstring>
#include <string>
#include <iostream>

void err_quit(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char **argv)
{
    // 参数传入ip和端口
    if (argc < 3)
    {
        err_quit("param too less!");
    }
    const char *servIp = argv[1];
    const char *servPortS = argv[2];
    short servPort = -1;
    servPort = atoi(servPortS);
    if (servPort < 0)
    {
        err_quit("port is incorrect!");
    }
    printf("connect to server %s:%d\n", servIp, servPort);

    // 定义套接字
    int sockFd;
    // 定义server地址结构体
    sockaddr_in servAddr;
    // 清空地址结构体
    memset(&servAddr, 0, sizeof(servAddr));
    // 给地址结构体复制
    servAddr.sin_family = AF_INET; // IPV4地址族
    servAddr.sin_port = htons(servPort); // 端口号转换为网络字节序
    inet_aton(servIp, &servAddr.sin_addr);
    if (servAddr.sin_addr.s_addr == INADDR_NONE)
    {
        err_quit("invalid ip!");
    }
    // 创建套接字
    sockFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockFd < 0)
    {
        err_quit("create socket error!");
    }
    // 建立连接
    if (connect(sockFd, (sockaddr *)&servAddr, sizeof(servAddr)) < 0)
    {
        err_quit("connnect error!");
    }
    // 连接成功之后等待输入
    char recvBuf[1024];
    for (;;)
    {
        printf("please input data to send:");
        // 等待输入
        std::string sendData;
        std::getline(std::cin, sendData);
        if (sendData == "quit")
        {
            close(sockFd);
            printf("success close connect,see you next time!");
            return 0;
        }
        // 发送
        // send(sockFd,sendData.c_str(),sendData.length());
        if (write(sockFd, sendData.c_str(), sendData.length()) <= 0)
        {
            close(sockFd);
            err_quit("send error!");
        }
        // 等待响应
        while (read(sockFd, recvBuf, 1024) > 0)
        {
            printf("recvData:%s\n", recvBuf);
            break;
        }
    }
    return 0;
}
