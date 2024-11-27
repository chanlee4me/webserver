#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H
#include <string>
#include <cassert>
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include "../buffer/buffer.h"
class HttpRequest{
public:

    enum Method{
        kInvalid, 
        kGet, 
        kPost, 
        kHead, 
        kPut, 
        kDelete
    };
    enum Version{
        kUnknown, kHttp10, kHttp11
    };
    
    HttpRequest();
    ~HttpRequest();

    Method getMethod() const;
    bool setMethod(const char* start, const char* end);

    string getPath();
    void setPath(const char* start, const char* end);

    string getQuery();
    void setQuery(const char* start, const char* end);

    Version getVersion();
    void setVersion(int v);

    string getHeader(const string &key);
    void addHeader(string& key, string& value);

    string getPost(const string &key);
    bool setPost(const char* start, const char* end);

    
private:
    Method method;  //请求方法
    string path;     //请求的URL
    string query;   //请求的参数
    Version version; //HTTP版本号
    unordered_map<string, string> headers; //请求头部
    unordered_map<string, string> post; //post请求数据


};



#endif