/**
 *@brief : 使用信号量实现父子进程的通信
 */

#include<unistd.h>
#include<sys/sem.h>
#include<stdio.h>
#include<sys/wait.h>
#include<stdlib.h>

union semun
{
    int val;
    struct semid_ds* buf;
    unsigned short int* array;
    seminfo* sem_info;
};

void pv(int sem_id, int op)
{
    sembuf sem_buf;
    sem_buf.sem_num = 0;
    sem_buf.sem_op = op;
    sem_buf.sem_flg = SEM_UNDO;
    semop(sem_id, &sem_buf, 1);
}

int main()
{
    //创建信号量
    int sem_id = semget(IPC_PRIVATE, 1, 0666); // 0666八进制表示 110表示rw(没有x执行权限），三个6分别表示所有者，组用户，其他人都有rw权限
    union semun sem_union;
    sem_union.val = 1;
    semctl(sem_id,0,SETVAL,sem_union);
    //创建子进程
    int child_pid = fork();
    if (child_pid == 0)
    {
        printf("process %d 's child process %d was started!\n",getppid(),getpid());
        printf("child process %d start to get the sem...\n",getpid());
        //获取信号量--对信号量进行P操作
        pv(sem_id,-1);
        printf("child process %d get the sem!\n",getpid());
        sleep(5);
        //释放信号量 --对信号量V操作
        pv(sem_id,1);
        sleep(3);
        exit(0);
    }
    else
    {
        printf("start child process %d success!\n",child_pid);
        sleep(1);
        //获取信号量
        printf("main process %d start to get the sem...\n",getpid());
        pv(sem_id,-1);
        printf("main process %d get the sem!\n",getpid());
        //释放信号量
        pv(sem_id,1);
    }

    //删除信号量
    semctl(sem_id,0,IPC_RMID,sem_union);
    //等待子进程退出
    auto child_pid_= waitpid(child_pid,NULL,0);
    printf("child process %d exit!\n",child_pid_);
    return 0;
}

/**
 输出：
start child process 67502 success!
process 67501 's child process 67502 was started!
child process 67502 start to get the sem...
child process 67502 get the sem!
main process 67501 start to get the sem...
main process 67501 get the sem!
child process 67502 exit!

进程已结束，退出代码为 0
*/