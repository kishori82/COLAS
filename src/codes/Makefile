CC=gcc
CPP=g++

CPPFLAGS= -std=c++11 -I/home/kishori/shared_kodocpp/include
CFLAGS= -lm -lzmq  -lczmq -Wimplicit-function-declaration -fpic #-Wall 
LDFLAGS=-L/home/kishori/shared_kodocpp/ -Wl,-Bdynamic -lkodoc -Wl,-rpath /home/kishori/shared_kodocpp 

DEPS=reed_solomon.h  
SRCS=reed_solomon.cpp  rlnc_example.cpp reed_solomon_example.cpp rlnc_rs_check.c

OBJSC=$(SRCS:%o=%cpp)

TARGET=libreed.so rlnc_example reed_solomon_example librlnc_rs.so rlnc_rs_check
all: $(TARGET) 

%.o:%.cpp
	$(CPP) -c $< -o $@ -DASLIBRARY $(CPPFLAGS)

rlnc_example: rlnc_example.o
	$(CPP)   -fPIC $<  -o $@  -DASLIBRARY  $(CPPFLAGS) $(LDFLAGS) 


libreed.so: $(SRCS) $(DEPS) $(OBJS)
	$(CPP) -fPIC -shared reed_solomon.cpp  -o libreed.so  -DASLIBRARY  $(CPPFLAGS) $(LDFLAGS) 

reed_solomon_example: reed_solomon_example.cpp
	$(CPP) -fPIC reed_solomon_example.cpp  -o $@   $(CPPFLAGS) $(LDFLAGS) 

librlnc_rs.so: rlnc_rs.cpp
	$(CPP) -fPIC -shared -DASLIBRARY $<  -o $@   $(CPPFLAGS) $(LDFLAGS) 

rlnc_rs_check: rlnc_rs_check.c
	$(CC) -fPIC  $<  -o $@   $(LDFLAGS)  -L. -lrlnc_rs

clean:
	rm -f abd $(TARGET)  *.o lib* *.so
