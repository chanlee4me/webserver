#ifndef HTTP_H
#define HTTP_H
#include <iostream>
#include "../buffer/buffer.h"

/* 主状态机状态: 当前正在解析请求行； 当前正在解析头部 */
enum CHECK_STATE{
    CHECK_STATE_REQUESTLINE = 0, CHECK_STATE_HEADER 
};
/* 从状态机状态 */
enum LINE_STATUS{
    LINE_OK = 0, //读取到完整的行（初始）
    LINE_BAD,   //行出错
    LINE_OPEN   //行数据尚不完整
};
/* 服务器处理 HTTP 请求的结果 */
enum HTTP_CODE{
    NO_REQUEST, //请求不完整，需要继续读取客户数据
    GET_REQUEST,//获得了一个完整的客户请求
    BAD_REQUEST,//客户请求有语法错误
    NO_RESOURCE,//服务器没有资源
    FORBIDDEN_REQUEST,//客户对资源没有足够的访问权限
    FILE_REQUEST,//文件请求
    INTERNAL_ERROR,//服务器内部错误
    CLOSED_CONNECTION//客户端已经关闭连接
};

/* 从状态机：解析出一行内容 */
LINE_STATUS parseLine(Buffer *buffer, int& checkedIndex, int& edgeIndex);
/* 主状态机：分析 HTTP 请求入口函数 */
HTTP_CODE parseContent(Buffer *buffer, int& checkedIndex, int& edgeIndex, CHECK_STATE &checkState, int& stareLine);
/* 主状态机：解析请求行 */
HTTP_CODE parseRequestLine(Buffer *buffer, const int startIndex, const int edgeIndex, CHECK_STATE &checkState);
/* 主状态机；解析消息头 */
HTTP_CODE parseHeaders(Buffer *buffer, const int startIndex, const int edgeIndex, CHECK_STATE &checkState);
#endif