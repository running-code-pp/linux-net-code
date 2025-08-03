#include <unistd.h>
#include <stdlib.h>
#include<stdio.h>
int main(){
    printf("主进程id:%d\n",getpid());
    int pid=fork();
    if(pid==0){
    printf("子进程id:%d\n",getpid());
    execl("/root/code/linux-net-code/build/bin/Debug/exec","arg1","arg2", "arg3",NULL);
    }
    else{
    printf("主进程继续执行...\n");
    }
    return 0;
}
/**
程序输出:
主进程id:5612
主进程继续执行...
子进程id:5613
exec pid:5613
total arg num:3arg1:arg1
arg2:arg2
arg3:arg3
 */