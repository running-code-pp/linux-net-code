/**
 * ==================================================
 *  @file signal_super.cpp
 *  @brief 信号的高级用法--统一事件源
 *  @author ywj
 *  @date 2025-07-09 23:44
 *  @version 1.0
 *  @copyright Copyright (c) 2025 ywj. All Rights Reserved.
 * ==================================================
 */

#include <cstring>
#include<signal.h>
#include<sys/epoll.h>
#include<sys/types.h>
#include<time.h>
#include<stdio.h>
#include<stdlib.h>
#include <strings.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<fcntl.h>
#include<errno.h>
#include<assert.h>

#define LOG_INFO(format,...) \
do{ \
time_t now=time(NULL);\
tm*time_info=localtime(&now);\
char time_str[20];\
strftime(time_str,sizeof(time_str),"%Y-%m-%d %H:%M:%S",time_info);\
printf("[INFO %s] " format " \n",time_str,##__VA_ARGS__);\
} while (0)

#define LOG_ERROR(format,...) \
do{ \
time_t now=time(NULL);\
tm*time_info=localtime(&now);\
char time_str[20];\
strftime(time_str,sizeof(time_str),"%Y-%m-%d %H:%M:%S",time_info);\
printf("[ERROR %s] " format "\n",time_str,##__VA_ARGS__);\
} while (0)

// pipe
static int pipe_fd[2];
static bool if_server_running = false;
int epfd,listen_fd;
#define MAX_EVENTS 1024
#define EPOLL_SIZE 1024
#define MAX_WAIT_QUEUE 1024

void signal_handler(int sig)
{
    // 确保信号处理函数不会影响主线程的errno
    int save_errno=errno;
    int msg=sig;
    send(pipe_fd[1], &msg, sizeof(msg), 0);
    errno=save_errno;
}

void register_signal(int sig)
{
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler=signal_handler;
    action.sa_flags=SA_RESTART;
    sigfillset(&action.sa_mask);
    assert(sigaction(sig,&action,NULL)!=-1);
}

int set_fd_noblock(int fd)
{
    int old_option=fcntl(fd,F_GETFL);
    int new_option=old_option|O_NONBLOCK;
    fcntl(fd,F_SETFL,new_option);
    return old_option;
}

void add_fd_epoll(int ep_fd,int fd,bool if_et=false)
{
    epoll_event ev{};
    ev.data.fd=fd;
    ev.events=EPOLLIN;
    if (if_et)
    {
        ev.events|=EPOLLET;
    }
    set_fd_noblock(fd);
    epoll_ctl(ep_fd,EPOLL_CTL_ADD,fd,&ev);
}

void epoll_et(int ep_fd,int event_num,epoll_event*events)
{
    for (int i=0;i<event_num;i++)
    {
        epoll_event event=events[i];
        if (event.data.fd==listen_fd)
        {
            //accpet client
            int client_fd=accept(listen_fd,NULL,NULL);
            LOG_INFO("a new client connect server");
            add_fd_epoll(ep_fd,client_fd,event.events);
        }
        else if (event.data.fd==pipe_fd[0])
        {
            // signal
            int sigs[1024];
            int num = read(pipe_fd[0], sigs, sizeof(sigs));
            for (int j = 0; j < num / sizeof(int); j++)
            {
                switch (sigs[j])
                {
                    case SIGCHLD:
                    case SIGHUP:
                        {
                            continue;
                        }
                    case SIGTERM:
                    case SIGINT:
                        {
                            if_server_running=false;
                        }

                }
            }
        }
        else
        {
            // client
            char buffer[1024]={0};
            int total=0;
            while(true)
            {
                  int num=recv(event.data.fd,buffer+total,sizeof(buffer)-total,0);
                  if (num<=0)
                  {
                      break;
                  }
                  total+=num;
                  if (total>=sizeof(buffer))
                  {
                      break;
                  }
            }
            if (total==0)
            {
                //client disconnect
                LOG_INFO("a client disconected from server");
            }
            else
            {
                LOG_INFO("recv data from client: %s",buffer);
            }
        }
    }
}

int main(int argc,char**argv)
{
   if (argc<2)
   {
       LOG_ERROR("usage : %s  <port>",argv[0]);
       return 1;
   }
    sockaddr_in serv_addr;
    bzero(&serv_addr,sizeof(serv_addr));
    socklen_t addr_len=sizeof(serv_addr);
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_port=htons(atoi(argv[1]));
    serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);

    listen_fd=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if (listen_fd<0)
    {
        LOG_ERROR("socket failed");
        return 1;
    }

    if (bind(listen_fd,(sockaddr*)&serv_addr,sizeof(serv_addr))<0)
    {
        LOG_ERROR("bind failed");
        close(listen_fd);
        return 1;
    }

    if (listen(listen_fd,MAX_WAIT_QUEUE)<0)
    {
        LOG_ERROR("listen failed");
        close(listen_fd);
        return 1;
    }
    // create pipe
    if (socketpair(PF_UNIX,SOCK_STREAM,0,pipe_fd)<0)
    {
        LOG_ERROR("create pipe failed");
        close(listen_fd);
        return 1;
    }

    // create epoll
    epfd=epoll_create(EPOLL_SIZE);
    add_fd_epoll(epfd,listen_fd,true);
    add_fd_epoll(epfd,pipe_fd[0],true);
    set_fd_noblock(pipe_fd[1]);
    epoll_event events[EPOLL_SIZE];
    if_server_running=true;
    register_signal(SIGCHLD);
    register_signal(SIGHUP);
    register_signal(SIGTERM);
    register_signal(SIGINT);
    while (if_server_running)
    {
        int num=epoll_wait(epfd,events,EPOLL_SIZE,-1);
        if (num<=0)
        {
            continue;
        }
        epoll_et(epfd,num,events);
    }
    shutdown(listen_fd,SHUT_RDWR);
    close(pipe_fd[0]);
    close(pipe_fd[1]);
    return 0;
}