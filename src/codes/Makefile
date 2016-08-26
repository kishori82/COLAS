CC=gcc
CPP=g++

CPPFLAGS= -std=c++11 -I/home/sgeadmin/shared_kodocpp/include
CFLAGS= -lm -lzmq  -lczmq -Wimplicit-function-declaration -fpic #-Wall 
LDFLAGS=-L/home/sgeadmin/shared_kodocpp/ -Wl,-Bdynamic -lkodoc -Wl,-rpath /home/sgeadmin/shared_kodocpp 

DEPS=reed_solomon.h  
SRCS=reed_solomon.cpp  example.cpp

OBJSC=$(SRCS:%o=%cpp)

TARGET=libreed.so example 
all: $(TARGET) 

%.o:%.cpp
	$(CPP) -c $< -o $@ -DASLIBRARY $(CPPFLAGS)

example: $(OBJS)
	$(CPP)   -fPIC $<  -o $@  -DASLIBRARY  $(CPPFLAGS) $(LDFLAGS) 


libreed.so: $(SRCS) $(DEPS) $(OBJS)
	$(CPP) -fPIC -shared reed_solomon.cpp  -o libreed.so  -DASLIBRARY  $(CPPFLAGS) $(LDFLAGS) 

clean:
	rm -f abd $(TARGET)  *.o lib* *.so
