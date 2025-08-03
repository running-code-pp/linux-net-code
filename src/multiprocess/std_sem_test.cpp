/**
 *@brief :C++标准库信号量测试
 */

#include<semaphore>
#include<stdlib.h>
#include<stdio.h>
#include<sys/wait.h>
#include<unistd.h>

// 注意c++中的信号量是用于线程同步的 不能用于进程同步，因为子进程持有的是副本

// 二进制信号量     std::binary_semaphore;
// 自定义范围信号量    std::counting_semaphore<>
// P操作 acquire
// V操作 release

// std::binary_semaphore binary_sem(1); 这个就是特化的counting_semaphore<1>
// std::counting_semaphore<3> count_sem(3);

