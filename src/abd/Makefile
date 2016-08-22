CC=gcc
CFLAGS=-lm -lzmq  -lczmq -Wimplicit-function-declaration 

#SOURCES: hwserver.c asyncsrv.c
DEPS=algo_utils.h

%.o: %.c 
	$(CC) -c -o $@  $< $(CFLAGS)

all: abd abd_client abd_server 
#hwserver: hwserver.o
#	$(CC) -o hwserver $< $(CFLAGS)

#asyncsrv: asyncsrv.o
abd: abd.o
	$(CC) -o $@ $< $(CFLAGS)

abd_client: abd_client.o  $(DEPS)
	$(CC) -o $@ $< $(CFLAGS)


abd_server: abd_server.o  $(DEPS)
	$(CC) -o $@ $< $(CFLAGS)

clean:
	rm -f abd abd_client abd_server  *.o

