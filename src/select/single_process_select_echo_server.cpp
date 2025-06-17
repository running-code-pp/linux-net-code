/**
 * ==================================================
 *  @file single_process_select_echo_server.cpp
 *  @brief 单进程select echo server
 *  @author ywj
 *  @date 2025-06-17 下午12:24
 *  @version 1.0
 *  @copyright Copyright (c) 2025 ywj. All Rights Reserved.
 * ==================================================
 */


#include <csignal>
#include <cstring>
#include <sys/select.h>
#include <sys/types.h>
#include <stdio.h>
#include<unistd.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<time.h>

int servSockFd;

// 定义 LOG 宏
#define LOG(format, ...) \
do { \
time_t now = time(NULL); \
struct tm *tm_info = localtime(&now); \
char timestamp[20]; \
strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info); \
printf("[%s] " format "\n", timestamp, ##__VA_ARGS__); \
} while (0)

void error_quit(const char *msg) {
    perror(msg);
    exit(1);
}

//退出信号处理
void exit_handler(int signo) {
    LOG("Received signal %d, shutting down server...", signo);
    close(servSockFd);
}



int main() {
    //关机信号
    signal(SIGTERM,exit_handler);
    //ctrl+c终止信号
    signal(SIGINT,exit_handler);

    int clientSock = -1, servSock = -1,maxFd=-1,maxIndex=-1,nready=0;
    int clientFds[FD_SETSIZE];
    for (int i=0;i<FD_SETSIZE;i++) {
        clientFds[i]=-1;
    }
    char buffer[FD_SETSIZE];
    memset(buffer,0,1024);
    sockaddr_in servAddr, clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    fd_set allset, rset;
    FD_ZERO(&allset);
    FD_ZERO(&rset);
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(9629);
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servSock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if (servSock<0) {
        error_quit("create socket failed");
    }
    if (bind(servSock,(sockaddr*)&servAddr,sizeof(servAddr))) {
        error_quit("bind failed");
    }
    if (listen(servSock,1024)) {
        error_quit("listen failed");
    }
    servSockFd=servSock;
    //将监听套接字描述符添加到集合
    FD_SET(servSock,&allset);
    maxFd=servSock;
    for (;;) {
        rset =allset;
        //阻塞直到有可读描述符出现
        nready=select(maxFd+1,&rset,NULL,NULL,NULL);
        //判断是否是servSock可读
        if (FD_ISSET(servSock,&rset)) {
            nready--;
            //接受客户端连接
            clientSock=accept(servSock,(sockaddr*)&clientAddr,&clientAddrLen);
            if (clientSock < 0) {
                LOG("accept failed!");
                continue;
            }else {
                printf("client %s:%d connect success!\n",inet_ntoa(clientAddr.sin_addr),ntohs(clientAddr.sin_port));
                //将客户端套接字添加到集合中
                if (maxIndex>=FD_SETSIZE-1) {
                    LOG("too many clients connected");
                    close(clientSock);
                    continue;
                }
                FD_SET(clientSock,&allset);
                int index=0;
                for (;index<FD_SETSIZE;index++) {
                    if (clientFds[index]==-1) {
                        clientFds[index]=clientSock;
                        break;
                    }
                }
                if (index==FD_SETSIZE) {
                    LOG("too many clients connected");
                    close(clientSock);
                    continue;
                }
                maxFd=maxFd> clientSock? maxFd : clientSock;
                if (index>maxIndex) {
                    maxIndex=index;
                }
            }
        }

        //遍历客户端连接文件描述符
        for (int i=0;i<=maxIndex;i++) {
            if (nready<=0) {
                break;;
            }
            if (FD_ISSET(clientFds[i],&rset)) {
                //客户端连接可读
                memset(buffer,0,1024);
                size_t nread=read(clientFds[i],buffer,1024);
                if (nread==0) {
                    //连接关闭
                    close(clientSock);
                    FD_CLR(clientFds[i],&allset);
                    LOG("one client disconnect from server");
                }else {
                    //有数据可读
                    LOG("recv data from client %s",buffer);
                    //回显给客户端
                    write(clientFds[i],buffer,nread);
                }
                nready--;
            }
        }
    }
    close(servSock);
    return 0;
}
