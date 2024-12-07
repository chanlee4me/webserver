#ifndef TINY_MUDUO_BASE_NONCOPYABLE_H
#define TINY_MUDUO_BASE_NONCOPYABLE_H

namespace tiny_muduo{
/* 类的设计目的是防止其派生类的对象被复制 */
class NonCopyable{
    /* 删除拷贝构造函数和拷贝赋值运算符来禁止复制操作： */
public:
    NonCopyable(const NonCopyable&) = delete;
    void operator=(const NonCopyable&) = delete;
protected:
    /*
        通过将构造函数和析构函数声明为protected，允许派生类访问它们，
        但仍然禁止外部代码直接实例化或销毁对象。
    */
    NonCopyable() = default;
    ~NonCopyable() = default;
};

}   //namespace tiny_muduo

#endif 