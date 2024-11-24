#include <iostream>
#include <cassert>
#include "buffer.h"

void testAppend() {
    Buffer buffer;
    const char* data1 = "Hello";
    buffer.append(data1, 5);
    assert(buffer.readableBytes() == 5);
    assert(std::string(buffer.peek(), buffer.readableBytes()) == "Hello");

    const char* data2 = " World";
    buffer.append(data2, 6);
    assert(buffer.readableBytes() == 11);
    assert(std::string(buffer.peek(), buffer.readableBytes()) == "Hello World");
}

void testPrepend() {
    Buffer buffer;
    const char* data1 = "World";
    buffer.append(data1, 5);
    assert(buffer.readableBytes() == 5);
    assert(std::string(buffer.peek(), buffer.readableBytes()) == "World");

    const char* data2 = "Hello ";
    buffer.prepend(data2, 6);
    assert(buffer.readableBytes() == 11);
    assert(std::string(buffer.peek(), buffer.readableBytes()) == "Hello World");
}

int main() {
    testAppend();
    testPrepend();
    std::cout << "All tests passed!" << std::endl;
    return 0;
}