#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
int global = 10;

// 子进程只是把内存复制过去了 二者拥有的并不是同一份
// 但是并不是fork之后，子进程又要重新开辟一个和父进程同等大小的内存空间，二者遵循读时共享，写时复制的策略

void signal_handle(int signo)
{
    switch (signo)
    {
    case SIGINT:
        printf("signal SIGINT was occurred in process:%d!\n", getpid());
        break;
    case SIGALRM:
        printf("signal SIGALRM was occurred in process %d!\n", getpid());
        break;
    default:
        break;
    }
}

int main(int argc, char **argv)
{
    int local = 4; // 局部变量
    int pid = fork();
    signal(SIGALRM, signal_handle);
    alarm(2);//在fork之前调用的那么fork出来的子进程也会拥有这个定时器状态
    if (pid == 0)
    { // 子进程
        printf("子进程中：global,local:%d,%d\n", global, local);
        // 子进程修改全局变量和局部变量
        global += 10;
        local += 10;
        printf("子进程开始休眠s\n");
        sleep(2);
        printf("子进程休眠结束\n");
        printf("子进程中：global,local:%d,%d\n", global, local);
        sleep(5);
    }
    else
    { // 父进程
        printf("父进程开始休眠s\n");
        sleep(1);
        printf("父进程休眠结束\n");
        printf("父进程中：global,local:%d,%d\n", global, local);
        global = -1;
        local = -10;
        alarm(3);
        sleep(5);
    }
    return 0;
}

/**
 运行结果:
父进程开始休眠s
子进程中：global,local:10,4
子进程开始休眠s
父进程休眠结束
父进程中：global,local:10,4
signal SIGALRM was occurred in process 2121!
子进程休眠结束
子进程中：global,local:20,14
signal SIGALRM was occurred in process 2120!
 */