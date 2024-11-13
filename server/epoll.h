#ifndef EPOLL_H
#define EPOLL_H
#include <sys/epoll.h>
#include <iostream>
#include <cstring>
#include <unistd.h>
class Epoll
{
private:
    int epfd;   //指向监听红黑树根节点的文件描述符
    struct EventInfo;
    bool create(int size);               //创建监听红黑树

public:
    Epoll();
    Epoll(int size);                     //size:监听节点数量
    ~Epoll();
    bool add(epoll_event *events);//添加fd到监听红黑树上
    bool del(epoll_event *events);//摘除fd到监听红黑树上
    bool mod(epoll_event *events);//修改监听红黑树上的fd
    int wait(int maxEvents, epoll_event* events, int timeout);//阻塞监听红黑树上的事件
};

#endif





