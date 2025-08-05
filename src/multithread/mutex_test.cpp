/**
 *@brief : linux api下的各种锁
 */

#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <format>
#include <time.h>
#include <sstream>
#include <map>

#define LOG(format, ...)                                                         \
    do                                                                           \
    {                                                                            \
        char timestr[26] = {0};                                                  \
        time_t now = time(NULL);                                                 \
        tm *time_info = localtime(&now);                                         \
        strftime(timestr, sizeof(timestr), "%Y-%m-%D %H:%M:%S", time_info);      \
        printf("[%s][%d] " format "\n", timestr, pthread_self(), ##__VA_ARGS__); \
    } while (0)

// 读写锁
class RwLock
{
public:
    /**
     * @param if_unlock_thread_exit if unlock the mutex when thread exit
     * @param if_process_shared if shared the rwlock between with other process
     * @note this init func will throw exception when init error
     */
    RwLock(bool if_unlock_thread_exit = false, bool if_process_shared = false) : _mutex(PTHREAD_MUTEX_INITIALIZER)
    {
        pthread_rwlockattr_t attr;
        pthread_rwlockattr_init(&attr);
        pthread_rwlockattr_setpshared(&attr, if_process_shared ? PTHREAD_PROCESS_SHARED : PTHREAD_PROCESS_PRIVATE);
        int ret = pthread_rwlock_init(&_mutex, &attr);
        if (ret != 0)
        {
            std::stringstream ss;
            ss << "pthread_rwlock_init error,code:" << ret;
            throw std::runtime_error(ss.str());
        }
    }

    // get read lcok if success return 0
    int rlock()
    {
        return pthread_rwlock_rdlock(&_mutex);
    }

    // try to get read lock if success return 0
    int try_rlock()
    {
        return pthread_rwlock_tryrdlock(&_mutex);
    }

    // try to get write lock success return 0
    int try_wlock()
    {
        return pthread_rwlock_trywrlock(&_mutex);
    }

    // unlock if success return 0
    int unlock()
    {
        return pthread_rwlock_unlock(&_mutex);
    }

    // get write lock if success return 0
    int wlock()
    {
        return pthread_rwlock_wrlock(&_mutex);
    }

    ~RwLock()
    {
        pthread_rwlock_destroy(&_mutex);
    }

private:
    pthread_rwlock_t _mutex;
};

class Mutex
{
public:
    /**
     * @param if_unlock_thread_exit if unlock the mutex when thread exit
     * @param if_process_shared if shared the mutex between with other process
     * @note this init func will throw exception when init error
     */
    Mutex(bool if_unlock_thread_exit = false, bool if_process_shared = false)
        : _mutex(PTHREAD_MUTEX_INITIALIZER)
    {
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_setpshared(&attr, if_process_shared ? PTHREAD_PROCESS_SHARED : PTHREAD_PROCESS_PRIVATE);
        int ret = pthread_mutex_init(&_mutex, &attr);
        if (ret != 0)
        {
            std::stringstream ss;
            ss << "pthread_mutex_init error, code:" << ret;
            throw std::runtime_error(ss.str());
        }
    }

    // lock the mutex, block if not available
    int lock()
    {
        return pthread_mutex_lock(&_mutex);
    }

    // try to lock the mutex, return 0 if success
    int try_lock()
    {
        return pthread_mutex_trylock(&_mutex);
    }

    // unlock the mutex
    int unlock()
    {
        return pthread_mutex_unlock(&_mutex);
    }

    ~Mutex()
    {
        pthread_mutex_destroy(&_mutex);
    }

private:
    pthread_mutex_t _mutex;
};

std::map<int, std::string> _gloablMap;

int main(int argc, char **argv)
{
    return 0;
}