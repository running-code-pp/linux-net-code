/**
 * ==================================================
 *  @file udp_client.cpp
 *  @brief TODO 描述该文件的功能
 *  @author ywj
 *  @date 2025-06-18 22:36
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
}
//关机信号
void handler_TERM(int signo)
{
    LOG_INFO("process exit!\n");
    close(udp_sock);
}

int main(int argc,char**argv)
{
    signal(SIGINT,handler_NT);
    signal(SIGTERM,handler_NT);
    //接收方的地址
    sockaddr_in accAddr;
    accAddr.sin_family=AF_INET;
    inet_aton("192.168.1.3",&accAddr.sin_addr);
    accAddr.sin_port=htons(9696);
    udp_sock=socket(AF_INET,SOCK_DGRAM,0);
    //本机地址
    sockaddr_in addr;
    addr.sin_family=AF_INET;
    addr.sin_port=htons(9658);
    addr.sin_addr.s_addr=htonl(INADDR_ANY);
    // char buffer[1024];
    const char*data="hello world!";

    if (bind(udp_sock,(sockaddr*)&addr,sizeof(addr))<0)
    {
        LOG_ERROR("bind error!\n");
        exit(-1);
    }
    sendto(udp_sock,data,strlen(data),0,(struct sockaddr *)&accAddr,sizeof(accAddr));
    close(udp_sock);
    return 0;
}