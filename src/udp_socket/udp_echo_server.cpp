/**
 * ==================================================
 *  @file udp_example.cpp
 *  @brief udp套接字基本编程
 *  @author ywj
 *  @date 2025-06-18 22:00
 *  @version 1.0
 *  @copyright Copyright (c) 2025 ywj. All Rights Reserved.
 * ==================================================
 */
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<stdlib.h>
#include<time.h>
#include<string.h>
#include<stdio.h>
#include<unistd.h>
#include<signal.h>

#define LOG_INFO(format, ...) \
do { \
time_t now = time(NULL); \
struct tm *tm_info = localtime(&now); \
char timestamp[20]; \
strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info); \
printf("[INFO %s] " format "\n", timestamp, ##__VA_ARGS__); \
} while (0)

#define LOG_ERROR(format, ...) \
do { \
time_t now = time(NULL); \
struct tm *tm_info = localtime(&now); \
char timestamp[20]; \
strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info); \
printf("[ERROR %s] " format "\n", timestamp, ##__VA_ARGS__); \
} while (0)

//定义socket
int udp_sock=-1;

//中断信号
void handler_NT(int signo)
{
    LOG_INFO("process exit!\n");
    close(udp_sock);
    exit(1);
}
//关机信号
void handler_TERM(int signo)
{
    LOG_INFO("process exit!\n");
    close(udp_sock);
    exit(1);
}

int main(int argc,char**argv)
{
    signal(SIGINT,handler_NT);
    signal(SIGTERM,handler_NT);
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr=htonl(INADDR_ANY);
    addr.sin_port=htons(9625);
    sockaddr_in clientAddr;
    socklen_t addrLen;

    //创建套接字
    udp_sock=socket(AF_INET,SOCK_DGRAM,0);
    if (udp_sock<0)
    {
        LOG_ERROR("sock description create error!");
        exit(1);
    }
    //绑定
    if (bind(udp_sock,(sockaddr*)&addr,sizeof(addr))<0)
    {
        LOG_ERROR("bind error!");
        exit(1);
    }
    //循环接受数据
    char buffer[1024];
    for (;;)
    {
        memset(buffer,0,sizeof(buffer));
        //阻塞
       int nread=recvfrom(udp_sock,buffer,sizeof(buffer),0,(sockaddr*)&clientAddr,&addrLen);
        if (nread>0)
        {
            LOG_INFO("recv data from %s:%d-->%s",inet_ntoa(clientAddr.sin_addr),ntohs(clientAddr.sin_port),buffer);
        }
        else
        {
            close(udp_sock);
        }
    }
    close(udp_sock);
    return 0;
}