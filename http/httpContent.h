#include "httpRequest.h"
class HttpContent{
public:
    enum PARSE_STATE{
        REQUEST_LINE,
        HEADERS,
        BODY,
        FINISH
    };

    bool parseRequest(Buffer* buffer);
    bool parseRequestLine(const char* begin, const char* end);
    bool parseHeaders(const char* begin, const char* colon, const char* end);


private:
    HttpRequest request;
    PARSE_STATE state;
};