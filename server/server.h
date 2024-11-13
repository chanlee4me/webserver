#ifndef SERVER_H
#define SERVER_H
#include <cstring>
#include <netinet/in.h> // for sockaddr_in
#include "sys/socket.h"
#include <iostream>
#include <unistd.h>
#include "epoll.h"

#define TIME_OUT 5000
#define OPEN_MAX 1024
class Server
{
private:
    Epoll* myEpoll;                  //epoll对象
    int listenFd;                   //监听客户端连接的套接字
    sockaddr_in serverAddr;         //服务器地址结构体
    epoll_event* tempEvent;         //指向临时事件的指针
    epoll_event events[OPEN_MAX];   //指向就绪事件的数组
    int nReady;                      //成功监听到的事件数
public:
    Server();
    ~Server();
    bool createSocket();            //创建socket
    bool bindSocket();              //绑定socket
    bool setListen(int backlog);    //设置连接上限
    void listenEvent();             //初始化监听事件
    void start();                   //启动服务器
    
};



#endif