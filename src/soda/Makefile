CC=gcc -g -DDEBUG

UTILS=../utilities/

LDFLAGS=-L../codes  -lm -L../ZMQ/czmqlibs -L../ZMQ/zmqlibs -lzmq -lczmq  -lrlnc_rs  -Wl,-rpath=../codes
CIFLAGS=-I../utilities -I../codes -I../ZMQ/include #--Wall 
CFLAGS= -Wimplicit-function-declaration 

DEPS=soda_reader.h $(UTILS)/base64.h $(UTILS)/algo_utils.h $(UTILS)/server.h $(UTILS)/client.h
SRCS=soda_reader.c $(UTILS)/base64.c $(UTILS)/algo_utils.c $(UTILS)/server.c $(UTILS)/client.c
OBJS=$(SRCS:%c=%o)

all: libsoda.so  

%.o: %.c 
	$(CC)  -fPIC -c -o $@  $< -DASLIBRARY $(CFLAGS) $(CIFLAGS) 


libsoda.so: $(OBJS)
	$(CC) -fPIC -shared -o $@ $(OBJS) $(CIFLAGS)  $(CFLAGS) $(LDFLAGS) 

clean:
	rm -f abd libsoda.so  *.o lib* *.so  $(OBJS)  ../utilities/*.o
