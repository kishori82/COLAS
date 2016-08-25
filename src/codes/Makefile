CC=gcc
CPP=g++

CPPFLAGS= -std=c++11 -I/home/docker/shared_kodocpp/include
CFLAGS= -lm -lzmq  -lczmq -Wimplicit-function-declaration -fpic #-Wall 
LDFLAGS=-L/home/docker/shared_kodocpp/ -Wl,-Bdynamic -lkodoc -Wl,-rpath /home/docker/shared_kodocpp 

DEPS=reed_solomon.h  
SRCS=reed_solomon.cpp 

OBJSCPP=$(SRCS:%o=%cpp)
OBJSC=$(SRCS:%o=%cpp)

all: libreed.so  


%.o: %.cpp
	$(CCP) -c -o $@  $< -DASLIBRARY $(CPPFLAGS) $(LDFLAGS)

libreed.so: $(SRCS) $(DEPS) $(OBJS)
	$(CPP) -fPIC -shared reed_solomon.cpp  -o libreed.so  -DASLIBRARY  $(CPPFLAGS) $(LDFLAGS) 

clean:
	rm -f abd libreed.so  *.o lib* *.so
