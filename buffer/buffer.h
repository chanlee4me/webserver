#ifndef BUFFER_H
#define BUFFER_H

#include <vector>
#include <string>
#include <cassert>
using namespace std;

// Buffer class for efficient I/O operations
class Buffer
{
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

    const string getString(size_t start, size_t edge) const{
        assert(edge - start <= readableBytes());
        return string(&(*buffer.begin()) + start, edge - start);
    }
    // Append data to the buffer
    void append(const char* data, size_t len);

    void append(const string& str){
        append(str.c_str(), str.size());
    }
    // Prepend data to the buffer
    void prepend(const char* data, size_t len);
    // Retrieve data from the buffer
    void retrieve(size_t len);
    // Retrieve all data from the buffer
    void retrieveAll();

    void retrieveUntil(const char* end){
        assert(peek() <= end);
        assert(end <= beginWrite());
        retrieve(end - peek());
    }

    // Get a pointer to the readable data
    const char* peek() const{
        return &(*buffer.begin()) + readIndex;
    }
    
    const char* begin() const{
        return &(*(buffer.begin()));
    }

    const char* beginWrite() const{
        return begin() + writeIndex;
    }
    // Find the CRLF in the buffer
    const char* findCRLF() const{
        const char* crlf = search(peek(), beginWrite(), kCRLF, kCRLF + 2);//kCRLF数组一共有 3 字节，前两字节是需要的字符，最后一个字节是结束符
        return crlf == beginWrite() ? NULL : crlf;
    }
private:
    static const int kCheapPrepend = 8; // Extra space at the front
    static const size_t kInitialSize = 1024; // Initial buffer size
    static const char kCRLF[];

    int readIndex;   // Index of the next readable byte
    int writeIndex;  // Index of the next writable byte
    vector<char> buffer; // Buffer storage
};

#endif