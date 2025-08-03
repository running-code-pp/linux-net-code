/**
 * @brief : 防止僵尸进程的产生
 * 子进程的返回值左移八位(*256)就是stat_lock的值 如果子进程正常退出的话
 */
#include<unistd.h>
#include<sys/wait.h>
#include<stdio.h>

void child_process_status_dump(int status)
{
        if (WIFEXITED(status)) {
            printf("Child exited, status=%d\n", WEXITSTATUS(status));
        }
        else if (WIFSIGNALED(status)) {
            printf("Child was killed by signal %d\n", WTERMSIG(status));
        }
        else if (WIFSTOPPED(status)) {
            printf("Child was stopped by signal %d\n", WSTOPSIG(status));
        }
        // 注意: 在Linux下还可以检查是否通过SIGCONT恢复了
#ifdef WCOREDUMP
        if (WCOREDUMP(status))
            printf("Child produced a core dump.\n");
#endif
}

int main(int argc,char**argv)
{
    printf("the main process %d start running...\n",getpid());
    auto ret= fork();
    if (ret==0)
    {
        printf("the child process 1->%d start running...,parent process %d \n",getpid(),getppid());
        // sleep 5 second to let parent process wait
        sleep(5);
        return 1; // the stat_loc is 256
    }
    else
    {
        printf("the main process %d continue running...\n",getpid());
        auto ret= fork();
        if (ret==0)
        {
            printf("the child process 2->%d start running...,parent process %d \n",getpid(),getppid());
            // sleep 6 second to let parent process wait
            sleep(6);
            return 3; // 3<<8 768
        }
        while(true)
        {
            int  stat_loc;
            auto child_pid=wait(&stat_loc);
            if (child_pid==-1)
            {
                printf("all child process was exited...\n");
                return 0;
            }
            printf("wait finished the childprocess %d was exited with code %d\n",child_pid,stat_loc);
            // dump the info of process exit state
            child_process_status_dump(stat_loc);
        }
        return 0;
    }
    return 0;
}