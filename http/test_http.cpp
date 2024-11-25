#include "http.h"
#include <cassert>
#include <iostream>

void testParseLine()
{
    {
        // Test case 1: Normal line ending with \r\n
        Buffer buffer;
        buffer.append("GET / HTTP/1.1\r\n");
        assert(parseLine(&buffer) == LINE_OK);
        assert(buffer.readableBytes() == 0);
    }

    {
        // Test case 2: Line ending with \n only
        Buffer buffer;
        buffer.append("GET / HTTP/1.1\n");
        assert(parseLine(&buffer) == LINE_BAD);
    }

    {
        // Test case 3: Line ending with \r only
        Buffer buffer;
        buffer.append("GET / HTTP/1.1\r");
        assert(parseLine(&buffer) == LINE_OPEN);
    }

    {
        // Test case 4: Incomplete line
        Buffer buffer;
        buffer.append("GET / HTTP/1.1");
        assert(parseLine(&buffer) == LINE_OPEN);
    }

    {
        // Test case 5: Empty buffer
        Buffer buffer;
        assert(parseLine(&buffer) == LINE_OPEN);
    }

    {
        // Test case 6: Line with \r\n in the middle
        Buffer buffer;
        buffer.append("GET / HTTP/1.1\r\nHost: example.com\r\n");
        assert(parseLine(&buffer) == LINE_OK);
        assert(buffer.readableBytes() == 17); // Remaining "Host: example.com\r\n"
    }

    std::cout << "All tests passed!" << std::endl;
}

int main()
{
    testParseLine();
    return 0;
}