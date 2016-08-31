CC=gcc


CIFLAGS= -I../utilities/C 
CFLAGS= -lm -lzmq  -lczmq -Wimplicit-function-declaration -fPIC #-Wall 

#SOURCES: hwserver.c asyncsrv.c
UT_DEPS=../utilities/C/algo_utils.h ../utilities/C/base64.h
UT_SRCS=../utilities/C/algo_utils.c ../utilities/C/base64.c


UT_OBJS=$(UT_SRCS:%c=%o)

all: libabd.so

%.o: %.c 
	$(CC) -fPIC -c -o $@  $< -DASLIBRARY  $(CFLAGS)  $(CIFLAGS)

#asyncsrv: asyncsrv.o

abd_client: abd_client.c  $(UT_OBJS) $(DEPS)
	$(CC) -DASLIBRARY -DASMAIN -o $@ $<  algo_utils.o base64.o  $(CFLAGS)

abd_server: abd_server.c $(UT_OBJS) $(DEPS)
	$(CC) -DASLIBRARY -DASMAIN -o $@ $? algo_utils.o base64.o  $(CFLAGS)

static: abd_client.c abd_server.c
	$(CC) -static -c  -o abd_client.o  abd_client.c  -DASLIBRARY #$(CFLAGS)
	$(CC) -static -c  -o abd_server.o  abd_server.c  -DASLIBRARY #$(CFLAGS)
	ar -cvq libabd.a  abd_server.o abd_client.o

libabd.so: abd_client.o abd_server.o client.o  server.o  $(UT_OBJS) 
	$(CC) -shared -fPIC -o $@  $?

clean:
	rm -f abd abd_client abd_server $(UT_OBJS)  *.o  *.so
