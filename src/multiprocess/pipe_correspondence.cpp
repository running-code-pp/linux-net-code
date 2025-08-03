/**
 *@brief 多进程管道通信
 * 本程序 实现子进程重定向输出到输出管道中父进程读取管道并输出到自己的终端
 */
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<stdio.h>
#include<stdlib.h>
#include <sys/socket.h>

int main(int argc, char** argv)
{
    // 创建管道
    int pipe_fds[2];
    int n = pipe(pipe_fds);
    if (n != 0)
    {
        printf("create pipe error!\n");
    }
    auto ret = fork();
    if (ret == 0)
    {
        //关闭子进程持有的管道的读端
        close(pipe_fds[0]);
        printf("child process %d start running...,it's parent process is:%d\n",getpid(),getppid());
        // 重定向标准输出到管道的写文件描述符
        auto ret=dup2(pipe_fds[1],STDOUT_FILENO);
        if (ret == -1)
        {
            close(pipe_fds[1]);
            exit(1);
        }
        for (int i=0;i<5;i++)
        {
            printf("child message %d\n",i+1);
            sleep(1);
        }
        close(pipe_fds[1]);
        exit(0);
    }
    else
    {
        // 关闭父进程持有的写端
        close(pipe_fds[1]);
        printf("main process %d start running...\n",getpid());
        //读取管道
        while (true)
        {
            char buffer[1024]={0};
            ssize_t read_num=read(pipe_fds[0],buffer,sizeof(buffer));
            if ( read_num ==0)
            {
                break;
            }
            else if ( read_num ==-1)
            {
                continue;
            }
            else
            {
                printf("recv child process message:%s\n",buffer);
            }
        }
        close(pipe_fds[0]);
        int stat_loc;
        auto ret=wait(&stat_loc);
        if (ret!=-1)
        {
            printf("wait child process %d exit success!,the exit code is :%d\n",ret,stat_loc>8);
            return 0;
        }
        else
        {
            printf("can not wait child process exit!\n");
        }
    }

    return 0;
}

/**
输出：
/home/pp/linux-net-code/cmake-build-debug/bin/Debug/pipe_correspondence
main process 26698 start running...
child process 26699 start running...,it's parent process is:26698
recv child process message:child message 1

recv child process message:child message 2

recv child process message:child message 3

recv child process message:child message 4

recv child process message:child message 5

wait child process 26699 exit success!,the exit code is :0

进程已结束，退出代码为 0

 */