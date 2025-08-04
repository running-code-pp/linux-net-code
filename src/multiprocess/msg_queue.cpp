/**
 *@brief :消息队列测试
 */
#include <cstring>
#include<sys/msg.h>
#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<sys/ipc.h>
#include<time.h>
#include <sys/wait.h>
#include <signal.h>
#include<iostream>
#include<string>

// 全局变量
bool if_server_running = true;
bool if_child_running = true;
int child_pid = 0;
int msg_queue_fd = 0;
#define BUFFER_SIZE 1024

#define LOG(format,...)\
 do{\
time_t now = time(NULL);\
char time_str[26]={0};\
tm* time_info = localtime(&now);\
strftime(time_str,sizeof(time_str),"%Y%m%d %H%M%S",time_info); \
printf("[%s] [%d] " format "\n",time_str,getpid(),##__VA_ARGS__); \
}while(0)
#define PROJECT_ID 9595

void signal_handler(int sig)
{
    LOG("received signal %d", sig);
    switch (sig)
    {
    case SIGCHLD:
        {
            int stat_loc;
            pid_t pid;
            while ((pid = waitpid(-1, &stat_loc, WNOHANG)) > 0)
            {
                LOG("child process %d exited with status %d", pid, WEXITSTATUS(stat_loc));
            }
            break;
        }
    case SIGTERM:
    case SIGINT:
        {
            if (child_pid > 0) kill(child_pid, SIGTERM);
            if_server_running = false;
            break;
        }
    default:
        break;
    }
}

void child_signal_handler(int signo)
{
    LOG("child process received signal %d", signo);
    switch (signo)
    {
    case SIGTERM:
        if_child_running = false;
    }
}

void run_child()
{
    signal(SIGPIPE, SIG_IGN);
    signal(SIGTERM, child_signal_handler);
    char buffer[BUFFER_SIZE];
    while (if_child_running)
    {
        // 阻塞等待任意类型的消息
        msgrcv(msg_queue_fd, buffer,BUFFER_SIZE, 0,MSG_NOERROR);
        LOG("child process receive msg from main process:%s", buffer);
    }
    close(msg_queue_fd);
    exit(0);
}

int main(int argc, char** argv)
{
    signal(SIGPIPE, SIG_IGN);
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP; // 关键：SA_NOCLDSTOP 防止子进程停止时触发
    sigaction(SIGCHLD, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    LOG("main process start!");
    int key = ftok(".",PROJECT_ID);
    if (key == -1)
    {
        LOG("create msg_queue key failed!");
        return 1;
    }
    msg_queue_fd = msgget(key, 0666 | IPC_CREAT);
    child_pid = fork();
    if (child_pid == -1)
    {
        LOG("create child process failed!");
    }
    else if (child_pid == 0)
    {
        LOG("a child process start!");
    }
    else
    {
        LOG("create child process %d success!", child_pid);
        run_child();
    }
    int stat_loc = -1;
    while (if_server_running)
    {
        printf("请输出要发送给子进程的消息:");
        std::string msg;
    std:
        getline(std::cin, msg);
        if (msgsnd(msg_queue_fd, msg.c_str(), msg.length(), 0) == -1)
        {
            LOG("send msg failed!");
        }
        else
        {
            LOG("send msg success!");
        }
    }
    close(msg_queue_fd);
    return 0;
}
