CC=gcc

UTILS=../utilities/C

LDFLAGS=-L../codes  -lm -lzmq -lczmq  -lreed  -Wl,-rpath=../codes
CIFLAGS=-I../codes -I../utilities/C -I../codes #-Wall 
CFLAGS= -Wimplicit-function-declaration 

DEPS=sodaw_client.h $(UTILS)/base64.h $(UTILS)/algo_utils.h
SRCS=sodaw_client.c $(UTILS)/base64.c $(UTILS)/algo_utils.c

OBJS=$(SRCS:%c=%o)

all: libsodaw.so  

%.o: %.c 
	$(CC)  -fPIC -c -o $@  $< -DASLIBRARY $(CFLAGS) $(CIFLAGS) 


libsodaw.so: $(OBJS)
	$(CC) -fPIC -shared -o $@ $(OBJS)  $(CFLAGS) $(LDFLAGS) 

clean:
	rm -f abd libsodaw.so  *.o lib* *.so  $(OBJS)
