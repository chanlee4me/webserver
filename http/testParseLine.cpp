#include "http.h"
#include "../buffer/buffer.h"
#include <iostream>
#include <cstring>

int main() {
    Buffer buffer;
    const char* testStr1 = "GET /index.html HTTP/1.1\r\n";
    const char* testStr2 = "GET /about.html HTTP/1.1\r";
    const char* testStr3 = "POST /submit HTTP/1.1\n";
    const char* testStr4 = "HEAD / HTTP/1.1\r\n\r";
    const char* testStr5 = "OPTIONS * HTTP/1.1\r\nExtraData";
    const char* testStr6 = "\r\n";
    const char* testStr7 = "";
    struct TestCase {
        const char* data;
        const char* description;
        LINE_STATUS expected;
    } testCases[] = {
        { testStr1, "正常的请求行，结尾为\\r\\n", LINE_OK },
        { testStr2, "仅有\\r结尾的请求行", LINE_OPEN },
        { testStr3, "仅有\\n结尾的请求行", LINE_BAD },
        { testStr4, "多余\\r的请求行", LINE_OK },
        { testStr5, "请求行后有多余数据", LINE_OK },
        { testStr6, "仅有\\r\\n的空行", LINE_OK },
        { testStr7, "空字符串", LINE_OPEN },
    };
    for (const auto& testCase : testCases) {
        buffer.retrieveAll();
        buffer.append(testCase.data, strlen(testCase.data));
        int checkedIndex = 0;
        int readIndex = buffer.readableBytes();
        LINE_STATUS result = parseLine(&buffer, checkedIndex, readIndex);
        std::cout << "测试用例: " << testCase.description << std::endl;
        std::cout << "预期结果: " << testCase.expected << ", 实际结果: " << result << std::endl;
        if(result == testCase.expected) {
            std::cout << "测试通过" << std::endl;
        } else {
            std::cout << "测试失败" << std::endl;
        }
        std::cout << "解析后 checkedIndex 值: " << checkedIndex << std::endl << std::endl;
    }
    return 0;
}