#ifndef HTTP_CONN_H
#define HTTP_CONN_H
#include "../server/server.h"
#include "../buffer/buffer.h"
#include <netinet/in.h> //sockaddr_in
#include "httpContent.h"
#include "httpResponse.h"
class HttpServer
{
private:
    /* data */
public:
    HttpServer();
    ~HttpServer();

    void start();
private:
    
    Server server;

    Buffer readBuffer;
    Buffer writeBuffer;

    HttpRequest request;
    HttpResponse response;
};
#endif