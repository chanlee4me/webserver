#include "threadPool.h"
using namespace std;
ThreadPool::threadpoolTask::threadpoolTask(){
    arg = nullptr;
}
ThreadPool::threadpoolTask::~threadpoolTask(){
    arg = nullptr;
}
ThreadPool::ThreadPoolData::ThreadPoolData() 
    : MIN_THREAD_SIZE(2), 
      maxThreadSize(10), 
      liveThreadNum(0), 
      busyThreadNum(0), 
      waitExitThreadNum(0), 
    //   taskQueueMaxSize(20), 
      isRunning(true)      
{     
    bool initSuccess = true;
    do{
        if (pthread_mutex_init(&mutex, NULL) != 0 || 
            pthread_cond_init(&queueNotEmpty, NULL) != 0) {
            cerr << "failed to initialize Mutex or conditionnal variable" << strerror(errno) << endl;
            initSuccess = false;
            break;
        }
        /*根据最大线程上限，为工作线程数组分配空间*/
        threads.resize(maxThreadSize);
        if(threads.size() != maxThreadSize){
            cerr << "failed to initialize threads array" << endl;
            initSuccess = false;
            break;
        }else{
            fill(threads.begin(), threads.end(), 0);  // 清零
        }
    }while(0);
    if (!initSuccess) {
        isRunning = false;
    }
}
ThreadPool::ThreadPoolData::~ThreadPoolData(){
    pthread_mutex_destroy(&mutex);
    // pthread_mutex_destroy(&counterMutex);
    pthread_cond_destroy(&queueNotEmpty);
    // pthread_cond_destroy(&queueNotFull);
}
ThreadPool::ThreadPool() {
    do {
        threadPoolData = new ThreadPoolData();
        pthread_mutex_t mutex = threadPoolData->mutex;
        /*创建任务线程*/
        for (int i = 0; i < threadPoolData->MIN_THREAD_SIZE; i++) {
            if(pthread_create(&threadPoolData->threads[i], NULL, workerThread, (void *)threadPoolData) == 0){
                pthread_detach(threadPoolData->threads[i]);//在创建时将线程设置为分离态，简化线程资源管理
                cout << "create thread：" << i + 1<< endl;
                pthread_mutex_lock(&mutex);
                threadPoolData->liveThreadNum++;
                pthread_mutex_unlock(&mutex);
            }else{
                cerr << "create thread error" << strerror(errno) << endl;
            }
        }
        // 创建管理线程
        if(pthread_create(&threadPoolData->adjustThreadId, NULL, adjustThreadPool, (void *)threadPoolData)){
            cerr << "create manager thread error" << strerror(errno) << endl;
            break;
        }else{
            cout << "create adjustThread" << endl;
        }
        cout << "successfully create threadPool" << endl;
    } while (0);
}
ThreadPool::~ThreadPool() {
    if (threadPoolData != nullptr){
        /* 置线程池状态为关闭 */
        threadPoolData->isRunning = false;
        /* 销毁管理者线程 */
        pthread_join(threadPoolData->adjustThreadId, nullptr);
        /* 通知所有空闲线程去自行销毁 */
        for(int i = 0; i < threadPoolData->liveThreadNum; i++){
            pthread_cond_broadcast(&threadPoolData->queueNotEmpty);
        }
        /* 由于在创建线程时设置为分离态，剩下在执行任务的线程会在执行完毕后自行销毁 */
        delete threadPoolData;
        threadPoolData = nullptr;
    }
}

