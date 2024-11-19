#include "threadPool.h"
using namespace std;
ThreadPool::ThreadPoolData::ThreadPoolData() 
    : MIN_THREAD_SIZE(2), 
      maxThreadSize(10), 
      liveThreadNum(0), 
      busyThreadNum(0), 
      waitExitThreadNum(0), 
      taskQueueMaxSize(20), 
      isRunning(true)      
{     
    bool initSuccess = true;
    do{
        if (pthread_mutex_init(&mutex, NULL) != 0 || 
            pthread_cond_init(&queueNotEmpty, NULL) != 0 ||
            pthread_cond_init(&queueNotFull, NULL) != 0) {
            cerr << "初始化互斥锁或条件变量失败" << strerror(errno) << endl;
            initSuccess = false;
            break;
        }
        /*根据最大线程上限，为工作线程数组分配空间*/
        threads.resize(maxThreadSize);
        if(threads.size() != maxThreadSize){
            cerr << "线程数组分配空间失败" << endl;
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
    pthread_cond_destroy(&queueNotFull);
}
ThreadPool::ThreadPool() {
    do {
        threadPoolData = new ThreadPoolData();
        pthread_mutex_t mutex = threadPoolData->mutex;
        /*创建任务线程*/
        for (int i = 0; i < threadPoolData->MIN_THREAD_SIZE; i++) {
            if(pthread_create(&threadPoolData->threads[i], NULL, workerThread, (void *)threadPoolData) == 0){
                pthread_detach(threadPoolData->threads[i]);//在创建时将线程设置为分离态，简化线程资源管理
                cout << "创建线程数：" << i + 1<< endl;
                pthread_mutex_lock(&mutex);
                threadPoolData->liveThreadNum++;
                pthread_mutex_unlock(&mutex);
            }else{
                cerr << "创建线程失败" << strerror(errno) << endl;
            }
        }
        // 创建管理线程
        if(pthread_create(&threadPoolData->adjustThreadId, NULL, adjustThreadPool, (void *)threadPoolData)){
            cerr << "创建管理线程失败" << strerror(errno) << endl;
            break;
        }else{
            cout << "创建管理线程" << endl;
        }
        cout << "线程池创建完成" << endl;
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
        sleep(10);
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
                        cerr << "创建线程失败" << strerror(errno) << endl;
                    }
                }
            }
        }
        /* 考虑任务队列和线程池负载历史,来决定是否销毁线程 */
        if(busyThreadNum * 3 < liveThreadNum && liveThreadNum > minThreadSize){
            // 冷却期：如果销毁线程的条件在过去 3 次判断中都成立，才执行销毁
            static int destroyCount = 0;
            destroyCount++;
            /* 一次销毁DEFAULT_THREAD_NUM个线程 */
            /* 
                说明：不是由管理者线程直接销毁，而是由子线程自毁
                    采取的方式是通知那些阻塞在等待任务条件变量上的线程，让他们自行判断是否需要自行销毁
             */
            if(destroyCount >= 3){
                int waitToExitThreadNum = busyThreadNum * 2 - liveThreadNum;
                _threadPoolData->waitExitThreadNum = waitToExitThreadNum;
                pthread_mutex_unlock(&mutex);
                for(int i = 0; i < waitToExitThreadNum; i++){
                    pthread_cond_signal(&_threadPoolData->queueNotEmpty);
                }
            }else{
                pthread_mutex_unlock(&mutex);
            }
        }else{
           pthread_mutex_unlock(&mutex);
        }
    }
    return nullptr;
}
//子线程回调函数
void* ThreadPool::workerThread(void *arg){
    ThreadPoolData *_threadPoolData = static_cast<ThreadPoolData*>(arg);
    pthread_mutex_t mutex = _threadPoolData->mutex;
    // pthread_mutex_t counterMutex = _threadPoolData->counterMutex;
    threadpoolTask task;
    pthread_mutex_lock(&mutex);/* 加锁 */
    while(_threadPoolData->isRunning){
        /* 当任务队列里无任务时，阻塞在while中 */
        while(_threadPoolData->taskQueue.empty() && _threadPoolData->isRunning){
            /* 等待条件变量 */
            pthread_cond_wait(&_threadPoolData->queueNotEmpty, &mutex);
            /* 清除空闲状态的线程 */
            /* 若当前存活线程个数大于最小线程数，则可以结束当前线程 */
            if(_threadPoolData->waitExitThreadNum > 0 && _threadPoolData->MIN_THREAD_SIZE){
                //Q：退出时，threadPoolData->threads数组中的相应元素值需要置0，怎么才能找到该元素
                //A：利用 pthread_self()来查找其在 threads 数组中相应位置
                for(int i = 0; i < _threadPoolData->threads.size(); i++){
                    if(_threadPoolData->threads[i] == pthread_self()){
                        _threadPoolData->threads[i] = 0;
                        _threadPoolData->waitExitThreadNum--;
                        _threadPoolData->liveThreadNum--;
                        pthread_mutex_unlock(&mutex);
                        cout << "线程" << pthread_self() << "终止" << endl;
                        pthread_exit(nullptr);
                    }
                }
            }
        }
        /* 关闭线程池*/
        if(!_threadPoolData->isRunning){
            _threadPoolData->liveThreadNum--;
            pthread_mutex_unlock(&mutex);
            cout << "线程" << pthread_self() << "退出" << endl;
            pthread_exit(nullptr);
        }
        /* 从任务队列中取出一个任务 */
        task.callBack = _threadPoolData->taskQueue.front().callBack;
        task.arg = _threadPoolData->taskQueue.front().arg;
        /* 从任务队列中删除一个任务 */
        _threadPoolData->taskQueue.pop_front();
        /* 通知新任务可以进入任务队列中 */
        pthread_cond_signal(&_threadPoolData->queueNotFull);
        /* 释放锁 */
        pthread_mutex_unlock(&mutex);
        /* 执行任务 */
        pthread_mutex_lock(&mutex);      //加锁
        cout << "线程" << pthread_self() << "开始执行任务" << endl;
        _threadPoolData->busyThreadNum++; //忙线程数+1
        pthread_mutex_unlock(&mutex);    //解锁
        task.callBack(task.arg);    //执行任务
        /* 任务执行完毕 */
        pthread_mutex_lock(&mutex);      //加锁
        cout << "线程" << pthread_self() << "任务执行完毕" << endl;
        _threadPoolData->busyThreadNum--; //忙线程数-1
        pthread_mutex_unlock(&mutex);    //解锁
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
    /* 通知线程有任务到来 */
    pthread_cond_signal(&threadPoolData->queueNotEmpty);
    pthread_mutex_unlock(&mutex);
}
// 销毁线程池
void ThreadPool::destoryThreadPool(){

}   


