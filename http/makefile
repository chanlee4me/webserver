# 指定编译器
CXX := g++

# 编译选项
CXXFLAGS := -std=c++11 -g -Wall

# 可执行文件
TARGET := testParseHeaders

# 源文件
SRCS := http.cpp testParseHeaders.cpp

# 头文件目录
INCLUDES := -I. -I../buffer

# 依赖的源文件
DEPS := ../buffer/buffer.cpp

# 编译规则
$(TARGET): $(SRCS) $(DEPS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(TARGET) $(SRCS) $(DEPS)

# 清理
clean:
	rm -f $(TARGET)