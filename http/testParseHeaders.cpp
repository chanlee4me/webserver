
#include "http.h"
#include "../buffer/buffer.h"
#include <cassert>
#include <cstring>
#include <iostream>

int main() {
    int startLine = 0;
    // 测试用例 1: 有效的头部
    {   
        Buffer buffer;
        CHECK_STATE state = CHECK_STATE_HEADER;
        int edgeIndex = 0;
        startLine = buffer.getCheapPrepend();;
        const char* header = "Host: example.com\r\nUser-Agent: test\r\n\r\n";
        buffer.append(header, strlen(header));
        edgeIndex = buffer.getWriteIndex();
        HTTP_CODE result = parseHeaders(&buffer, startLine, edgeIndex, state);
        assert(result == GET_REQUEST);
        std::cout << "测试用例 1 通过。" << std::endl;

    }


    // 测试用例 2: 无效的头部（缺少冒号）
    {

        Buffer buffer;
        CHECK_STATE state = CHECK_STATE_HEADER;
        int edgeIndex = 0;
        startLine = buffer.getCheapPrepend();;
        const char* header = "Host example.com\r\n\r\n";
        buffer.append(header, strlen(header));
        edgeIndex = buffer.getWriteIndex();
        HTTP_CODE result = parseHeaders(&buffer, startLine, edgeIndex, state);
        assert(result == BAD_REQUEST);
        std::cout << "测试用例 2 通过。" << std::endl;
    }

    // 测试用例 3: 空头部
    {
        CHECK_STATE state = CHECK_STATE_HEADER;
        int edgeIndex = 0;
        Buffer buffer;
        startLine = buffer.getCheapPrepend();
        const char* header = "\r\n";
        buffer.append(header, strlen(header));
        edgeIndex = buffer.getWriteIndex();
        HTTP_CODE result = parseHeaders(&buffer, startLine, edgeIndex, state);
        assert(result == GET_REQUEST);
        std::cout << "测试用例 3 通过。" << std::endl;

    }
    // 更多测试用例可以在这里添加

    return 0;
}