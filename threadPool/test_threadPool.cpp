#include <iostream>
#include <unistd.h> // for sleep()
#include "threadPool.h"
void* task1(void* arg) {
    int* num = static_cast<int*>(arg);
    std::cout << "Task 1: " << *num << std::endl;
    return nullptr;
}

void* task2(void* arg) {
    int* num = static_cast<int*>(arg);
    std::cout << "Task 2: " << *num << std::endl;
    return nullptr;
}

int main() {
    ThreadPool threadPool;
    threadPool.addTask(task1, new int(1));
    threadPool.addTask(task2, new int(2));
    sleep(2); // Wait for tasks to complete
    threadPool.destoryThreadPool();
    return 0;
}