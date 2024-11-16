#include <pthread.h>

#include <deque>
#include <vector>
#include <cassert>  // assert
#include <iostream>
#include <functional> // For std::function and std::bind
using namespace std;
const int DEFAULT_THREAD_NUM = 2;
class ThreadPool {
   private:
    struct threadpoolTask {        // 子线程任务
        void *(*callBack)(void *); /* 回调函数指针 */
        void *arg;                 /* 上面函数的参数 */
    };
    struct ThreadPoolData {            // 线程池数据
        pthread_mutex_t mutex;         // 互斥锁--用于锁住本结构体
        pthread_cond_t queueNotEmpty;  // 条件变量--任务队列不为空
        pthread_cond_t queueNotFull;   // 条件变量--任务队列不为满
    
        vector<pthread_t> threads;        // 存放线程池中每个线程的id
        pthread_t adjust_tid;             // 存放管理线程的id
        deque<threadpoolTask> taskQueue;  // 任务队列
    
        const int MIN_THREAD_SIZE;      // 线程池中最小线程数
        int maxThreadSize;      // 线程池中最大线程数
        int liveThreadNum;      // 线程池中存活的线程数
        int busyThreadNum;      // 线程池中正在工作的线程数
        int waitExitThreadNum;  // 等待销毁的线程数
    
        int taskQueueMaxSize;  // 任务队列中可容纳任务数上限
    
        bool isRunning;  // 线程池是否运行
    
        ThreadPoolData() : MIN_THREAD_SIZE(2) {} // Add a default constructor
    };
    
    ThreadPoolData *threadPoolData;

   public:
    ThreadPool(/* args */);
    ~ThreadPool();
    void createThreadPool();
    void destoryThreadPool();  // 销毁线程池
    void submitTask();         // 分配任务

    static void* adjustThreadPool(void *arg);  // 管理线程回调函数
    
    static void* workerThread(void *arg);//子线程回调函数

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
        threadPoolData->maxThreadSize = 8;
        threadPoolData->liveThreadNum = 2;
        threadPoolData->busyThreadNum = 0;
        threadPoolData->waitExitThreadNum = 0;

        threadPoolData->taskQueueMaxSize = 10;
        threadPoolData->isRunning = true;

        /*根据最大线程上限，为工作线程数组分配空间*/
        threadPoolData->threads.resize(threadPoolData->maxThreadSize);
        if(threadPoolData->threads.size() != threadPoolData->maxThreadSize){
            cerr << "线程数组分配空间失败" << endl;
            break;
        }
        fill(threadPoolData->threads.begin(), threadPoolData->threads.end(), 0);  // 清零

        /*初始化互斥锁和条件变量*/
        if (pthread_mutex_init(&threadPoolData->mutex, NULL) && pthread_cond_init(&threadPoolData->queueNotEmpty, NULL) && pthread_cond_init(&threadPoolData->queueNotFull, NULL)) {
            cerr << "初始化互斥锁或条件变量失败" << strerror(errno) << endl;
            break;
        }

        /*创建任务线程*/
        for (int i = 0; i < threadPoolData->MIN_THREAD_SIZE; i++) {
            if(pthread_create(&threadPoolData->threads[i], NULL, workerThread, (void *)threadPoolData) != 0){
                cerr << "创建线程失败" << strerror(errno) << endl;
                break;
            }else{
                cout << "创建线程数：" << i << endl;
            }
        }
        // 创建管理线程
        if(pthread_create(&threadPoolData->adjust_tid, NULL, adjustThreadPool, (void *)threadPoolData)){
            cerr << "创建管理线程失败" << strerror(errno) << endl;
            break;
        }
    } while (0);
    destoryThreadPool();
}


