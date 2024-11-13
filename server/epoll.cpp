#include "epoll.h"
using namespace std;
Epoll::Epoll()
{
    epfd = -1;
}
Epoll::Epoll(int size)
{
    if(!create(size)){
        cerr << "epoll_create error:" << strerror(errno) << endl;
    }
}
Epoll::~Epoll() { // 添加析构函数定义
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

bool Epoll::add(epoll_event *events){
    //添加fd到监听红黑树上
    if(epoll_ctl(epfd, EPOLL_CTL_ADD, events->data.fd, events) == -1){
        cerr << "epoll_ctl add error:" << strerror(errno) << endl;
        return false;
    }
    return true;
}

bool Epoll::del(epoll_event *events){
    //摘除fd到监听红黑树上
    if(epoll_ctl(epfd, EPOLL_CTL_DEL, events->data.fd, events) == -1){
        cerr << "epoll_ctl del error:" << strerror(errno) << endl;
        return false;
    }
    return true;
}

bool Epoll::mod(epoll_event *events){
    //修改监听红黑树上的fd
    if(epoll_ctl(epfd, EPOLL_CTL_MOD, events->data.fd, events) == -1){
        cerr << "epoll_ctl mod error:" << strerror(errno) << endl;
        return false;
    }
    return true;
}

int Epoll::wait(int maxEvents, epoll_event* events, int timeout){
    /*
        阻塞监听红黑树上的事件
        n > 0 满足监听的总个数；n == 0 无fd满足监听事件； n < 0 失败
    */
    int n = epoll_wait(epfd, events, maxEvents, timeout);
    if(n == -1){
        cerr << "epoll_wait error:" << strerror(errno) << endl;
        return -1;
    }
    return n;
}