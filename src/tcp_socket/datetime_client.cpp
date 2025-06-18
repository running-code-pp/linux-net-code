/**
 * @File    :   tcp1.cpp
 * @Time    :   2025/06/03 13:00:43
 * @Author  :   ppywj
 * @Version :   1.0
 * @Desc    :   一个简单的时间获取客户程序
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include<unistd.h>

#define MAXLINE 4096

void err_quit(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char const *argv[])
{
    int sockfd, n; // 定义套接字文件描述符
    char recvline[MAXLINE + 1];
    struct sockaddr_in servaddr;
    if (argc != 3)
    {
        err_quit("usage: tcp1 <IPaddress> <Port>");
    }
    // 创建一个tcp套接字 指定地址族为IPV4(AF_INET) 指定为流式协议(SOCK_STREAM) 协议通常设置为0让系统自动选择
    if (sockfd = socket(AF_INET, SOCK_STREAM, 0) < 0)
    {
        err_quit("create socket error");
    }
    // 创建server 地址结构体

    memset(&servaddr, 0, sizeof(servaddr)); // 初始化servaddr为0
    servaddr.sin_family=AF_INET; // 设置地址族为IPv4
    servaddr.sin_port = htons(atoi(argv[2]));  // 设置端口号为12345，使用网络字节序

    //将字符串地址转为二进制格式并储存在servaddr.sin_addr中
    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)
    {
        char errMsg[1024];
        snprintf(errMsg, sizeof(errMsg), "inet_pton error for %s", argv[1]);
        err_quit(errMsg);
    }
    //链接服务器

    if (connect(sockfd, (const sockaddr*)&servaddr, sizeof(servaddr)) < 0){
        err_quit("connect error");
    }
    
    while((n = read(sockfd,recvline,MAXLINE))>0){
        recvline[n]=0;
        if(fputs(recvline,stdout)==EOF){
            err_quit("fputs error");
        }
    }
    if(n<0){
        err_quit("read error");
    }

    return 0;
}
