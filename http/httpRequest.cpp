#include "httpRequest.h"
using namespace std;

HttpRequest::Method HttpRequest::getMethod() const{
    return method;
}

string HttpRequest::getPath() const{
    return path;
}

string HttpRequest::getQuery() const{
    return query;
}

HttpRequest::Version HttpRequest::getVersion() const{
    return version;
}

string HttpRequest::getHeader(const string &key) const{
    return headers.at(key);
}

string HttpRequest::getPost(const string &key) const{
    return post.at(key);
}


bool HttpRequest::setMethod(const char* start, const char* end){
    assert(method == kInvalid);
    string m(start, end);
    if(m == "GET"){
        method = kGet;
    }else if(m == "POST"){
        method = kPost;
    }else if(m == "HEAD"){
        method = kHead;
    }else if(m == "PUT"){
        method = kPut;
    }else if(m == "DELETE"){
        method = kDelete;
    }else{
        method = kInvalid;
    }
    return method != kInvalid;
}

void HttpRequest::setPath(const char* start, const char* end){
    path.assign(start, end);
}

void HttpRequest::setQuery(const char* start, const char* end){
    query.assign(start, end);
}

void HttpRequest::setVersion(int v){
    if(v == kHttp11){
        version = kHttp11;
    }else if(v == kHttp10){
        version = kHttp10;
    }else{
        version = kUnknown;
    }
}

void HttpRequest::addHeader(string& key, string& value){
    headers[key] = value;
}
