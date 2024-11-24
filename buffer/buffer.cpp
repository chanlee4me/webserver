#include <stdio.h>
#include "buffer.h"

using namespace std;
Buffer::Buffer()
    : readIndex(kCheapPrepend),
      writeIndex(kCheapPrepend),
      buffer(kCheapPrepend + kInitialSize)
{
    assert(readableBytes() == 0);
    assert(writableBytes() == kInitialSize);
    assert(prependableBytes() == kCheapPrepend);
}
Buffer::~Buffer(){
}
void Buffer::append(const char* data, size_t len){
    if(writableBytes() < len){
        /* the real memory is enough*/
        if(prependableBytes() > 500){
            /* move the readable data to the front */
            size_t newStartIndex = Buffer::kCheapPrepend;
            size_t dataSize = readableBytes();
            copy(buffer.begin() + readIndex, buffer.begin() + writeIndex, buffer.begin() + newStartIndex);
            readIndex = newStartIndex;
            writeIndex = readIndex + dataSize;
        }
        else{
            /* resize the buffer */
            buffer.resize(writeIndex + len);
        }
    }
    /* insert new data */
    assert(writableBytes() >= len);
    buffer.insert(buffer.begin() + writeIndex, data, data + len);
    /* update the index */
    writeIndex += len;
}

void Buffer::prepend(const char* data, size_t len){
    if(prependableBytes() < len){
        printf("Please justify the value of kCheapPrepend\n");
        return;
    }
    /* insert new data */
    copy(data, data + len, buffer.begin() + readIndex - len);
    /* update the readIndex */
    readIndex -= len;
}

void Buffer::retrieve(size_t len){
    assert(len <= readableBytes());
    readIndex += len;
}
void Buffer::retrieveAll(){
    readIndex = kCheapPrepend;
    writeIndex = kCheapPrepend;
}

