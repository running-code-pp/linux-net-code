/**
 *@brief : 共享内存实现 聊天室server
 */

#include <assert.h>
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
#include<string.h>
#include<sys/wait.h>
#include<chrono>
#include <unordered_map>
#include <bitset>
#include<sys/signalfd.h>

#define MAX_WAIT_QUEUE 1024
#define MAX_EPOLL_EVENTS 1024
#define MAX_PROCESS_LIMIT 65535
#define MAX_USER_SIZE 4
#define BUFFER_SIZE 1024

struct user_client;

// 全局变量
int sig_pipe_fd = 0; //信号绑定的文件描述符
int listen_fd = 0; //接入客户端连接的socket文件描述符
std::unordered_map<int, user_client> user_map; //储存子进程和userdata的映射
std::bitset<MAX_USER_SIZE> mem_use_set; // 内存块是否被占用
char* shm_mem = NULL; //共享内存映射的地址
const char* shm_name = "/chat_server_shm"; //共享内存对象名称
int ep_fd = 0; //epoll文件描述符
int shm_fd = 0;
bool stop_server = false;
bool if_run = true; //子进程的运行标志

// 获取空闲的内存块
int get_free_mem()
{
    for (int i = 0; i < mem_use_set.size(); i++)
    {
        if (!mem_use_set[i])
        {
            return i;
        }
    }
    return -1;
}

//前向声明
sockaddr_in serv_addr;
void release_resource();
void init_resource();

