/**
 * ==================================================
 *  @file pthread_test.cpp
 *  @brief 使用pthread修改的并发echo_server
 *  @author ywj
 *  @date 2025-06-22 10:56
 *  @version 1.0
 *  @copyright Copyright (c) 2025 ywj. All Rights Reserved.
 * ==================================================
 */

#include<pthread.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<stdio.h>
#include<signal.h>
#include<time.h>
#include<string.h>

#define BUFFER_SIZE 1024
#define MAX_WAIT_QUEUE 10
#define PORT 9090
bool ifThreadRun=false;

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

//全局变量
int listenFd;

//定义NT信号处理函数
void handler_SIGNT(int signo)
{
    ifThreadRun=false;
    close(listenFd);
}

//定义TERM信号处理函数
void handler_SIGTERM(int signo)
{
    ifThreadRun=false;
    close(listenFd);
}



//定义线程处理函数--传入的参数是客户端套接字描述符
void* threadFunc(void*arg)
{
    if (arg==NULL)
    {
        pthread_exit(NULL);
        return NULL;
    }
    char buffer[BUFFER_SIZE]={0};
    int sockFd=*((int*)arg);
    while(ifThreadRun)
    {

        int nread=recv(sockFd,buffer,BUFFER_SIZE,0);
        if (nread==0)
        {
            //断开连接
            close(sockFd);
            sockaddr_in addr;
            socklen_t addrLen=sizeof(addr);
            getpeername(sockFd,(sockaddr*)&addr,&addrLen);
            LOG_ERROR("a client disconnected from server-->%s:%d\n",inet_ntoa(addr.sin_addr),ntohs(addr.sin_port));
            //退出线程
            pthread_exit(NULL);
        }
        else if (nread>0)
        {
            //处理可读数据
            sockaddr_in addr;
            socklen_t addrLen=sizeof(addr);
            getpeername(sockFd,(sockaddr*)&addr,&addrLen);
            LOG_INFO("recv data from client%s:%d-->%s\n",inet_ntoa(addr.sin_addr),ntohs(addr.sin_port),buffer);
            //echo
            send(sockFd,buffer,nread,0);
            memset(buffer,0,nread);
        }
    }
    close(sockFd);
    return NULL;
}

int main()
{
    //注册信号处理
    signal(SIGINT,handler_SIGNT);
    signal(SIGTERM,handler_SIGNT);

    //创建socket
    listenFd=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if (listenFd<0)
    {
        LOG_ERROR("create listenFd error!\n");
        exit(1);
    }
    //定义地址结构体
    sockaddr_in servAddr;
    sockaddr_in clientAddr;
    socklen_t sockLen=sizeof(clientAddr);
    servAddr.sin_family=AF_INET;
    servAddr.sin_addr.s_addr=htonl(INADDR_ANY);
    servAddr.sin_port=htons(9090);
    //设置套接字选项
    int opt=-1;
    // 设置地址可重用
    setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 设置端口复用（Linux 3.9+）
    setsockopt(listenFd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
    //绑定地址
    if (bind(listenFd,(sockaddr*)&servAddr,sizeof(servAddr))<0)
    {
        LOG_ERROR("bind error!\n");
        exit(1);
    }
    //监听
    if (listen(listenFd,MAX_WAIT_QUEUE)<0)
    {
        LOG_ERROR("listen error!\n");
        close(listenFd);
        exit(1);
    }
    ifThreadRun=true;
    for (;;)
    {
        int clientFd=accept(listenFd,(sockaddr*)&clientAddr,&sockLen);
        if (clientFd<0)
        {
            continue;
        }
        LOG_INFO("a new client connected to server success-->%s:%d\n",inet_ntoa(clientAddr.sin_addr),ntohs(clientAddr.sin_port));
        //创建一个新线程处理该客户端的读写
         pthread_t threadId;
        if (pthread_create(&threadId,NULL,&threadFunc,(void*)&clientFd)==0)
        {
            LOG_INFO("thread %lu start work for client %d\n",threadId,clientFd);
        }
    }
    return 0;
}