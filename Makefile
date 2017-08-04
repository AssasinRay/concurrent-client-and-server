# CS438 - spring 2016 MP1
#
# NOTE: if you decide to write your solution in C++, you will have to change the compiler 
# in this file. 


CC=g++
FLAGS=-g3
LIBS=-lpthread

all: http_client http_server
debug: CC += -DDEBUG -g
debug: FLAGS += -DDEBUG -g
debug: http_client http_server

.phony: clean 

http_client: http_client.cpp
	$(CC) $(FLAGS) http_client.cpp -o http_client $(LIBS)

http_server: http_server.cpp
	$(CC) $(FLAGS) http_server.cpp -o http_server $(LIBS)

clean: 
	rm -rf *.o http_client http_server 

# CXXFLAGS = -g3 -gdwarf2
# CCFLAGS = -g3 -gdwarf2
# LIBS=-lpthread

# all: http_client http_server
# debug: CXXFLAGS += -DDEBUG -g
# debug: CCFLAGS += -DDEBUG -g
# debug: http_client http_server

# .phony: clean 

# http_client: http_client.cpp
# 	$(CXX) http_client.cpp -o http_client $(LIBS)

# http_server: http_server.cpp
# 	$(CXX) http_server.cpp -o http_server $(LIBS)

# clean: 
# 	rm -rf *.o http_client http_server 