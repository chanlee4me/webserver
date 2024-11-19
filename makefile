src_dirs = server threadPool .
exclude_dirs = test
exclude_files = threadPool/test_threadPool.cpp

# 获取所有源文件
src = $(foreach dir, $(src_dirs), $(wildcard $(dir)/*.cpp))

# 排除指定的目录和文件
src := $(filter-out $(foreach dir, $(exclude_dirs), $(wildcard $(dir)/*.cpp)), $(src))
src := $(filter-out $(exclude_files), $(src))

obj = $(patsubst %.cpp, %.o, $(src))

obj = $(patsubst %.cpp, %.o, $(src))

ALL: webserver

webserver: $(obj)
	g++ -g -o $@ $^

%.o: %.cpp
	g++ -g -c $< -o $@

clean:
	-rm -rf $(obj) webserver

.PHONY: clean ALL





