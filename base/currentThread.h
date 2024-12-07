#ifndef BASE_CURRENT_THREAD_H
#define BASE_CURRENT_THREAD_H

namespace tiny_muduo{

namespace current_thread{
/*
    __thread 关键字表示这些变量是线程局部存储的，每个线程都有自己独立的变量副本。
    __thread 它是一个编译器扩展，不需要引入特定的头文件
*/
    extern __thread int t_cachedTid;// 用于缓存当前线程的 ID
    extern __thread char t_tidString[32];// 存储线程 ID 的字符串表示
    extern __thread const char* t_threadName;// 指向线程名称的指针

    void cacheTid();// 用于缓存当前线程的 ID

    // 返回线程 ID
    inline int tid(){
        /*
        __builtin_expect 是 GCC 提供的一个内建函数，用于提示编译器某个条件的可能性，以便进行优化。
        在这里，它提示编译器 t_cachedTid == 0 的可能性较低。
        */
        if(__builtin_expect(t_cachedTid == 0, 0)){
            cacheTid();
        }
        return t_cachedTid;
    }

    // 返回线程 ID 的字符串表示
    inline const char* tidString(){
        return t_tidString;
    }

    // 返回线程名称
    inline const char* name(){
        return t_threadName;
    }
}   //namespace CurrentThread

}   //namespace tiny_muduo