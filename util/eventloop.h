#ifndef UTIL_EVENTLOOP
#define UTIL_EVENTLOOP
#include <cassert>
#include <sys/types.h>

#include "../base/noncopyable.h"
#include "../base/currentThread.h"
namespace tiny_muduo{
class EventLoop : NonCopyable{
public:
    EventLoop();
    ~EventLoop();

    void loop();
    void assertInLoopThread(){//断言是否在IO线程中
        if(!isInLoopThread()){
            abortNotInLoopThread();//不在IO线程中则终止程序
        }
    }
    bool isInLoopThread() const { return threadId == current_thread::tid(); }//判断是否在IO线程中
    
    static EventLoop* getEventLoopOfCurrnetThread();
private:
    void abortNotInLoopThread();//不在IO线程中则终止程序
    const pid_t threadId;//IO线程的id
    bool looping;//是否在循环中
    bool quit;//是否退出

};
}   //namespace tiny_muduo
#endif