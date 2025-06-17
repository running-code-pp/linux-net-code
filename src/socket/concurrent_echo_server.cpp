#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <sys/wait.h> // waitpid 头文件

// 定义套接字描述符
int sockFd = -1;
// 定义地址结构体
sockaddr_in servAddr;

void error_quit(const char* msg) {
    perror(msg);
    exit(1);
}

void sig_handler(int signo) {
    if (signo == SIGINT) {
        printf("Caught SIGINT, closing socket and exiting...\n");
        close(sockFd);
        exit(0); // 确保程序退出时关闭套接字并终止执行
    }
}

void sig_chld_handler(int signo) {
    pid_t pid;
    int status;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        printf("Child %d terminated\n", pid);
    }
}

void sig_machineclose_handler(int signo) {
    close(sockFd);
    exit(0);
}
// 子进程函数
void sonProcessFunc(int clientSock, sockaddr_in clientAddr) {
    char* buf = (char*)malloc(1024);
    memset(buf, 0, 1024);
    const char* clientAddrStr = inet_ntoa(clientAddr.sin_addr);
    short clientPort = ntohs(clientAddr.sin_port);
    printf("process %d start work for client %s:%d\n", getpid(), clientAddrStr, clientPort);

    for (;;) {
        ssize_t nread = read(clientSock, buf, 1024);
        if (nread > 0) {
            if (strcmp(buf, "quit") == 0) {
                break;
            }
            printf("process %d recv data from client %s:%d-->%s\n", getpid(), clientAddrStr, clientPort, buf);
            write(clientSock, buf, nread); // 回显给客户端
            memset(buf, 0, nread);
        } else if (nread == 0) { // 客户端关闭连接
            break;
        } else { // 错误发生
            perror("read error");
            break;
        }
    }

    free(buf);
    close(clientSock);
}

int main(int argc, char** argv) {
    // 注册 SIGINT 和 SIGCHLD 的信号处理器
    signal(SIGINT, sig_handler);
    signal(SIGCHLD, sig_chld_handler);
    signal(SIGTERM, sig_machineclose_handler);
    short port = 9527;
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(port);
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    sockFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockFd < 0) {
        error_quit("create socket error!");
    }

    if (bind(sockFd, (const sockaddr*)&servAddr, sizeof(servAddr)) < 0) {
        error_quit("bind error!");
    }

    listen(sockFd, 1024);

    for (;;) {
        int clientFd;
        socklen_t len = sizeof(sockaddr_in);
        sockaddr_in clientAddr;

        clientFd = accept(sockFd, (sockaddr *)&clientAddr, &len);
        if (clientFd < 0) {
            perror("accept error!\n");
            continue;
        } else {
            printf("client %s:%d connect success!\n",
                   inet_ntoa(clientAddr.sin_addr),
                   ntohs(clientAddr.sin_port));

            pid_t pid = fork();
            if (pid == 0) {
                // 子进程
                close(sockFd); // 子进程不需要监听套接字
                printf("start a son process\n");
                sonProcessFunc(clientFd, clientAddr);
                close(clientFd);
                exit(0); // 子进程执行完退出
            } else if (pid > 0) {
                // 父进程
                printf("parent process: created child PID %d\n", pid);
                close(clientFd); // 父进程不再需要该客户端连接描述符
            } else {
                perror("fork error");
                close(clientFd);
            }
        }
    }

    return 0;
}