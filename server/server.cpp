#include "server.h"


using namespace std;
Server::Server()
{
    listenFd = -1;
}
Server::~Server() { // 添加析构函数定义
    if (listenFd != -1) {
        close(listenFd);
    }
}
void Server::listenEvent(){
    createSocket();
    bindSocket();
    setListen(OPEN_MAX);
    myEpoll = new Epoll(OPEN_MAX);                        //初始化 epoll 监听红黑树
    tempEvent->events = EPOLLIN | EPOLLET;
    tempEvent->data.fd = listenFd;
    myEpoll->add(tempEvent);
}
    
void Server::start(){
    listenEvent();
    //反应堆模型

    while(1){
        nReady = myEpoll->wait(OPEN_MAX, events, TIME_OUT);
        if(nReady == 0) continue;
        if(nReady == -1) break;
        for(int i = 0; i < nReady; i++){
            if(!(events[i].events & EPOLLIN))
                continue;
            if(events[i].data.fd == listenFd){//满足事件为 listenFd
                sockaddr_in clientAddr;
                socklen_t clientAddrLen = sizeof(clientAddr);
                int connectFd = accept(listenFd, (sockaddr*)&clientAddr, &clientAddrLen);
                tempEvent->events = EPOLLIN | EPOLLET;
                tempEvent->data.fd = connectFd;
                myEpoll->add(tempEvent);
            }else{//满足事件为connectFd
                char buf[1024];
                int n = read(events[i].data.fd, buf, sizeof(buf));
                if(n == 0){//客户端关闭连接
                    tempEvent->data.fd = events[i].data.fd;
                    myEpoll->del(tempEvent);
                    close(tempEvent->data.fd);
                }else if(n < 0){
                    cerr << "read error" << strerror(errno) << endl;
                    tempEvent->data.fd = events[i].data.fd;
                    myEpoll->del(tempEvent);
                    close(tempEvent->data.fd);
                }else{
                    write(STDOUT_FILENO, buf, n);
                }
            }
        }
    }
}
bool Server::createSocket(){
    listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenFd == -1){
        cerr << "socket error" << strerror(errno) << endl;
        return false;
    }
    return true;
}
bool Server::bindSocket(){
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(listenFd, (sockaddr*)&serverAddr, sizeof(serverAddr)) == -1){
        cerr << "bind error" << strerror(errno) << endl;
        return false;
    }
    return true;
}
bool Server::setListen(int backlog){
    if(listen(listenFd, backlog) == -1){
        cerr << "listen error" << strerror(errno) << endl;
        return false;
    }
    return true;
}


