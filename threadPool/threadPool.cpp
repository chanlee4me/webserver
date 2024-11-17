#include "threadPool.h"
using namespace std;

ThreadPool::ThreadPool() {
    createThreadPool();
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
        if (pthread_mutex_init(&threadPoolData->mutex, NULL) && pthread_mutex_init(&threadPoolData->counterMutex, NULL) && pthread_cond_init(&threadPoolData->queueNotEmpty, NULL) && pthread_cond_init(&threadPoolData->queueNotFull, NULL)) {
            cerr << "初始化互斥锁或条件变量失败" << strerror(errno) << endl;
            break;
        }

        /*创建任务线程*/
        for (int i = 0; i < threadPoolData->MIN_THREAD_SIZE; i++) {
            if(pthread_create(&threadPoolData->threads[i], NULL, workerThread, (void *)threadPoolData) != 0){
                cerr << "创建线程失败" << strerror(errno) << endl;
                break;
            }else{
                pthread_detach(threadPoolData->threads[i]);//在创建时将线程设置为分离态，简化线程资源管理
                cout << "创建线程数：" << i + 1<< endl;
            }
        }
        // 创建管理线程
        if(pthread_create(&threadPoolData->adjustThreadId, NULL, adjustThreadPool, (void *)threadPoolData)){
            cerr << "创建管理线程失败" << strerror(errno) << endl;
            break;
        }else{
            cout << "创建管理线程" << endl;
        }
    } while (0);
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
    return nullptr;
}
//子线程回调函数
void* ThreadPool::workerThread(void *arg){
    ThreadPoolData *threadPoolData = static_cast<ThreadPoolData*>(arg);
    pthread_mutex_t mutex = threadPoolData->mutex;
    pthread_mutex_t counterMutex = threadPoolData->counterMutex;
    threadpoolTask task;
    while(threadPoolData->isRunning){
        /* 加锁 */
        pthread_mutex_lock(&mutex);
        /* 当任务队列里无任务时，阻塞在while中 */
        while(threadPoolData->taskQueue.empty() && threadPoolData->isRunning){
            /* 等待条件变量 */
            pthread_cond_wait(&threadPoolData->queueNotEmpty, &mutex);
            /* 清除空闲状态的线程 */
            if(threadPoolData->waitExitThreadNum > 0){
                threadPoolData->waitExitThreadNum--;
                /* 若当前存活线程个数大于最小线程数，则可以结束当前线程 */
                if(threadPoolData->liveThreadNum > threadPoolData->MIN_THREAD_SIZE){
                    //Q：退出时，threadPoolData->threads数组中的相应元素值需要置0，怎么才能找到该元素
                    //A：利用 pthread_self()来查找其在 threads 数组中相应位置
                    for(int i = 0; i < threadPoolData->threads.size(); i++){
                        if(threadPoolData->threads[i] == pthread_self()){
                            threadPoolData->threads[i] = 0;
                        }
                    }
                    threadPoolData->liveThreadNum--;
                    pthread_mutex_unlock(&mutex);
                    cout << "线程" << pthread_self() << "终止" << endl;
                    pthread_exit(nullptr);
                }
            }
        }
        /* 关闭线程池*/
        if(!threadPoolData->isRunning){
            threadPoolData->liveThreadNum--;
            pthread_mutex_unlock(&mutex);
            cout << "线程" << pthread_self() << "退出" << endl;
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
        /* 执行任务 */
        cout << "线程" << pthread_self() << "开始执行任务" << endl;
        pthread_mutex_lock(&counterMutex);      //加锁
        threadPoolData->busyThreadNum++; //忙线程数+1
        pthread_mutex_unlock(&counterMutex);    //解锁
        task.callBack(task.arg);    //执行任务
        /* 任务执行完毕 */
        cout << "线程" << pthread_self() << "任务执行完毕" << endl;
        pthread_mutex_lock(&counterMutex);      //加锁
        threadPoolData->busyThreadNum--; //忙线程数-1
        pthread_mutex_unlock(&counterMutex);    //解锁
    }
    pthread_exit(NULL);
    return nullptr;
}
//添加任务

void ThreadPool::addTask(function<void(void *)> callBack, void *arg){
    pthread_mutex_t mutex = threadPoolData->mutex;
    pthread_mutex_lock(&mutex);
    /* 如果线程池已经停止运行，不能再添加任务 */ 
    if(!threadPoolData->isRunning) {
        pthread_mutex_unlock(&mutex);
        return;
    }
    /* 队列已满，调用 wait 阻塞 */
    if(threadPoolData->taskQueue.size() == threadPoolData->taskQueueMaxSize){
        pthread_cond_wait(&threadPoolData->queueNotFull, &mutex);
    }
    /* 添加任务 */
    threadpoolTask task = {callBack, arg};
    threadPoolData->taskQueue.push_back(task);
    pthread_mutex_unlock(&mutex);
    /* 通知线程有任务到来 */
    pthread_cond_signal(&threadPoolData->queueNotEmpty);
}
// 销毁线程池
void ThreadPool::destoryThreadPool(){
    if(!threadPoolData->isRunning){
        return;
    }
    /* 置线程池状态为关闭 */
    threadPoolData->isRunning = false;
    /* 销毁管理者线程 */
    pthread_join(threadPoolData->adjustThreadId, nullptr);
    /* 通知所有空闲线程去自行销毁 */
    for(int i = 0; i < threadPoolData->liveThreadNum; i++){
        pthread_cond_broadcast(&threadPoolData->queueNotEmpty);
    }
    /* 由于在创建线程时设置为分离态，剩下在执行任务的线程会在执行完毕后自行销毁 */

    /* 释放资源 */
    pthread_mutex_destroy(&threadPoolData->mutex);
    pthread_mutex_destroy(&threadPoolData->counterMutex);
    pthread_cond_destroy(&threadPoolData->queueNotEmpty);
    pthread_cond_destroy(&threadPoolData->queueNotFull);

    delete threadPoolData;
    threadPoolData = nullptr;
}
