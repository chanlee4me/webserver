CXX = g++
CXXFLAGS = -Wall -g

TARGET = test_http
SRCS = http.cpp test_http.cpp ../buffer/buffer.cpp 
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean