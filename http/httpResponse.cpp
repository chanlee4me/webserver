#include "httpResponse.h"
const string HttpResponse::CRLF = "\r\n";
using namespace std;
void HttpResponse::appendToBuffer(Buffer* buffer){
    // 写入状态行到 buffer
    if(version == HttpRequest::Version::kHttp10){
        buffer->append("HTTP/1.0 ");
    }else{  
        buffer->append("HTTP/1.1 ");
    }
    buffer->append(to_string(statusCode));
    buffer->append(" ");
    buffer->append(statusMessage);
    buffer->append(CRLF);
    
    // 写入响应头部到 buffer
    for(const auto& header : headers){
        buffer->append(header.first);
        buffer->append(": ");
        buffer->append(header.second);
        buffer->append(CRLF);
    }
}
