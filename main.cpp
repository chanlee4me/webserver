#include "server/epoll.h"
#include "server/server.h"


int main(){
    try{
        Server* server = new Server();
        server->start();
        delete server;
    }catch(const std::exception& e){
        std::cerr << e.what() << std::endl;
        return -1;
    }
    return 0;
}