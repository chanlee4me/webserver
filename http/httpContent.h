#include "httpRequest.h"
class HttpContent{
public:
    enum PARSE_STATE{
        REQUEST_LINE,
        HEADERS,
        BODY,
        FINISH
    };
    HttpContent():request(), state(REQUEST_LINE){}
    ~HttpContent();
    bool parseRequest(Buffer* buffer);
    bool parseRequestLine(const char* begin, const char* end);
    bool parseHeaders(const char* begin, const char* colon, const char* end);
    bool parseBody(const char* begin, const char* end);//fixme

private:
    HttpRequest request;
    PARSE_STATE state;
};