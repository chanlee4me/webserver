#include <iostream>
#include <thread>
#include <chrono>
#include "server.h"

void runServer() {
    server myServer;
    myServer.start();
}

int main() {
    // 启动服务器线程
    std::thread serverThread(runServer);

    // 等待服务器启动
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // 创建客户端套接字并连接到服务器
    int clientFd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientFd == -1) {
        std::cerr << "客户端 socket 创建失败: " << strerror(errno) << std::endl;
        return -1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (connect(clientFd, (sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr << "客户端连接失败: " << strerror(errno) << std::endl;
        close(clientFd);
        return -1;
    }

    // 发送测试消息
    const char* testMessage = "Hello, server!";
    if (send(clientFd, testMessage, strlen(testMessage), 0) == -1) {
        std::cerr << "消息发送失败: " << strerror(errno) << std::endl;
        close(clientFd);
        return -1;
    }

    // 关闭客户端套接字
    close(clientFd);

    // 等待服务器线程结束
    serverThread.join();

    return 0;
}