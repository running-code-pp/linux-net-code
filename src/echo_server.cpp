#include<sys/socket.h>
#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include<memory.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<signal.h>


void error_quit(const char*msg){
    perror(msg);
    exit(1);
}

int main(int argc,char**argv){
    //定义server套接字和client套接字
    int sockFd=-1,clientFd=-1;
    //定义地址结构体
    sockaddr_in clientAddr,servAddr;
    //初始化地址结构体
    memset(&clientAddr,0,sizeof(clientAddr));
    memset(&servAddr,0,sizeof(servAddr));
    servAddr.sin_addr.s_addr=htonl(INADDR_ANY);//可以接受任意ip发起的连接
    servAddr.sin_family=AF_INET;//ipv4
    servAddr.sin_port=htons(9595);
    //创建套接字
    sockFd=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if(sockFd<0){
        error_quit("create socket error!");
    }
    //绑定地址结构体给套接字
    if(bind(sockFd,(sockaddr*)&servAddr,sizeof(servAddr))<0){
        error_quit("bind error!");
    }
    //监听
    listen(sockFd,1024);
    for(;;){
        //阻塞等待连接
        if((clientFd=accept(sockFd,NULL,NULL))<0)//不需要关注client地址信息后面两个参数可以置空
        {
            close(sockFd);
            error_quit("accept error!");
        }
        printf("client connected!\n");
        //读取缓冲区
        char buf[1024]={0};
        int nread=read(clientFd,buf,1024);
        if(nread>0){
        printf("recvData:%s\n",buf);
        //回显给client
        write(clientFd,buf,nread);
        }else{
            // close(sockFd);
            close(clientFd);
            // error_quit("read error!");
            continue;
        }
    }
    return 0;
}