#define LOG(format,...) \
do{\
auto now = std::chrono::system_clock::now(); \
std::time_t now_time_t = std::chrono::system_clock::to_time_t(now); \
std::tm now_tm = *std::localtime(&now_time_t); \
std::ostringstream ss; \
ss << std::put_time(&now_tm, "%Y-%m-%d %H:%M:%S"); \
printf("[%s] [%d]" format " \n",ss.str().c_str(),getpid(),##__VA_ARGS__);\
}while (0)

struct user_client
{
    sockaddr_in client_addr;
    int pipe_fd[2]; //用于和父进程通信的双向管道
    int listen_fd; //客户端链接socket描述符
    int child_pid; //子进程pid
    size_t memIdx; //共享内存中分配的内存块的索引
};

int set_fd_nonblock(int fd)
{
    int old_option = fcntl(fd,F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd,F_SETFL, new_option);
    return old_option;
}

void add_fd_to_epoll(int ep_fd, int fd)
{
    set_fd_nonblock(fd);
    epoll_event ev;
    ev.data.fd = fd;
    ev.events = EPOLLIN | EPOLLET;
    epoll_ctl(ep_fd, EPOLL_CTL_ADD, fd, &ev);
}


// 初始化操作
void init()
{
    // 创建共享内存对象
    shm_fd = shm_open(shm_name,O_CREAT | O_RDWR, 0666);
    assert(shm_fd != -1);
    int ret = ftruncate(shm_fd, MAX_USER_SIZE * BUFFER_SIZE);
    assert(ret != -1);
    //映射共享内存
    shm_mem = (char*)mmap(NULL, BUFFER_SIZE * MAX_USER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    assert(shm_mem != MAP_FAILED);
    close(shm_fd); //关闭本进程对该文件描述符的引用但不会释放掉共享内存 只有shm_unlink才会

    // 初始化监听套接字
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(9527);
    listen_fd = socket(AF_INET,SOCK_STREAM, 0);
    if (listen_fd == -1)
    {
        LOG("create listen socket failed!\n");
        release_resource();
        exit(1);
    }
    ret = bind(listen_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    if (ret == -1)
    {
        LOG("bind listen socket failed!\n");
        release_resource();
        exit(1);
    }
    ret = listen(listen_fd,MAX_WAIT_QUEUE);
    if (ret == -1)
    {
        LOG("listen listen socket failed!\n");
        release_resource();
        exit(1);
    }

    // 创建epoll
    ep_fd = epoll_create(MAX_EPOLL_EVENTS);
    if (ep_fd == -1)
    {
        LOG("create epoll failed!\n");
        release_resource();
        exit(1);
    }

    // 创建信号文件描述符
    sigset_t mask;
    sigemptyset(&mask);

    // 添加信号到掩码集
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGTERM);
    sigaddset(&mask, SIGCHLD);
    sigaddset(&mask, SIGPIPE);
    // 设置该进程为对应的信号集的内容（当前已经的信号集合做并集、交集、覆盖）
    sigprocmask(SIG_BLOCK, &mask, NULL);

    // 创建 signalfd 句柄（绑定信号）
    sig_pipe_fd = signalfd(-1, &mask, SFD_CLOEXEC | SFD_NONBLOCK);
    add_fd_to_epoll(ep_fd, sig_pipe_fd);


    // 添加信号管道读端到epoll中
    add_fd_to_epoll(ep_fd, listen_fd);
    add_fd_to_epoll(ep_fd, sig_pipe_fd);
    set_fd_nonblock(sig_pipe_fd);
    LOG("main process %d init success", getpid());
}

//释放资源
void release_resource()
{
    close(sig_pipe_fd);
    close(ep_fd);
    close(listen_fd);
    shm_unlink(shm_name);
}

// 边缘触发读取封装
int read_et(char* buffer, size_t max_size, int fd)
{
    int total = 0;
    while (true)
    {
        int read_size = recv(fd, buffer + total, max_size - total, 0);
        if (read_size <= 0)
        {
            return total > 0 ? total : read_size;
        }
        total += read_size;
        if (total >= max_size)
        {
            return total;
        }
    }
    return -1;
}


void run_child(int client_fd, user_client user_data)
{
    sigset_t mask;
    sigaddset(&mask,SIGTERM);
    sigaddset(&mask,SIGPIPE);
    sigprocmask(SIG_BLOCK, &mask, NULL);
    int child_signal_fd = signalfd(-1, &mask, 0);
    if (child_signal_fd == -1)
    {
        close(client_fd);
        return;
    }
    set_fd_nonblock(child_signal_fd);

    user_data.child_pid = getpid();
    close(user_data.pipe_fd[0]);
    int pipe_fd = user_data.pipe_fd[1];
    std::stringstream addr_ss;
    addr_ss << inet_ntoa(user_data.client_addr.sin_addr) << ":" << ntohs(user_data.client_addr.sin_port);
    // 关闭从主进程复制过来的不需要的资源
    close(ep_fd);
    close(listen_fd);
    close(sig_pipe_fd);
    // 分配的共享内存的起始地址
    char* buffer = shm_mem + user_data.memIdx * BUFFER_SIZE;

    LOG("a new child process %d start work for client %s", getpid(), addr_ss.str().c_str());

    // 创建epoll
    int epoll_fd = epoll_create(MAX_EPOLL_EVENTS);
    if (epoll_fd == -1)
    {
        LOG("create epoll failed!");
        return;
    }
    add_fd_to_epoll(epoll_fd, client_fd);
    add_fd_to_epoll(epoll_fd, pipe_fd);
    add_fd_to_epoll(epoll_fd, child_signal_fd);
    epoll_event events[MAX_EPOLL_EVENTS];
    while (if_run)
    {
        int ev_num = epoll_wait(epoll_fd, events, MAX_EPOLL_EVENTS, -1);
        if ((ev_num < 0) && (errno != EINTR))
        {
            printf("epoll failure\n");
            break;
        }
        for (int i = 0; i < ev_num; i++)
        {
            auto event = events[i];
            if (event.events & EPOLLIN)
            {
                if (event.data.fd == client_fd)
                {
                    // 客户端可读
                    int ret = read_et(buffer,BUFFER_SIZE, event.data.fd);
                    if (ret <= 0)
                    {
                        //断开连接
                        if_run = false;
                        break;
                    }
                    LOG("recv data from client %s -> %s", addr_ss.str().c_str(), buffer);
                    // 发送给主进程
                    size_t send_size = send(pipe_fd, (const void*)buffer, ret, 0);
                    LOG("send data to main process size:%ld", send_size);
                }
                else if (event.data.fd == pipe_fd)
                {
                    // 主进程可读
                    read_et(buffer,BUFFER_SIZE, event.data.fd);
                    LOG("recv data from client %s -> %s", addr_ss.str().c_str(), buffer);
                    // 转发给客户端
                    send(client_fd, buffer, strlen(buffer), 0);
                }
                else if (event.data.fd == child_signal_fd)
                {
                    signalfd_siginfo info;
                    read(event.data.fd, &info, sizeof(info));
                    switch (info.ssi_signo)
                    {
                    case SIGTERM:
                        if_run = false;
                        break;
                    default:
                        break;
                    }
                }
            }
        }
    }
    close(client_fd);
    close(pipe_fd);
    close(epoll_fd);
}

// 主循环
void main_loop()
{
    epoll_event events[MAX_EPOLL_EVENTS];
    while (!stop_server)
    {
        int event_num = epoll_wait(ep_fd, events,MAX_EPOLL_EVENTS, -1);
        if ((event_num == -1) && (errno != EINTR)) //注意这里必须要判断errno是不是系统中断因为信号会触发中断但是这是我们预期的正常行为
        {
            break;
        }
        else
        {
            for (int i = 0; i < event_num; i++)
            {
                auto event = events[i];
                if ((event.data.fd == listen_fd) && (event.events & EPOLLIN))
                {
                    //有新的客户端接入
                    sockaddr_in client_addr;
                    socklen_t client_addr_len = sizeof(client_addr);
                    int client_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &client_addr_len);
                    if (client_fd == -1)
                    {
                        LOG("accept error!");
                        continue;
                    }
                    else
                    {
                        //判断是否超过限制
                        if (user_map.size() >= MAX_USER_SIZE)
                        {
                            const char* data = "sorry the client num has arrived the limit!";
                            send(client_fd, data, strlen(data), 0);
                            close(client_fd);
                            continue;
                        }
                        //创建子进程
                        user_client user_data;
                        socketpair(AF_UNIX, SOCK_STREAM, 0, user_data.pipe_fd);
                        user_data.listen_fd = client_fd;
                        user_data.client_addr = client_addr;
                        int memIdx = get_free_mem();
                        if (memIdx == -1)
                        {
                            const char* data = "sorry the client num has arrived the limit!";
                            send(client_fd, data, strlen(data), 0);
                            close(client_fd);
                            continue;
                        }
                        user_data.memIdx = memIdx;
                        int child_pid = fork();
                        if (child_pid == -1)
                        {
                            LOG("create child process failed!");
                            close(client_fd);
                            continue;
                        }
                        else if (child_pid == 0)
                        {
                            run_child(client_fd, user_data);
                            LOG("child process ready to exit");
                            // 解除子进程对共享内存的映射
                            munmap(shm_mem,BUFFER_SIZE * MAX_USER_SIZE);
                            exit(0);
                        }
                        else
                        {
                            mem_use_set[memIdx] = true;
                            close(client_fd);
                            close(user_data.pipe_fd[1]);
                            add_fd_to_epoll(ep_fd, user_data.pipe_fd[0]);
                            user_data.child_pid = child_pid;
                            user_map[child_pid] = user_data;
                        }
                    }
                }
                else if ((event.data.fd == sig_pipe_fd) && (event.events & EPOLLIN))
                {
                    // 信号管道可读
                    // linux中自带的信号没有超过256的可以用一个字节表示
                    struct signalfd_siginfo signal_info;
                    int recv_size = read(sig_pipe_fd, &signal_info, sizeof(signal_info));
                    LOG("handle signal %d", signal_info.ssi_signo);
                    switch (signal_info.ssi_signo)
                    {
                    case SIGCHLD:
                        {
                            //子进程中断或者退出
                            int stat_loc;
                            int ret;
                            while ((ret = waitpid(-1, &stat_loc, WNOHANG)) > 0)
                            {
                                LOG("child process %d exit with code %d", ret, stat_loc<<8);
                                // 清楚客户端的资源
                                close(user_map[ret].pipe_fd[0]);
                                mem_use_set[user_map[ret].memIdx] = false;
                                user_map.erase(ret);
                                break;
                            }
                            break;
                        }
                    case SIGINT:
                    case SIGTERM:
                        {
                            LOG("kill all child process...");
                            // 关闭所有子进程
                            for (auto pair : user_map)
                            {
                                LOG("kill child process%d", pair.second.child_pid);
                                close(user_map[pair.first].pipe_fd[0]);
                                kill(pair.first, SIGTERM);
                            }
                            user_map.clear();
                            LOG("main process exit...");
                        }
                    }
                }
                else if (event.events & EPOLLIN)
                {
                    //子进程有消息
                    std::pair<int, user_client> user_data;
                    user_data.first = -1;
                    for (const auto& user : user_map)
                    {
                        if (user.second.pipe_fd[0] == event.data.fd)
                        {
                            user_data = user;
                        }
                    }
                    if (user_data.first != -1)
                    {
                        char buffer[BUFFER_SIZE];
                        //读取子进程发送给主进程的数据
                        int read_size = recv(event.data.fd, buffer, sizeof(buffer), 0);
                        if (read_size > 0)
                        {
                            for (const auto& pair : user_map)
                            {
                                if (pair.second.pipe_fd[0] == event.data.fd)
                                {
                                    continue;
                                }
                                send(pair.second.pipe_fd[0], buffer, read_size, 0);
                            }
                        }
                    }
                }
            }
        }
    }
}

int main(int argc, char** argv)
{
    // 初始化
    init();
    main_loop();
    release_resource();
    return 0;
}
