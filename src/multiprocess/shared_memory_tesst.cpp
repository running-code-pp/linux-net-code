/**
 *@brief : 共享内存实现 聊天室server
 */

#include <sys/mman.h>
#include<sys/shm.h>
#include<sys/epoll.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<fcntl.h>
#include<vector>
#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>

#define MAX_WAIT_QUEUE 1024;
#define MAX_EPOLL_EVENTS 1024;
#define MAX_PROCESS_LIMIT 65535;
#define MAX_USER_SIZE 1024;

struct user_client
{
    sockaddr client_addr;
    int pipe_fd[2];//用于和父进程通信的双向管道
    int conn_fd; //客户端链接socket描述符
    int child_pid;//子进程pid
};

int set_fd_nonblock(int fd)
{
    int old_option = fcntl(fd,F_GETFL);
    fcntl(fd,F_SETFL, old_option | O_NONBLOCK);
    return old_option;
}

void add_fd_to_epoll(int ep_fd, int fd)
{
    epoll_event ev;
    ev.data.fd = fd;
    ev.events = EPOLLIN | EPOLLET;
    epoll_ctl(ep_fd, EPOLL_CTL_ADD, fd, &ev);
}

// 全局变量
int sigpipe_fd[2]={0};//传输信号的管道
int conn_fd=0;//接入客户端连接的socket文件描述符
std::vector<user_client> users;
char* shm_mem = NULL; //共享内存映射的地址
const char* shm_name = "/chat_server_shm"; //共享内存对象名称
int ep_fd = 0; //epoll文件描述符

// 初始化操作
void init()
{
    //映射共享内存
    shm_mem = mmap(NULL,BUFSIZ*MAX_USER_SIZE,PROT_READ|PROT_WRITE, MAP_SHARED,,0);
}

//释放资源
void release_resource()
{
    close(sigpipe_fd[0]);
    close(sigpipe_fd[1]);
    close(ep_fd);
    close(conn_fd);

}
int main(int argc, char** argv)
{
    // shm_open()
    return 0;
}
