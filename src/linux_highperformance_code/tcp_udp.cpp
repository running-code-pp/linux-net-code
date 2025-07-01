/**
 * ==================================================
 *  @file tcp_udp.cpp
 *  @brief 同一个端口同时监听tcp和udp
 *  @author ywj
 *  @date 2025-07-01 22:08
 *  @version 1.0
 *  @copyright Copyright (c) 2025 ywj. All Rights Reserved.
 * ==================================================
 */

#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>
#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<sys/epoll.h>
#include<signal.h>
#include<fcntl.h>

#define BUF_SIZE 1024
#define WAIT_QUEUE 1024
#define MAX_EVENTS 1024
#define MAX_FD_SIZE 1024

#define LOG_INFO(format,...) \
do{ \
time_t now=time(NULL); \
tm*time_info=localtime(&now); \
char time_str[20]={0};  \
strftime(time_str,sizeof(time_str),"%Y%m%d %H:%M:%S",time_info); \
printf("[INFO %s] " format " \n",time_str,##__VA_ARGS__); \
}while(0)

#define LOG_ERROR(format,...) \
do{ \
time_t now=time(NULL); \
tm*time_info=localtime(&now); \
char time_str[20]={0}; \
strftime(time_str,sizeof(time_str),"%Y%m%d %H:%M:%S",time_info); \
printf("[ERROR %s] " format " \n",time_str,##__VA_ARGS__); \
}while(0)
void add_fd_epoll(int ep_fd, int fd);
void remove_fd_epoll(int ep_fd, int fd);

int ep_fd = -1, tcp_fd = -1, udp_fd = -1;
bool if_server_running = false;

void safe_quit()
{
    if_server_running=false;
    remove_fd_epoll(ep_fd, tcp_fd);
    remove_fd_epoll(ep_fd, udp_fd);
    shutdown(tcp_fd,SHUT_RDWR);
    shutdown(udp_fd,SHUT_RDWR);
    close(ep_fd);
    exit(0);
}

void handle_signal(int sig_no)
{
    switch (sig_no)
    {
    case SIGINT:
    case SIGTERM:
    case SIGPIPE:
        safe_quit();
    }
}

void set_nonblock(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd,F_SETFL, flags | O_NONBLOCK);
}

void add_fd_epoll(int ep_fd, int fd)
{
    if (fd < 0)
        return;
    set_nonblock(fd);
    //set mode et
    epoll_event ev;
    ev.data.fd = fd;
    ev.events = EPOLLIN;
    ev.events |= EPOLLET;
    epoll_ctl(ep_fd,EPOLL_CTL_ADD, fd, &ev);
}

void remove_fd_epoll(int ep_fd, int fd)
{
    epoll_event ev;
    ev.data.fd = fd;
    ev.events = EPOLLIN;
    epoll_ctl(ep_fd,EPOLL_CTL_DEL, fd, &ev);
}

int main(int argc, char** argv)
{
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    signal(SIGPIPE, handle_signal);
    short port=0;
    if (argc < 2)
    {
        // printf("Usage: ./tcp_udp <Port>\n");
        // return 0;
        port=8080;
    }
    else
    {
        port=atoi(argv[1]);
    }
    sockaddr_in serv_addr;
    socklen_t addr_len = sizeof(serv_addr);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    tcp_fd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    udp_fd = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
    if (tcp_fd < 0 || udp_fd < 0)
    {
        LOG_ERROR("create socket failed!");
        safe_quit();
    }
    //bind
    if (bind(tcp_fd, (sockaddr*)&serv_addr, addr_len) < 0 || bind(udp_fd, (sockaddr*)&serv_addr, addr_len) < 0)
    {
        LOG_ERROR("bind failed!\n");
        safe_quit();
    }
    //listen
    listen(tcp_fd,WAIT_QUEUE);
    listen(udp_fd,WAIT_QUEUE);
    //create epoll
    ep_fd = epoll_create(MAX_FD_SIZE);
    if (ep_fd < 0)
    {
        LOG_ERROR("epoll_create failed!\n");
        safe_quit();
    }
    epoll_event events[MAX_EVENTS];
    add_fd_epoll(ep_fd, tcp_fd);
    add_fd_epoll(ep_fd, udp_fd);
    if_server_running = true;
    while (if_server_running)
    {
        //blocking wait
        int event_num = epoll_wait(ep_fd, events,MAX_EVENTS, -1);
        for (int i = 0; i < event_num; i++)
        {
            epoll_event ev = events[i];
            //accpet tcp connection
            if (ev.data.fd == tcp_fd)
            {
                sockaddr_in client_addr;
                socklen_t addr_len = sizeof(client_addr);
               int client_fd= accept(tcp_fd, (sockaddr*)&client_addr, &addr_len);
                LOG_INFO("tcp: new connection accpet:%s:%d", inet_ntoa(client_addr.sin_addr),
                         ntohs(client_addr.sin_port));
                add_fd_epoll(ep_fd,client_fd);
            }
            else if (ev.data.fd == udp_fd)
            {
                sockaddr_in client_addr;
                socklen_t addr_len = sizeof(client_addr);
                char buffer[BUF_SIZE] = {0};
                int nread = recv(udp_fd, buffer, sizeof(buffer),0);
                if (nread > 0)
                {
                    LOG_INFO("recv data from udp client:%s", buffer);
                }
            }
            else if (ev.events&EPOLLIN)
            {
                char buffer[BUF_SIZE] = {0};
                int total = 0;
                while (true)
                {
                    int nread = recv(ev.data.fd, buffer + total, sizeof(buffer) - total, 0);
                    if (total+nread > BUF_SIZE || nread <= 0)
                    {
                        break;
                    }
                    total+=nread;
                }
                if (total <= 0)
                {
                    remove_fd_epoll(ep_fd, ev.data.fd);
                    LOG_INFO("tcp client disconnected!");
                }
                else
                {
                    LOG_INFO("recv data from tcp cient:%s", buffer);
                }
            }
        }
    }
    safe_quit();
    return 0;
}
