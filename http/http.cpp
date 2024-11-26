#include "http.h"
LINE_STATUS parseLine(Buffer *buffer, int& readIndex, int &edgeIndex){
    /*
        readIndex 指向buffer中当前正在分析的字节，
        edgeIndex 指向buffer中客户数据末尾的字节
    */
    /* 遍历当前的可读数据 */
    const char *str = buffer->peek();
    char temp;
    for(; readIndex < edgeIndex; readIndex++){
        temp = str[readIndex];
        if(temp == '\r'){
            if(readIndex + 1 == edgeIndex){
                return LINE_OPEN;
            }else if(str[readIndex + 1] == '\n'){
                readIndex += 2;
                return LINE_OK;
            }else
                return LINE_BAD;
        }else if(temp == '\n'){
            if(readIndex > 0 && str[readIndex - 1] == '\r'){
                readIndex++;
                return LINE_OK;
            }
            return LINE_BAD;
        }
    }
    return LINE_OPEN;
}

HTTP_CODE parseContent(Buffer *buffer, int& readIndex, int& edgeIndex, CHECK_STATE &checkState, int& lineStart){
    LINE_STATUS lineStatus = LINE_OK; // 记录当前行的读取状态
    HTTP_CODE retCode = NO_REQUEST; // 记录HTTP请求的处理结果
    /* 从 buffer 中取出所有完整的行 */
    while((lineStatus = parseLine(buffer, readIndex, edgeIndex)) == LINE_OK){
        int startIndex = lineStart;
        lineStart = readIndex;
        /* checkState 记录当前主机的状态 */
        switch (checkState){
            case CHECK_STATE_REQUESTLINE:
                retCode = parseRequestLine(buffer, startIndex, edgeIndex, checkState);
                if(retCode == BAD_REQUEST){
                    return BAD_REQUEST;
                }
                break;
            case CHECK_STATE_HEADER:
                retCode = parseHeaders(buffer, startIndex, readIndex, checkState);
                if(retCode == BAD_REQUEST){
                    return BAD_REQUEST;
                }
                break;
            default:
                return INTERNAL_ERROR;
        }
        edgeIndex = buffer->getWriteIndex();
    }
}

HTTP_CODE parseRequestLine(Buffer *buffer, const int startIndex, const int edgeIndex, CHECK_STATE &checkState){
    assert(checkState == CHECK_STATE_REQUESTLINE);
    string requestLine = buffer->getString(startIndex, edgeIndex);

    /* 提取请求方法 */
    size_t spacePos = requestLine.find(' ');//查找方法和URL之间的空格
    if(spacePos == string::npos){
        return BAD_REQUEST;
    }
    string method = requestLine.substr(0, spacePos);//提取方法
    if(method != "GET" && method != "POST"){
        return BAD_REQUEST;
    }
    requestLine = requestLine.substr(spacePos + 1);//移除字符串中的方法

    /* 提取URL */
    spacePos = requestLine.find(' ');//查找URL和HTTP版本之间的空格 
    if(spacePos == string::npos){
        return BAD_REQUEST;
    }
    string url = requestLine.substr(0, spacePos);//提取 URL
    if(url.empty()){
        return BAD_REQUEST;
    }
    requestLine = requestLine.substr(spacePos + 1);//移除字符串中的URL

    /* 提取 HTTP 版本号 */
    requestLine = requestLine.substr(0, requestLine.size() - 2);//去掉末尾的回车换行符
    string version = requestLine;//提取HTTP版本号
    if(version != "HTTP/1.1"){
        return BAD_REQUEST;
    }
    /* 检验URL是否合法 */
    //to do

    checkState = CHECK_STATE_HEADER;
    return NO_REQUEST;
}

HTTP_CODE parseHeaders(Buffer *buffer, const int startIndex, const int edgeIndex, CHECK_STATE &checkState){
    assert(checkState == CHECK_STATE_HEADER);
    string requestLine = buffer->getString(startIndex, edgeIndex); 
    if(requestLine == "\r\n"){
        return GET_REQUEST;
    }
    size_t colonPos = requestLine.find(':');
    if(colonPos == string::npos){
        return BAD_REQUEST;
    }       
    size_t crlfPos = requestLine.find("\r\n", colonPos);
    if (crlfPos == string::npos){
        return BAD_REQUEST;
    }
    string key = requestLine.substr(0, colonPos);
    size_t valueStart = colonPos + 1;
    while(valueStart < crlfPos && isspace(requestLine[valueStart])){
        valueStart++;
    }
    string value = requestLine.substr(valueStart, crlfPos - valueStart);
    printf("Parse Headers:\n\tthe key is : %s\n\tthe value is : %s\n", key.c_str(), value.c_str());
    return NO_REQUEST;
}