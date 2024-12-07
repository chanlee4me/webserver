#ifndef UTIL_THREAD_H
#define UTIL_THREAD_H


#include "thread.h"


void current_thread::cacheTid(){
    if(t_cachedTid == 0){
        t_cachedTid = gettid();
        int n = snprintf(t_tidString, sizeof(t_tidString), "%5d ", t_cachedTid);
    }
}

#endif