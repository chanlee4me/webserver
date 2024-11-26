#include "http.h"
#include "../buffer/buffer.h"
#include <iostream>

void testParseRequestLine(const std::string& requestLine, HTTP_CODE expectedCode, CHECK_STATE expectedState){
    Buffer buffer;
    buffer.append(requestLine.c_str(), requestLine.size());

    int checkedIndex = 0;
    int edgeIndex = buffer.getWriteIndex();
    CHECK_STATE checkState = CHECK_STATE_REQUESTLINE;
    int startLine = buffer.getCheapPrepend();

    HTTP_CODE result = parseRequestLine(&buffer, startLine, edgeIndex, checkState);
    if(result == expectedCode && checkState == expectedState){
        std::cout << "测试通过: " << requestLine << std::endl;
    }
    else{
        std::cout << "测试失败: " << requestLine << std::endl;
        std::cout << "预期 HTTP_CODE: " << expectedCode << ", 实际: " << result << std::endl;
        std::cout << "预期 CHECK_STATE: " << expectedState << ", 实际: " << checkState << std::endl;
    }
}

int main(){
    // 正确的 GET 请求
    testParseRequestLine("GET /index.html HTTP/1.1\r\n", NO_REQUEST, CHECK_STATE_HEADER);

    // 正确的 POST 请求
    testParseRequestLine("POST /submit HTTP/1.1\r\n", NO_REQUEST, CHECK_STATE_HEADER);

    // 无效的方法
    testParseRequestLine("PUT /index.html HTTP/1.1\r\n", BAD_REQUEST, CHECK_STATE_REQUESTLINE);

    // 缺少 HTTP 版本
    testParseRequestLine("GET /index.html\r\n", BAD_REQUEST, CHECK_STATE_REQUESTLINE);

    // 无效的 HTTP 版本
    testParseRequestLine("GET /index.html HTTP/2.0\r\n", BAD_REQUEST, CHECK_STATE_REQUESTLINE);

    // 缺少 URL
    testParseRequestLine("GET  HTTP/1.1\r\n", BAD_REQUEST, CHECK_STATE_REQUESTLINE);

    // 空字符串
    testParseRequestLine("", BAD_REQUEST, CHECK_STATE_REQUESTLINE);

    // 只有方法
    testParseRequestLine("GET\r\n", BAD_REQUEST, CHECK_STATE_REQUESTLINE);

    // 只有方法和 URL
    testParseRequestLine("GET /index.html\r\n", BAD_REQUEST, CHECK_STATE_REQUESTLINE);

    return 0;
}