/**
 * ==================================================
 *  @file writev_readv_test.cpp
 *  @brief epoll结合readv_writev模拟http获取文件
 *  @author ywj
 *  @date 2025-06-26 21:41
 *  @version 1.0
 *  @copyright Copyright (c) 2025 ywj. All Rights Reserved.
 * ==================================================
 */
#include<sys/epoll.h>
#include<sys/socket.h>
#include<pthread.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<time.h>
#include<signal.h>
#include<sys/uio.h>
#include<string>
#include<fstream>
#include<sstream>
#include<fcntl.h>

#define LOG_INFO(format,...) \
do{\
 time_t now=time(NULL);\
 char timeStr[20];\
 tm*time_info=localtime(&now);\
 strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", time_info);\
 printf("[INFO %s] " format "\n",timeStr,##__VA_ARGS__);\
}while (0)

#define LOG_ERROR(format,...) \
do{\
time_t now=time(NULL);\
char timeStr[20];\
tm*time_info=localtime(&now);\
strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", time_info);\
printf("[ERROR %s] " format "\n",timeStr,##__VA_ARGS__);\
}while (0)

#define WAIT_QUEUE 10
#define BUF_SIZE 1024
#define EPOLL_TABLE_SIZE 1024
#define MAX_EVENT_SIZE 1024
int listenFd;
int epFd;
bool if_server_running = false;
int client_fd_list[EPOLL_TABLE_SIZE];
epoll_event events[MAX_EVENT_SIZE];

const char* avilable_file_list[2] = {"/home/ywj/test_filedir/test1.txt", "/home/ywj/test_filedir/test2.txt"};
const char* status_line[2] = {"200 OK", "500 Server Internal Error"};

void initSockAddr(sockaddr_in* addrPtr, const char* ip, short port)
{
    addrPtr->sin_family = AF_INET;
    addrPtr->sin_addr.s_addr = inet_addr(ip);
    addrPtr->sin_port = htons(port);
}

int setnonblocking(int fd) {
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

void addfd(int epollfd, int fd, bool enable_et) {
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN;
    if(enable_et) {
        event.events |= EPOLLET;
    }
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setnonblocking(fd);
}

void et(int epFd, int listenFd, epoll_event* events, int event_num)
{
    for (int i = 0; i < event_num; i++)
    {
        //new connection
        if (events[i].data.fd == listenFd)
        {
            sockaddr_in client_addr;
            bzero(&client_addr, sizeof(client_addr));
            socklen_t addr_size = sizeof(client_addr);
            int clientFd = -1;
            clientFd = accept(listenFd, (sockaddr*)&client_addr, &addr_size);
            //add clientFd to epoll
            if (clientFd < 0)
            {
                LOG_ERROR("accept error!");
            }
            else
            {
                addfd(epFd,clientFd, true);
                LOG_INFO("new connection accept success-->%s:%d", inet_ntoa(client_addr.sin_addr),
                         ntohs(client_addr.sin_port));
            }
        }
        else
        {
            //client fd
            int clientFd = events[i].data.fd;
            //read all
            char buffer[BUF_SIZE] = {0};
            int total = 0;
            while (true)
            {
                int nread = read(clientFd, buffer + total,BUF_SIZE - total);
                if (nread<=0)
                {
                    break;
                }
                total += nread;
                if (total >= BUF_SIZE )
                {
                    break;
                }
            }
            if (total == 0)
            {
                //client disconenct this packet is FIN
                close(clientFd);
                LOG_INFO("one client disconnected from server");
            }
            else
            {
                LOG_INFO("accpet msg from client-->%s", buffer);
                //response msg
                for (int i = 0; i < 2; i++)
                {
                    if (strcmp(avilable_file_list[i], buffer) == 0)
                    {
                        iovec bufs[2];
                        bufs[0].iov_base = (void*)status_line[0];
                        bufs[0].iov_len = strlen(status_line[0]);
                        std::ifstream ifs(avilable_file_list[i]);
                        std::ostringstream oss;
                        oss << ifs.rdbuf();
                        std::string fileconent = oss.str();
                        bufs[1].iov_base = (void*)fileconent.c_str();
                        bufs[1].iov_len = fileconent.length();
                        //one time write muti buf to one distination file description
                        writev(clientFd, bufs, 2);
                        return;
                    }
                }
                const char*err_rsp="you have no permission to this file!";
                write(clientFd,err_rsp,strlen(err_rsp));
            }
        }
    }
}


void safe_quit()
{
    if_server_running = false;
    shutdown(listenFd,SHUT_RDWR);
    close(epFd);
    LOG_INFO("safe quit...");
    exit(0);
}

//hangle NT signal
void handle_signal_NT(int signo)
{
    safe_quit();
}

void handle_signal_TERM(int signo)
{
    safe_quit();
}

void handle_signal_PIPE(int signo)
{
    safe_quit();
}

int main(int argc, char** argv)
{
    signal(SIGPIPE, handle_signal_PIPE);
    signal(SIGINT, handle_signal_NT);
    signal(SIGTERM, handle_signal_TERM);
    sockaddr_in serv_addr, client_addr;
    bzero(&serv_addr, sizeof(serv_addr));
    bzero(&client_addr, sizeof(client_addr));
    socklen_t addr_size = sizeof(serv_addr);
    int clientFd = -1;
    char* ip = "0.0.0.0";
    short port = 9091;
    if (argc > 2)
    {
        ip = argv[1];
        port = atoi(argv[2]);
    }
    initSockAddr(&serv_addr, ip, port);
    listenFd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    //bind
    if (bind(listenFd, (sockaddr*)&serv_addr, addr_size) < 0)
    {
        LOG_ERROR("bind error!");
        close(listenFd);
        exit(0);
    }
    //listen
    if (listen(listenFd,WAIT_QUEUE) < 0)
    {
        LOG_ERROR("listen error!");
        close(listenFd);
        exit(0);
    }
    //init epoll
    epFd = epoll_create(EPOLL_TABLE_SIZE);
    if (epFd < 0)
    {
        LOG_ERROR("create epoll  error!");
        close(listenFd);
        exit(0);
    }
    //add listenFd to epoll

    addfd(epFd, listenFd, true);
    //epoll wait
    while (true)
    {
        //blocking wait
        int event_num = epoll_wait(epFd, events,MAX_EVENT_SIZE, -1);
        if (event_num > 0)
        {
            //边缘触发
            et(epFd, listenFd, events, event_num);
        }
    }
    safe_quit();
    return 0;
}
