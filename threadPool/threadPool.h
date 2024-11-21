#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <deque>
#include <vector>
#include <cassert>  // assert
#include <cstring>  //strerror
#include <unistd.h> // sleep
#include <iostream>
#include <pthread.h>
#include <functional> // For std::function and std::bind
using namespace std;
const int DEFAULT_THREAD_NUM = 2;
class ThreadPool {
   private:
    struct threadpoolTask {        // 子线程任务
        function<void(void *)> callBack;
        void *arg;                 /* 上面函数的参数 */
        threadpoolTask();
        ~threadpoolTask();
    };
    struct ThreadPoolData {            // 线程池数据
        pthread_mutex_t mutex;         // 互斥锁--用于锁住本结构体
        pthread_mutex_t taskMutex;         // 互斥锁--用于锁住任务队列
        pthread_cond_t queueNotEmpty;  // 条件变量--任务队列不为空
        pthread_cond_t queueNotFull;   // 条件变量--任务队列不为满
    
        vector<pthread_t> threads;        // 存放线程池中每个线程的id
        pthread_t adjustThreadId;             // 存放管理线程的id
        deque<threadpoolTask> taskQueue;  // 任务队列
    
        const int MIN_THREAD_SIZE;      // 线程池中最小线程数
        const int maxThreadSize;      // 线程池中最大线程数
        int liveThreadNum;      // 线程池中存活的线程数
        int busyThreadNum;      // 线程池中正在工作的线程数
        int waitExitThreadNum;  // 等待销毁的线程数
    
        const int taskQueueMaxSize;  // 任务队列中可容纳任务数上限
    
        bool isRunning;  // 线程池是否运行
    
        ThreadPoolData();
        ~ThreadPoolData();
    };
    ThreadPoolData *threadPoolData;

   public:
    ThreadPool();
    ~ThreadPool();
    bool createThreadPool();
    void destoryThreadPool();  // 销毁线程池
    void addTask(function<void(void *)> callBack, void *arg);            // 添加任务
    static void* adjustThreadPool(void *arg);  // 管理线程回调函数
    static void* workerThread(void *arg);//子线程回调函数

};

#endif // THREADPOOL_H