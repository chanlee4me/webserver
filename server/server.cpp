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
//初始化监听事件
void Server::initListen(){  
    createSocket();
    bindSocket();
    setListen(OPEN_MAX);
    myEpoll = new Epoll(OPEN_MAX);                        //初始化epoll监听红黑树
    tempEvent->events = EPOLLIN | EPOLLET;
    tempEvent->data.fd = listenFd;
    myEpoll->addEvent(tempEvent->events, &myEvents[OPEN_MAX]);    //将listenFd添加到监听红黑树上
}
void Server::acceptConnect(){//与客户端建立连接
    sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    int connectFd = accept(listenFd, (sockaddr*)&clientAddr, &clientAddrLen);
    if(connectFd == -1){
        cerr << "accept error" << strerror(errno) << endl;
        return;
    }
    do{ 
        int i;
        for(i = 0; i < MAX_EVENTS; i++){    //找到 myEvents中空闲的位置
            if(myEvents[i].status == 0) break;
        }
        if(i == MAX_EVENTS){
            cerr << "connectFd is full" << endl;
            close(connectFd);
            break;
        }
        setEventInfo(&myEvents[i], connectFd, bind(&Server::recvData, this, std::placeholders::_1), &myEvents[i]);//设置回调函数和监听的fd
        myEpoll->addEvent(tempEvent->events, &myEvents[i]);           //将监听事件挂到红黑树上
    }while(0);

}           

void Server::reactor(){
    acceptConnect();
    myEpoll->waitEvent(OPEN_MAX, events, TIME_OUT);
    
    
}
void Server::start(){
    initListen();
}
//反应堆模型
void Server::reactor(){

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
//设置MyEvent结构体变量
void Server::setEventInfo(Epoll::EventInfo* eventInfo, int fd, std::function<void(void*)> callBack, void* arg){  //设置EventInfo结构体变量

    eventInfo->fd = fd;
    eventInfo->events = 0;
    eventInfo->callBack = callBack;
    eventInfo->arg = arg;
    eventInfo->status = 0;
    memset(eventInfo->buf, 0, sizeof(eventInfo->buf));
    eventInfo->lastActive = time(NULL);
}

void Server::recvData(void* arg){
    Epoll::EventInfo* eventInfo = static_cast<Epoll::EventInfo*>(arg);
    int len = read(eventInfo->fd, eventInfo->buf, sizeof(eventInfo->buf));
    myEpoll->delEvent(eventInfo);

    if(len == 0){//客户端关闭连接
        close(eventInfo->fd);
        cout << "client close" << endl;    
    }else if(len < 0){
        cerr << "read error" << strerror(errno) << endl;
        close(eventInfo->fd);
    }else{
        cout << "recv data:" << eventInfo->buf << endl;
        int events = EPOLLOUT | EPOLLET;
        myEpoll->addEvent(events, eventInfo);
        setEventInfo(eventInfo, eventInfo->fd, bind(&Server::sendData, this, std::placeholders::_1), eventInfo);
    }
}
void Server::sendData(void* arg){ //发送数据
    Epoll::EventInfo* eventInfo = static_cast<Epoll::EventInfo*>(arg);
    int len = write(eventInfo->fd, eventInfo->buf, sizeof(eventInfo->buf));
    myEpoll->delEvent(eventInfo);

    if(len < 0){
        cerr << "write error" << strerror(errno) << endl;
    }else if(len == 0)
    {   
        close(eventInfo->fd);
        cout << "client close" << endl;
    }else{
        cout << "send data:" << eventInfo->buf << endl;
        int events = EPOLLIN | EPOLLET;
        myEpoll->addEvent(events, eventInfo);
        setEventInfo(eventInfo, eventInfo->fd, bind(&Server::recvData, this, std::placeholders::_1), eventInfo);
    }
    
    return;
}


