/**
 * 在exec_test 中运行的目标程序
 */

 #include<stdio.h>
 #include<stdlib.h>
 #include<unistd.h>
 int main(int argc,char**argv){
    printf("exec pid:%d\n",getpid());
    printf("total arg num:%d",argc);
    for(int i=0;i<argc;i++){
        printf("arg%d:%s\n",i+1,argv[i]);
    }
    return 0;
 }