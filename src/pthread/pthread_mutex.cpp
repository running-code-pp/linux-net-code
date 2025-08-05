/**
 * ==================================================
 *  @file pthread_02.cpp
 *  @brief pthread更高级的用法
 *  @author ywj
 *  @date 2025-06-22 16:40
 *  @version 1.0
 *  @copyright Copyright (c) 2025 ywj. All Rights Reserved.
 * ==================================================
 */

#include <cstdio>
#include<pthread.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>

#define COUNTER_TIME 500

int count=0;

pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;
//如果这个函数没有返回值可能会造成意想不到的结果
void* counterFunc(void*arg)
{
    printf("thread %lu start work for counter!\n",pthread_self());
    for (int i=0;i<COUNTER_TIME;i++)
    {
        //获取互斥锁
        pthread_mutex_lock(&counter_mutex);
        ++count;
        printf("the value of count is:%d\n",count);
        //释放锁
        pthread_mutex_unlock(&counter_mutex);
    }
    return NULL;
}

int main()
{
    //初始化互斥锁
    pthread_mutex_init(&counter_mutex,NULL);
    pthread_t countThread01;
    pthread_t countThread02;
    if (pthread_create(&countThread01,NULL,&counterFunc,NULL)==0)
    {
        printf("create countThread01 success!\n");
    }
    else
    {
        printf("create countThread01 failed!\n");
        exit(1);
    }

    if (pthread_create(&countThread02,NULL,&counterFunc,NULL)==0)
    {
        printf("create countThread02 success!\n");
    }
    else
    {
        printf("create countThread02 failed!\n");
        exit(1);
    }

    pthread_join(countThread01,NULL);
    pthread_join(countThread02,NULL);
    pthread_mutex_destroy(&counter_mutex);
    printf("the end value of count is:%d\n",count);

    return 0;
}