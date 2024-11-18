#include "server/epoll.h"
#include "server/server.h"


int main(){
    Server* server = new Server();
    server->start();
    return 0;
}