// 管理线程回调函数
void* ThreadPool::adjustThreadPool(void *arg){
    ThreadPoolData *threadPoolData = static_cast<ThreadPoolData*>(arg);
    pthread_mutex_t mutex = threadPoolData->mutex;
    while (threadPoolData->isRunning)
    {
        /* 定时对线程池进行管理 */
        sleep(10);
        /* 加锁 */
        pthread_mutex_lock(&mutex);
        /* 获取数据 */
        int minThreadSize = threadPoolData->MIN_THREAD_SIZE;//线程池最小线程数
        int maxThreadSize = threadPoolData->maxThreadSize;//线程池最大线程数
        int taskQueueSize = threadPoolData->taskQueue.size();//当前任务数
        int liveThreadNum = threadPoolData->liveThreadNum;//存活线程数
        int busyThreadNum = threadPoolData->busyThreadNum;//忙线程数  
        /* 判断是否创建新线程：任务数小于线程池最小线程数，且存活的线程数小于线程池最大线程数 */
        if(taskQueueSize > minThreadSize && liveThreadNum < maxThreadSize){
            /* 一次性增加DEFAULT_THREAD_NUM个线程 */
            int count = 0;
            for(int i = 0; i < maxThreadSize && count < DEFAULT_THREAD_NUM && liveThreadNum < maxThreadSize; i++){
                if(threadPoolData->threads[i] != 0){
                    if(pthread_create(&threadPoolData->threads[i], NULL, workerThread, (void *)threadPoolData) == 0){
                        count++;
                        liveThreadNum++;
                    }else{
                        cerr << "创建线程失败" << strerror(errno) << endl;
                    }
                }
            }
        }
        pthread_mutex_unlock(&mutex);

        /* 判断是否需要销毁多余的线程：忙线程x2 < 存活线程数 && 存活线程数 > 最小线程数 */
        if(busyThreadNum * 2 < liveThreadNum && liveThreadNum > minThreadSize){
            /* 一次销毁DEFAULT_THREAD_NUM个线程 */
            /* 
                说明：不是由管理者线程直接销毁，而是由子线程自毁
                    采取的方式是通知那些阻塞在等待任务条件变量上的线程，让他们自行判断是否需要自行销毁
             */
            pthread_mutex_lock(&mutex);
            threadPoolData->waitExitThreadNum = DEFAULT_THREAD_NUM;
            pthread_mutex_unlock(&mutex);
            for(int i = 0; i < DEFAULT_THREAD_NUM; i++){
                pthread_cond_signal(&threadPoolData->queueNotEmpty);
            }
        }

    }
    
}
//子线程回调函数
void* ThreadPool::workerThread( void *arg){
    ThreadPoolData *threadPoolData = static_cast<ThreadPoolData*>(arg);
    pthread_mutex_t mutex = threadPoolData->mutex;
    threadpoolTask task;
    while(threadPoolData->isRunning){
        /* 加锁 */
        pthread_mutex_lock(&mutex);
        /* 当任务队列里无任务时，阻塞在while中 */
        while(threadPoolData->taskQueue.empty() && threadPoolData->isRunning){
            /* 等待条件变量 */
            pthread_cond_wait(&threadPoolData->queueNotEmpty, &mutex);
            /* 清楚空闲状态的线程 */
            if(threadPoolData->waitExitThreadNum > 0){
                threadPoolData->waitExitThreadNum--;
                //Q：退出时，threadPoolData->threads数组中的相应元素值怎么置 0？
                //怎么才能找到该元素
            }
        }
        /* 关闭线程池*/
        if(!threadPoolData->isRunning){
            pthread_mutex_unlock(&mutex);
            cout << "线程" << pthread_self() << "退出" << endl;
            pthread_detach(pthread_self());
            pthread_exit(NULL);
        }
        /* 从任务队列中取出一个任务 */
        task.callBack = threadPoolData->taskQueue.front().callBack;
        task.arg = threadPoolData->taskQueue.front().arg;
        /* 从任务队列中删除一个任务 */
        threadPoolData->taskQueue.pop_front();
        /* 通知新任务可以进入任务队列中 */
        pthread_cond_broadcast(&threadPoolData->queueNotFull);
        /* 释放锁 */
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(NULL);
}
// 销毁线程池
void ThreadPool::destoryThreadPool(){

}