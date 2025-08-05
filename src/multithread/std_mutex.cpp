/**
*@brief : 标准库下的各种锁
 */

#include<pthread.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<format>
#include<time.h>
#include<mutex>
#include<shared_mutex>
#include<atomic>
#include<cassert>
#include<type_traits>

#define LOG(format,...)\
do{\
char timestr[26]={0};\
time_t now = time(NULL);\
tm* time_info = localtime(&now);\
strftime(timestr,sizeof(timestr),"%Y-%m-%D %H:%M:%S",time_info);\
printf("[%s][%d] " format "\n",timestr,pthread_self(),##__VA_ARGS__);\
}while (0)\

// 读写锁
template<typename T>
class RWLock
{
 // 获取读锁
 template<typename Func>
 auto access_read(Func f)
 {
  static_assert(std::is_invocable_v<Func,T&>(),"read func must and only can have a param type of T&");
  std::shared_lock<std::shared_mutex>(_mutex);
  return f(_data);
 }


 template<typename Func>
 auto access_write(Func f)
 {
  static_assert(std::is_invocable_v<Func,T&>(),"write func must and only can have a param type of T&");
  std::unique_lock<std::shared_mutex>(_mutex);
  return f(_data);
 }
private:
 std::shared_mutex _mutex;
 T _data;
};

// 自旋锁
class SpinLock
{
 void spin()
 {

 }
private:
 std::atomic_flag _flag;
};