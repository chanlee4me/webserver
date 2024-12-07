#include "eventloop.h"

using namespace tiny_muduo;
namespace{
    __thread EventLoop* t_loopInThisThread = nullptr;
}

EventLoop::EventLoop()
    : threadId(current_thread::tid()),
      looping(false),
      quit(false){
    if(t_loopInThisThread){
        // LOG_FATAL << "Another EventLoop " << t_loopInThisThread
        //           << " exists in this thread " << threadId;
    }else{
        t_loopInThisThread = this;
    }
}

EventLoop::~EventLoop(){
    assert(!looping);
    t_loopInThisThread = nullptr;
}
EventLoop* EventLoop::getEventLoopOfCurrnetThread(){
    return t_loopInThisThread;
}

void EventLoop::loop(){
    assert(!looping);
    assertInLoopThread();
    looping = true;
    while(looping){
        //FIXME
    }
    looping = false;
}