// 管理线程回调函数
void* ThreadPool::adjustThreadPool(void *arg){
    ThreadPoolData *_threadPoolData = static_cast<ThreadPoolData*>(arg);
    pthread_mutex_t mutex = _threadPoolData->mutex;
    while (_threadPoolData->isRunning){
        /* 定时对线程池进行管理 */
        sleep(5);
        /* 加锁 */
        pthread_mutex_lock(&mutex);
        /* 获取数据 */
        int minThreadSize = _threadPoolData->MIN_THREAD_SIZE;//线程池最小线程数
        int maxThreadSize = _threadPoolData->maxThreadSize;//线程池最大线程数
        int taskQueueSize = _threadPoolData->taskQueue.size();//当前任务数
        int liveThreadNum = _threadPoolData->liveThreadNum;//存活线程数
        int busyThreadNum = _threadPoolData->busyThreadNum;//忙线程数  
        /* 判断是否创建新线程：任务数大于线程池最小线程数，且存活的线程数小于线程池最大线程数 */
        if(taskQueueSize > minThreadSize && liveThreadNum < maxThreadSize){
            /* 一次性增加DEFAULT_THREAD_NUM个线程 */
            int count = 0;
            for(int i = 0; i < maxThreadSize && count < DEFAULT_THREAD_NUM && liveThreadNum < maxThreadSize; i++){
                if(_threadPoolData->threads[i] == 0){
                    if(pthread_create(&_threadPoolData->threads[i], NULL, workerThread, (void *)_threadPoolData) == 0){
                        count++;
                        liveThreadNum++;
                    }else{
                        cerr << "failed to add new thread" << strerror(errno) << endl;
                    }
                }
            }
        }
        pthread_mutex_unlock(&mutex);
        /* 考虑任务队列和线程池负载历史,来决定是否销毁线程 */
        // if(busyThreadNum * 3 < liveThreadNum && liveThreadNum > minThreadSize){
        //     // 冷却期：如果销毁线程的条件在过去 3 次判断中都成立，才执行销毁
        //     static int destroyCount = 0;
        //     destroyCount++;
        //     /* 一次销毁DEFAULT_THREAD_NUM个线程 */
        //     /* 
        //         说明：不是由管理者线程直接销毁，而是由子线程自毁
        //             采取的方式是通知那些阻塞在等待任务条件变量上的线程，让他们自行判断是否需要自行销毁
        //      */
        //     if(destroyCount >= 3){
        //         int waitToExitThreadNum = busyThreadNum * 2 - liveThreadNum;
        //         _threadPoolData->waitExitThreadNum = waitToExitThreadNum;
        //         pthread_mutex_unlock(&mutex);
        //         for(int i = 0; i < waitToExitThreadNum; i++){
        //             pthread_cond_signal(&_threadPoolData->queueNotEmpty);
        //         }
        //     }else{
        //         pthread_mutex_unlock(&mutex);
        //     }
        // }else{
        //    pthread_mutex_unlock(&mutex);
        // }
    }
    return nullptr;
}
//子线程回调函数
void* ThreadPool::workerThread(void *arg){
    ThreadPoolData *_threadPoolData = static_cast<ThreadPoolData*>(arg);
    pthread_mutex_t mutex = _threadPoolData->mutex;
    threadpoolTask task;
    pthread_mutex_lock(&mutex);/* 加锁 */
    while(_threadPoolData->isRunning){
        /* 当任务队列里无任务时，阻塞在while中 */
        while(_threadPoolData->isRunning && _threadPoolData->taskQueue.empty()){
            /* 等待条件变量 */
            pthread_cond_wait(&_threadPoolData->queueNotEmpty, &mutex);
            
            /* 
                清除空闲状态的线程:若当前存活线程个数大于最小线程数，则可以结束当前线程
                To be continued...
            */
 
        }
        /* 关闭线程池*/
        if(!_threadPoolData->isRunning){
            _threadPoolData->liveThreadNum--;
            pthread_mutex_unlock(&mutex);
            cout << "thread 0x" << pthread_self() << " exit" << endl;
            pthread_exit(nullptr);
        }
        /* 从任务队列中取出一个任务 */
        task.callBack = _threadPoolData->taskQueue.front().callBack;
        task.arg = _threadPoolData->taskQueue.front().arg;
        /* 从任务队列中删除一个任务 */
        _threadPoolData->taskQueue.pop_front();

        /* 
            通知新任务可以进入任务队列中 
            To be continued...
        */

        /* 执行任务 */
        _threadPoolData->busyThreadNum++; //忙线程数+1
        pthread_mutex_unlock(&mutex);    //解锁

        cout << "thread 0x" << pthread_self() << "start working" << endl;
        task.callBack(task.arg);    //执行任务

        pthread_mutex_lock(&mutex);      //加锁
        _threadPoolData->busyThreadNum--; //忙线程数-1
        pthread_mutex_unlock(&mutex);    //解锁
        cout << "thread 0x" << pthread_self() << "finished woking" << endl;
    }
    pthread_exit(NULL);
    return nullptr;
}
//添加任务
void ThreadPool::addTask(function<void(void *)> callBack, void *arg){
    pthread_mutex_t mutex = threadPoolData->mutex;
    /* 如果线程池已经停止运行，不能再添加任务 */ 
    if(!threadPoolData->isRunning) {
        return;
    }
    pthread_mutex_lock(&mutex);

    /* 
        队列已满，调用 wait 阻塞
        To be continued...
     */

    /* 添加任务 */
    threadpoolTask task;
    task.arg = arg;
    task.callBack = callBack;
    threadPoolData->taskQueue.push_back(task);
    pthread_mutex_unlock(&mutex);
    /* 通知线程有任务到来 */
    pthread_cond_signal(&threadPoolData->queueNotEmpty);
}


