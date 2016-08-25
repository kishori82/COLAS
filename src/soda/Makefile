CC=gcc

LDFLAGS=-L../codes  -lm -lzmq -lczmq  -lreed  -Wl,-rpath=../codes
CIFLAGS=-I../codes -I../utilities/C -I../codes #-Wall 
CFLAGS= -Wimplicit-function-declaration 

DEPS=soda_client.h ../utilities/C/base64.h ../utilities/C/algo_utils.h
SRCS=soda_client.c ../utilities/C/base64.c ../utilities/C/algo_utils.c

OBJS=$(SRCS:%c=%o)

all: libsoda.so  

%.o: %.c 
	$(CC)  -fPIC -c -o $@  $< -DASLIBRARY $(CFLAGS) $(CIFLAGS) 


libsoda.so: $(OBJS)
	$(CC) -fPIC -shared -o $@ $(OBJS)  $(CFLAGS) $(LDFLAGS) 

clean:
	rm -f abd libsoda.so  *.o lib* *.so  $(OBJS)
