src_server = $(wildcard server/*.cpp)
src_local = $(wildcard *.cpp)
src = $(src_server) $(src_local)

obj_server = $(patsubst server/%.cpp, server/%.o, $(src_server))
obj_local = $(patsubst %.cpp, %.o, $(src_local))

obj = $(obj_server) $(obj_local)

ALL: webserver

webserver: $(obj)
	g++ -g -o $@ $^

%.o: %.cpp
	g++ -g -c $< -o $@

server/%.o: server/%.cpp
	g++ -g -c $< -o $@

clean:
	-rm -rf $(obj) webserver

.PHONY: clean ALL