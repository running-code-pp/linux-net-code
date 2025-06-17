#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<time.h>
#include<memory>
#include <string.h>
#define MAXLINE 4096
#define MAXWAITQUEUE 1024

void err_quit(const char*msg){
    perror(msg);
    exit(1);
}

int main(int argc, char const *argv[])
{
    //定义一个监听套接字和客户端连接套接字
    int listenfd,connfd;
    //定义监听地址结构体
    struct sockaddr_in servaddr;
    //接受缓冲区
    char buff[MAXLINE];
    time_t ticks;
    //创建套接字如果失败则会返回负数
    if((listenfd=socket(AF_INET,SOCK_STREAM,0))<0){
        err_quit("create listenfd error");
    }
    
    //清空地址结构体
    memset(&servaddr,0,sizeof(servaddr));
    //设置地址组/地址/端口
    servaddr.sin_family= AF_INET; // 设置地址族为IPv4
    servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
    servaddr.sin_port=htons(12345);
    //绑定端口和地址给套接字 绑定失败返回负数
    if(bind(listenfd,(const sockaddr*)&servaddr,sizeof(servaddr))<0){
        err_quit("bind error");
    }
    //监听，可以定义最长等待队列长度
    if(listen(listenfd,MAXWAITQUEUE)<0)
    {
     err_quit("listen error");
    }
    
    for(;;){
        connfd=accept(listenfd,NULL,NULL);
        ticks=time(NULL);
        snprintf(buff,sizeof(buff),"%.24s\r\n",ctime(&ticks));
        write(connfd,buff,strlen(buff));
        close(connfd);
    }
    
    return 0;
}
