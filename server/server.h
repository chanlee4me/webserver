#ifndef SERVER_H
#define SERVER_H
#include <cstring>
#include <netinet/in.h> // for sockaddr_in
#include "sys/socket.h"
#include <iostream>
#include <unistd.h>
#include "epoll.h"
#include <functional> // For std::function and std::bind
#define TIME_OUT 5000
#define MAX_EVENTS 1024
class Server
{
private:
    Epoll* myEpoll;                  //epoll对象
    int listenFd;                   //监听客户端连接的套接字
    sockaddr_in serverAddr;         //服务器地址结构体
    epoll_event* tempEvent;         //指向epoll临时监听事件的指针
    epoll_event events[OPEN_MAX];   //指向epoll就绪事件的数组
    int nReady;                     //epoll成功监听到的事件数
    Epoll::EventInfo myEvents[MAX_EVENTS + 1]; //用于反应堆的结构体数组 
                
public:
    Server();
    ~Server();
    bool createSocket();            //创建socket
    bool bindSocket();              //绑定socket
    bool setListen(int backlog);    //设置连接上限
    void initListen();             //初始化监听事件
    void acceptConnect();           //与客户端建立连接
    void start();                   //启动服务器
    void reactor();                 //反应堆模型
    
    void setEventInfo(Epoll::EventInfo* eventInfo, int fd, std::function<void(void*)> callBack, void* arg);  //设置EventInfo结构体变量
    void recvData(void* arg); //接收数据
    void sendData(void* arg); //发送数据
};



#endif