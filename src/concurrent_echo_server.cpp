/**
 * ==================================================
 *  @file concurrent_echo_server.cpp
 *  @brief 并发回显服务器（多进程）
 *  @author ywj
 *  @date 2025-06-16 下午4:17
 *  @version 1.0
 *  @copyright Copyright (c) 2025 ywj. All Rights Reserved.
 * ==================================================
 */

#include <netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<signal.h>

//定义套接字描述符
int sockFd=-1;
//定义地址结构体
sockaddr_in servAddr;

void error_quit(const char*msg) {
    perror(msg);
    exit(1);
}

void sig_handler(int signo) {
    close(sockFd);
}


//子进程函数
/**
 *
 * @param clientSock client socket描述符
 * @param sock server socket描述符
 * @param clientAddr client 地址结构体
 */
void sonProcessFunc(int clientSock,int sock,sockaddr_in clientAddr) {
    char*buf= (char*)malloc(1024);
    memset(buf,0,1024);
    const char*clientAddrStr=inet_ntoa(clientAddr.sin_addr);
    const short clientPort=ntohs(clientAddr.sin_port);
    printf("process %d start work for client %s:%d\n",getpid(),clientAddrStr,clientPort);
    //循环读取
    for (;;) {
        size_t nread=read(clientSock,buf,1024);
        if (nread>0) {
            if (strcmp(buf,"quit")==0) {
                break;
            }
            printf("process %d recv data from client %s:%d-->%s\n",getpid(),clientAddrStr,clientPort,buf);
            //回显给客户端
            write(sock,buf,nread);
            memset(buf,0,nread);
        }
    }
    free(buf);
    close(clientSock);
}

int main(int argc,char**argv) {
    //注册信号处理函数
    signal(SIGINT,sig_handler);
    short port=9527;
    //初始化地址结构体
    memset(&servAddr,0,sizeof(servAddr));
    servAddr.sin_family=AF_INET;
    servAddr.sin_port=htons(port);
    servAddr.sin_addr.s_addr=htonl(INADDR_ANY);
    //创建socket描述符
    sockFd=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if (sockFd<0) {
        error_quit("create socket error!");
    }
    //创建socket成功，绑定端口
    if (bind(sockFd,(const sockaddr*)&servAddr,sizeof(servAddr))<0) {
        error_quit("bind error!");
    }
    //绑定成功，开始监听,等地啊队列长度为1024
    listen(sockFd,1024);
    for (;;) {
        //阻塞等待连接
        int clientFd=-1;
        socklen_t len;
        sockaddr_in clientAddr;
        clientFd=accept(sockFd,(sockaddr *)&clientAddr,&len);
        if (clientFd<0) {
           perror("accept error!\n" );
            continue;
        }else {
            printf("client %s:%d connect success!\n",inet_ntoa(clientAddr.sin_addr),ntohs(clientAddr.sin_port));
            //开启一个子进程
            pid_t pid=fork();

            if (pid==0) {
                //子进程
                printf("start a son process\n");
                sonProcessFunc(clientFd,clientFd,clientAddr);
            }else {
                //父进程
                printf("parent process:%d\n",pid);
            }
        }
    }


    return 0;
}