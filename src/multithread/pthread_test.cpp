/**
 * @brief: pthread api 基本使用:单生产者-多消费者
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <queue>
#include <time.h>
#define LOG(format, ...)\
do{\
char str[26]={0};\
time_t now =time(NULL);\
tm*time_info = localtime(&now);\
strftime(str,sizeof(str),"%Y-%m-%D %H:%M:S",time_info);\
printf("[%s] [%d] " format "\n",str,pthread_self(),##__VA_ARGS__);\
}while(0)

// 全局变量
std::queue<int> que;
pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;
std::vector<pthread_t> threads;
bool consume_thread_if_exit = false;
// 生产者线程函数
void* produce_thread_func(void* arg)
{
    LOG("produce thread start!");
    int* p = (int*)arg;
    if (*p <= 0)
    {
        consume_thread_if_exit = true;
        return NULL;
    }
    for (int i = 0; i < *p; i++)
    {
        usleep(100);
        pthread_mutex_lock(&count_mutex);
        LOG("produce %d", i+1);
        que.push(i + 1);
        pthread_mutex_unlock(&count_mutex);
    }
    consume_thread_if_exit = true;
    return NULL;
}

// 消费者线程函数
void* consume_thread_func(void* arg)
{
    LOG("consume thread start!");
    while (!consume_thread_if_exit)
    {
        pthread_mutex_lock(&count_mutex);
        if (!que.empty())
        {
            LOG("consume %d", que.front());
            que.pop();
        }
        pthread_mutex_unlock(&count_mutex);
    }
    return NULL;
}

void create_threads()
{
    // 创建生产者
    pthread_t produce_thread_id = 0;
    int produce_count = 100;
    pthread_create(&produce_thread_id,NULL, produce_thread_func, (void*)&produce_count);
    threads.push_back(produce_thread_id);
    // 创建三个消费者
    for (int i = 0; i < 3; i++)
    {
        pthread_t consume_thread_id = 0;
        pthread_create(&consume_thread_id,NULL, consume_thread_func,NULL);
        pthread_join(consume_thread_id, NULL);
    }
}

int main(int argc, char** argv)
{
    LOG("start!");
    // 初始化互斥锁
    pthread_mutex_init(&count_mutex,NULL);
    create_threads();
    LOG("release the mutex!");
    //释放资源
    pthread_mutex_destroy(&count_mutex);
    return 0;
}
