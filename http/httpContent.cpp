#include "httpContent.h"
using namespace std;

bool HttpContent::parseRequest(Buffer* buffer){
    bool ok = true;
    bool hasMore = true;
    while(hasMore){
        switch (state)
        {
        case REQUEST_LINE:
            const char* crlf = buffer->findCRLF();
            if(crlf){
                ok = parseRequestLine(buffer->peek(), crlf);
                if(ok){
                    state = HEADERS;
                    buffer->retrieveUntil(crlf + 2);
                }else{
                    hasMore = false;
                }
            }else{
                hasMore = false;
            }
            break;
        case HEADERS:
            const char* crlf = buffer->findCRLF();
            if(crlf){
                const char* colon = find(buffer->peek(), crlf, ':');
                if(colon != crlf){
                    parseHeaders(buffer->peek(), colon, crlf);
                }else{
                    //空行
                    state = BODY;
                    //fixme
                    hasMore = false;
                }
                buffer->retrieveUntil(crlf + 2);
            }
            break;
        case BODY:
            /* fixme */
            break;
        case FINISH:    
            return true;
        default:
            break;
        }
    }
}

bool HttpContent::parseRequestLine(const char* begin, const char* end){
    //eg:POST /path?name=value HTTP/1.1
    bool result = false;
    const char* start = begin;
    const char* space = find(start, end, ' ');
    if(!space && request.setMethod(start, space)){
        start = space + 1;
        space = find(start, end, ' ');
        if(space != end){
            const char* question = find(start, space, '?');
            if(question != space){
                request.setPath(start, question);
                request.setQuery(question, space);
            }else{
                request.setPath(start, space);
            }
            start = space + 1;
            int length = end - start;
            if(length == 8 && equal(start, end - 1, "HTTP/1.")){
                if(*(end - 1) == '1'){
                    request.setVersion(HttpRequest::kHttp11);
                    result = true;
                }else if(*(end - 1) == '0'){
                    request.setVersion(HttpRequest::kHttp10);
                    result = true;
                }
            }
        }
    }
    return result;
}

bool HttpContent::parseHeaders(const char* begin, const char* colon, const char* end){
    string key = string(begin, colon);
    //去除前导空格
    ++colon;
    while(colon < end && isspace(*colon)){
        ++colon;
    }

    string value = string(colon, end);
    //去除末尾空格
    int right = value.size() - 1;
    while(right >= 0 && isspace(value[right])){
        right--;
    }
    if(right >= 0){
        value.resize(right + 1);
    }

    request.addHeader(key, value);
}
