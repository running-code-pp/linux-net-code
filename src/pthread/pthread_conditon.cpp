/**
 * ==================================================
 *  @file pthread_conditon.cpp
 *  @brief 条件变量--实现一个线程安全的队列
 *  @author ywj
 *  @date 2025-06-22 17:29
 *  @version 1.0
 *  @copyright Copyright (c) 2025 ywj. All Rights Reserved.
 * ==================================================
 */

#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>
#include<stdio.h>
#include<queue>

//注意要先通知后释放锁
struct SafeQueue
{
    void init(size_t capacity)
    {
        pthread_mutex_lock(&_mutex);
        _capacity = capacity;
        pthread_mutex_unlock(&_mutex);
    }

    void push(int val)
    {
        //获取锁
        pthread_mutex_lock(&_mutex);
        //判断是否非满,可能存在虚假唤醒所有用循环
        while (_size >= _capacity)
        {
            //等待非满
            pthread_cond_wait(&_condNotFull, &_mutex);
        }
        //被条件非满条件变量唤醒
        _queue.push(val);
        ++_size;

        //通知非空
        pthread_cond_signal(&_condNotEmpty);
        //释放锁
        pthread_mutex_unlock(&_mutex);
    }

    int pop()
    {
        //获取锁
        pthread_mutex_lock(&_mutex);
        while (_size == 0)
        {
            //等待非空条件变量通知
            pthread_cond_wait(&_condNotEmpty, &_mutex);
        }
        //唤醒之后
        int ret = _queue.front();
        _queue.pop();
        --_size;
        //通知非满
        pthread_cond_signal(&_condNotFull);
        //释放锁
        pthread_mutex_unlock(&_mutex);

        return ret;
    }
    ~SafeQueue()
    {
        pthread_mutex_destroy(&_mutex);
        pthread_cond_destroy(&_condNotEmpty);
        pthread_cond_destroy(&_condNotFull);
    }
    std::queue<int> _queue;
    size_t _size = 0;
    size_t _capacity = 100;
    pthread_mutex_t _mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t _condNotEmpty = PTHREAD_COND_INITIALIZER;
    pthread_cond_t _condNotFull = PTHREAD_COND_INITIALIZER;
};

SafeQueue safeQueue;

void* threadFunc(void* arg)
{
    for (int i = 0; i < 100; i++)
    {
        safeQueue.push(i);
        printf("thread %lu push safeQueue-->%d\n", pthread_self(), i);
    }
    return NULL;
}

void* threadFunc1(void* arg)
{
    for (int i = 100; i < 200; i++)
    {
        safeQueue.push(i);
        printf("thread %lu push safeQueue-->%d\n", pthread_self(), i);
    }
    return NULL;
}

void* threadFunc2(void* arg)
{
    for (int i = 0; i < 200; i++)
    {
        int val = safeQueue.pop();
        printf("thread %lu pop safeQueue-->%d\n", pthread_self(), val);
    }
    return NULL;
}

int main()
{
    //初始化
    safeQueue.init(100);
    pthread_t pushThread1, pushThread2, popThread;
    if (pthread_create(&pushThread1, NULL, threadFunc, NULL) || pthread_create(&pushThread2,NULL, threadFunc1,NULL) ||
        pthread_create(&popThread, NULL, threadFunc2,NULL))
    {
        printf("create pthread fail\n");
        exit(1);
    }
    pthread_join(pushThread1,NULL);
    pthread_join(pushThread2,NULL);
    pthread_join(popThread,NULL);
    return 0;
}
