#ifndef EPOLL_H
#define EPOLL_H
#include "../threadPool/threadPool.h"
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <functional>
#include <sys/epoll.h>
#define BUFF_SIZE 1024
#define OPEN_MAX 1024
class Epoll
{
private:
    int epfd;   //指向监听红黑树根节点的文件描述符

public:
    struct EventInfo{                 //自定义的用于epoll反应堆的结构体
        int fd; //要监听的文件描述符
        int events; //对应的监听事件
        void* arg;  //泛型参数，用于传参
        std::function<void(void*)> callBack;
        // void (*callBack)(void* arg);    //回调函数
        int status; //是否处于监听态：1->在红黑树上(监听), 0->不在(不监听) 
        char buf[BUFF_SIZE];
        long lastActive;    //记录加入红黑树的时间
        EventInfo();
        ~EventInfo();
    };  
    Epoll();
    Epoll(int size);                     //size:监听节点数量
    ~Epoll();
    bool addEvent(int events, EventInfo* eventInfo);//添加fd到监听红黑树上
    bool delEvent(EventInfo* eventInfo);//摘除fd到监听红黑树上
    bool modEvent(int events, EventInfo* eventInfo);//修改监听红黑树上的fd
    int waitEvent(ThreadPool *threadPool, int maxEvents, epoll_event* events, int timeout);//阻塞监听红黑树上的事件
};

#endif





