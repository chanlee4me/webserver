#ifndef BUFFER_H
#define BUFFER_H

#include <vector>
#include <cassert>
using namespace std;

// Buffer class for efficient I/O operations
class Buffer
{
private:
    static const int kCheapPrepend = 8; // Extra space at the front
    static const size_t kInitialSize = 1024; // Initial buffer size

    int readIndex;   // Index of the next readable byte
    int writeIndex;  // Index of the next writable byte
    vector<char> buffer; // Buffer storage

public:
    Buffer();
    ~Buffer();

    // Get kCheapPrepend
    static const int getCheapPrepend(){ return kCheapPrepend; }
    // Get kInitialSize
    static const int getInitialSize() { return kInitialSize; }

    // Returns the number of bytes that can be read
    inline size_t readableBytes() const{
        return writeIndex - readIndex;
    }
    // Returns the number of bytes that can be written
    inline size_t writableBytes() const{
        return buffer.size() - writeIndex;
    }
    // Returns the number of bytes before the readable area
    inline size_t prependableBytes() const{
        return readIndex;
    }
    size_t getReadIndex() const{
        return readIndex;
    }
    size_t getWriteIndex() const{
        return writeIndex;
    }
    // Append data to the buffer
    void append(const char* data, size_t len);
    // Prepend data to the buffer
    void prepend(const char* data, size_t len);
    // Retrieve data from the buffer
    void retrieve(size_t len);
    // Retrieve all data from the buffer
    void retrieveAll();
    // Get a pointer to the readable data
    const char* peek() const{
        return &(*buffer.begin()) + readIndex;
    }
};

#endif