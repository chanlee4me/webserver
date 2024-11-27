#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H
#include "httpRequest.h"
#include <string>
#include <map>

using namespace std;

class HttpResponse{
public:
    enum HttpStatusCode {
        OK = 200,
        BadRequest = 400,
        Forbidden = 403,
        NotFound = 404,
    };
    HttpResponse():
        version(HttpRequest::kUnknown),
        statusCode(-1),
        statusMessage(""),
        headers(),
        body("")
    {}
    ~HttpResponse();
    void setVersion(HttpRequest::Version version){
        this->version = version;
    }
    
    void setStatusCode(int statusCode){
        this->statusCode = statusCode;
    }
    
    void setStatusMessage(string statusMessage){
        this->statusMessage = statusMessage;
    }
    
    void setHeader(string key, string value){
        headers[key] = value;
    }
    
    void setBody(string body){
        this->body = body;
    }
 
    void appendToBuffer(Buffer* output);
 private:
    HttpRequest::Version version;
    int statusCode;
    string statusMessage;
    map<string, string> headers;
    string body;
    static const string CRLF;
};
#endif