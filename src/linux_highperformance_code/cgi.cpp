/**
 * ==================================================
 *  @file cgi.cpp
 *  @brief cgi实现原理模拟
 *  @author ywj
 *  @date 2025-06-24 23:35
 *  @version 1.0
 *  @copyright Copyright (c) 2025 ywj. All Rights Reserved.
 * ==================================================
 */

#include<stdio.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<time.h>
#include<signal.h>
#include<unistd.h>
#include<sys/uio.h>

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


int listenFd=-1;
#define WAIT_QUEUE_LEN 10

//NT信号处理
void handleSIGNT(int signo)
{
    shutdown(listenFd, SHUT_RDWR);
    exit(0);
}

//TERM信号处理
void handleSIGTERM(int signo)
{
    shutdown(listenFd, SHUT_RDWR);
    exit(0);
}

int main(int argc,char**argv)
{
    char*ip=NULL;
    short port;
    if(argc<3){
        // error_quit("Usage: ./cgi_modify <host> <port>");
        ip="0.0.0.0";
        port=9094;
    }else{
        ip=argv[1];
        port=atoi(argv[2]);
    }
    //注册信号
    signal(SIGTERM, handleSIGTERM);
    signal(SIGINT, handleSIGTERM);
    signal(SIGPIPE,handleSIGNT);

    sockaddr_in servAddr;
    servAddr.sin_family=AF_INET;
    inet_aton(ip,&servAddr.sin_addr);
    servAddr.sin_port=htons(port);
    listenFd=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    //bind
    if (bind(listenFd,(sockaddr*)&servAddr,sizeof(servAddr))<0)
    {
        error_quit("bind error!");
    }
    //listen
    if (listen(listenFd,WAIT_QUEUE_LEN)<0)
    {
        error_quit("listen error!");
    }


    int clientFd=accept(listenFd,NULL,NULL);
    if (clientFd>0)
    {
        close(STDOUT_FILENO);
        //udp创建一个同样指向监听套接字的文件描述符
        //需要注意的是第一这里是用clientFd,第二用dup2(clietenFd,STD_FILENO)更好
        dup(clientFd);//1:因为标准输出对应的文件描述符就是1
        close(clientFd);
        while (true)
        {
            char buffer[1024];
            int nread=recv(STDOUT_FILENO,buffer,sizeof(buffer),0);
            if (nread<0)
            {
                close(listenFd);
                shutdown(listenFd,SHUT_RDWR);
                exit(0);
            }
            else
            {

                //调用标准输出--》直接回显给客户端
                //增加换行符
                buffer[nread]='\n';
                printf(buffer);
                fflush(stdout);
            }
        }
    }
    shutdown(listenFd,SHUT_RDWR);

}