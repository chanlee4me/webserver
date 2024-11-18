#include "epoll.h"
using namespace std;
Epoll::Epoll()
{
    epfd = -1;
}
Epoll::Epoll(int size)
{   //构造监听红黑树
    create(size);
}
Epoll::~Epoll() { 
    if (epfd != -1) {
        close(epfd);
    }
}
bool Epoll::create(int size){
    //创建监听红黑树
    epfd = epoll_create(size);
    if(epfd == -1){
        cerr << "epoll_create error:" << strerror(errno) << endl;
        return false;
    }
    return true;
}

bool Epoll::addEvent(int events, EventInfo* eventInfo){
    epoll_event tempEvent;
    tempEvent.events = events;
    tempEvent.data.ptr = eventInfo;
    eventInfo->events = events;
    if(eventInfo->status == 1){
        cerr << "eventAdd error: the fd has been added" << endl;
    }else{
        if(epoll_ctl(epfd, EPOLL_CTL_ADD, eventInfo->fd, &tempEvent) == -1){
            cerr << "epoll_ctl add error:" << strerror(errno) << endl;
            return false;
        }
        eventInfo->status = 1;
        eventInfo->lastActive = time(NULL);
    }
    //添加fd到监听红黑树上
    return true;
}

bool Epoll::delEvent(EventInfo* eventInfo){
    //摘除监听红黑树上的fd
    if(epoll_ctl(epfd, EPOLL_CTL_DEL, eventInfo->fd, NULL) == -1){
        cerr << "epoll_ctl del error:" << strerror(errno) << endl;
        return false;
    }
    eventInfo->status = 0;  //状态为 0 标志着不在监听态
    return true;
}

bool Epoll::modEvent(int events, EventInfo* eventInfo){
    epoll_event tempEvent;
    tempEvent.data.ptr = eventInfo;
    tempEvent.events = eventInfo->events = events;
    eventInfo->events = events;
    //修改监听红黑树上的fd
    if(epoll_ctl(epfd, EPOLL_CTL_MOD, eventInfo->fd, &tempEvent) == -1){
        cerr << "epoll_ctl mod error:" << strerror(errno) << endl;
        return false;
    }
    return true;
}

int Epoll::waitEvent(ThreadPool *threadPool, int maxEvents, epoll_event* events, int timeout){
    /*
        阻塞监听红黑树上的事件
        n > 0 满足监听的总个数；n == 0 无fd满足监听事件； n < 0 失败
    */
    int n = epoll_wait(epfd, events, maxEvents, timeout);
    if(n == -1){
        cerr << "epoll_wait error:" << strerror(errno) << endl;
        return -1;
    }
    for(int i = 0; i < n; i++){//处理所有监听到的事件
        EventInfo* eventInfo = static_cast<EventInfo*>(events[i].data.ptr);
        if(eventInfo->events & EPOLLIN){
            //将监听到的事件添加到任务队列中
            threadPool->addTask(eventInfo->callBack, eventInfo->arg);
        }else if(eventInfo->events & EPOLLOUT){
            //将监听到的事件添加到任务队列中
            threadPool->addTask(eventInfo->callBack, eventInfo->arg);
        }
    }
    return n;
}