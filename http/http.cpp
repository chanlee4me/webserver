#include "http.h"

/* 从状态机：解析出一行内容 */
LINE_STATUS parseLine(Buffer *buffer)
{
    /*
        buffer的 readIndex 指向当前正在分析的字节，
        buffer的 writeIndex 指向客户数据末尾的字节
    */
    /* 遍历当前的可读数据 */
    int curIndex = 0;
    int end = buffer->readableBytes();
    const char* cur = buffer->peek();
    for(; curIndex < end; curIndex++){
        if(cur[curIndex] == '\r'){ //'\r'是回车符
            if((curIndex + 1) == end){
                return LINE_OPEN;
            }else if(cur[curIndex + 1] == '\n'){ //'\n'是换行符
                buffer->retrieve(curIndex + 2);
                return LINE_OK;
            }else 
                return LINE_BAD;
        }else if(cur[curIndex] == '\n'){ //'\n'是换行符
            if((curIndex > 0) && (cur[curIndex - 1] == '\r')){ //'\r'是回车符
                buffer->retrieve(curIndex + 1);
                return LINE_OK;
            }else return LINE_BAD;
        }
    }
    return LINE_OPEN;
}