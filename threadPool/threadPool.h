#include <pthread.h>

#include <deque>
#include <vector>
#include <cassert>  // assert
#include <iostream>
#include <functional> // For std::function and std::bind
using namespace std;
class ThreadPool {
   private:
    struct threadpoolTask {        // 各子线程任务
        void *(*function)(void *); /* 函数指针，回调函数 */
        void *arg;                 /* 上面函数的参数 */
    };
    struct ThreadPoolData {            // 线程池数据
        pthread_mutex_t mutex;         // 互斥锁--用于锁住本结构体
        pthread_cond_t queueNotEmpty;  // 条件变量--任务队列不为空
        pthread_cond_t queueNotFull;   // 条件变量--任务队列不为满

        vector<pthread_t> threads;        // 存放线程池中每个线程的id
        pthread_t adjust_tid;             // 存放管理线程的id
        deque<threadpoolTask> taskQueue;  // 任务队列

        int minThreadSize;      // 线程池中最小线程数
        int maxThreadSize;      // 线程池中最大线程数
        int liveThreadNum;      // 线程池中存活的线程数
        int busyThreadNum;      // 线程池中正在工作的线程数
        int waitExitThreadNum;  // 等待销毁的线程数

        int taskQueueMaxSize;  // 任务队列中的任务数

        bool isRunning;  // 线程池是否运行
    };
    ThreadPoolData *threadPoolData;

   public:
    ThreadPool(/* args */);
    ~ThreadPool();
    void createThreadPool();
    void destoryThreadPool();  // 销毁线程池
    void submitTask();         // 分配任务

    static void* adjustThreadPool(void *arg);  // 管理线程回调函数
    
    static void* threadPoolThread(void *arg);//子线程回调函数

};

ThreadPool::ThreadPool(/* args */) {
}

ThreadPool::~ThreadPool() {
}
// 创建线程池
void ThreadPool::createThreadPool() {
    do {
        threadPoolData = new ThreadPoolData();
        if (threadPoolData == NULL) {
            cout << "创建线程池失败" << endl;
            break;
        }
        threadPoolData->minThreadSize = 2;
        threadPoolData->maxThreadSize = 8;
        threadPoolData->liveThreadNum = 2;
        threadPoolData->busyThreadNum = 0;
        threadPoolData->waitExitThreadNum = 0;

        threadPoolData->taskQueueMaxSize = 10;
        threadPoolData->isRunning = true;

        /*根据最大线程上限，为工作线程数组分配空间*/
        threadPoolData->threads.resize(threadPoolData->maxThreadSize);
        assert(threadPoolData->threads.size() == threadPoolData->maxThreadSize);
        fill(threadPoolData->threads.begin(), threadPoolData->threads.end(), 0);  // 清零

        /*初始化互斥锁和条件变量*/
        if (pthread_mutex_init(&threadPoolData->mutex, NULL) && pthread_cond_init(&threadPoolData->queueNotEmpty, NULL) && pthread_cond_init(&threadPoolData->queueNotFull, NULL)) {
            cout << "初始化互斥锁或条件变量失败" << endl;
            break;
        }

        /*创建任务线程*/
        for (int i = 0; i < threadPoolData->minThreadSize; i++) {
            pthread_create(&threadPoolData->threads[i], NULL, threadPoolThread, (void *)threadPoolData);
            cout << "创建线程数：" << i << endl;
        }
        // 创建管理线程
        pthread_create(&threadPoolData->adjust_tid, NULL, adjustThreadPool, (void *)threadPoolData);

    } while (0);
    destoryThreadPool();
}


// 管理线程回调函数
void* ThreadPool::adjustThreadPool(void *arg){

}
//子线程回调函数
void* ThreadPool::threadPoolThread(void *arg){

}
// 销毁线程池
void destoryThreadPool(){

}