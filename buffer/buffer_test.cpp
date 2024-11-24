#include <iostream>
#include <cassert>
#include <cstring>
#include "buffer.h"

void testRetrieve() {
    Buffer buffer;
    const char* data = "Hello, World!";
    size_t dataLen = strlen(data);

    buffer.append(data, dataLen);
    assert(buffer.readableBytes() == dataLen);

    buffer.retrieve(5);
    assert(buffer.readableBytes() == dataLen - 5);

    buffer.retrieve(dataLen - 5);
    assert(buffer.readableBytes() == 0);
}

void testRetrieveAll() {
    Buffer buffer;
    const char* data = "Hello, World!";
    size_t dataLen = strlen(data);

    buffer.append(data, dataLen);
    assert(buffer.readableBytes() == dataLen);

    buffer.retrieveAll();
    assert(buffer.readableBytes() == 0);
    assert(buffer.prependableBytes() == buffer.getCheapPrepend());
}

int main() {
    testRetrieve();
    testRetrieveAll();
    std::cout << "All tests passed!" << std::endl;
    return 0;
}