CC=gcc
CFLAGS= -lm -lzmq  -lczmq -Wimplicit-function-declaration -fpic #-Wall 

#SOURCES: hwserver.c asyncsrv.c
DEPS=algo_utils.h

all: abd_client abd_server  

%.o: %.c 
	$(CC) -c -o $@  $< $(CFLAGS)


#asyncsrv: asyncsrv.o

abd_client: abd_client.c   algo_utils.o  $(DEPS)
	$(CC) -DASLIBRARY -DASMAIN -o $@ $<  algo_utils.o  $(CFLAGS)


abd_server: abd_server.c algo_utils.o  $(DEPS)
	$(CC) -DASLIBRARY -DASMAIN -o $@ $< algo_utils.o  $(CFLAGS)

static: abd_client.c abd_server.c
	$(CC) -static -c  -o abd_client.o  abd_client.c  -DASLIBRARY #$(CFLAGS)
	$(CC) -static -c  -o abd_server.o  abd_server.c  -DASLIBRARY #$(CFLAGS)
	#ar rcs libabd.a  abd_server.o abd_client.o
	ar -cvq libabd.a  abd_server.o abd_client.o

libabd_shared.so: abd_client.c abd_server.c  algo_utils.o
	$(CC) -c -fPIC -o abd_client.o  abd_client.c  -DASLIBRARY 
	$(CC) -c -fPIC -o abd_server.o  abd_server.c  -DASLIBRARY 
	$(CC) -shared -fPIC -o libabd_shared.so  abd_client.o  abd_server.o   algo_utils.o

abd_client_shared: abd_client.c  $(DEPS)  libabd_shared.so
	$(CC) -DASMAIN -o abd_client abd_client.c -L. -labd_shared $(CFLAGS) 

abd_server_shared: abd_server.c  $(DEPS) libabd_shared.so
	$(CC) -DASMAIN -o abd_server abd_server.c -L. -labd_shared $(CFLAGS) 

clean:
	rm -f abd abd_client abd_server  *.o lib* *.so

