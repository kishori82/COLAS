CC=gcc -g -DDEBUG



UTILS=../utilities
CIFLAGS= -I$(UTILS) -I../ZMQ/include  -I../utilities/
CFLAGS= -lm -L../ZMQ/zmqlibs -lzmq  -L../ZMQ/czmqlibs/lib -lczmq -Wimplicit-function-declaration -fPIC -Wall 

#SOURCES: hwserver.c asyncsrv.c
UT_DEPS=$(UTILS)/algo_utils.h $(UTILS)/base64.h  $(UTILS)/client.h $(UTILS)/server.h
UT_SRCS=$(UTILS)/algo_utils.c $(UTILS)/base64.c  $(UTILS)/client.c $(UTILS)/server.c

UT_OBJS=$(UT_SRCS:%c=%o)


ABD_DEPS=abd_client.h abd_server.h abd_reader.h abd_writer.h
ABD_SRCS=abd_client.c abd_server.c abd_reader.c abd_writer.c

DEPS=$(UTILS)/base64.h $(UTILS)/algo_utils.h $(UTILS)/server.h $(UTILS)/client.h $(ABD_DEPS)
SRCS=$(UTILS)/base64.c $(UTILS)/algo_utils.c $(UTILS)/server.c $(UTILS)/client.c $(ABD_SRCS) 
OBJS=$(SRCS:%c=%o)


all: libabd.so

%.o: %.c 
	$(CC) -fPIC -c -o $@  $< -DASLIBRARY  $(CFLAGS)  $(CIFLAGS)

#asyncsrv: asyncsrv.o

abd_client:  $(OBJS) $(DEPS)  
	$(CC) -DASLIBRARY -DASMAIN -o $@   algo_utils.o base64.o $<  $(CFLAGS)

abd_server:  $(OBJS) $(DEPS)
	$(CC) -DASLIBRARY -DASMAIN -o $@  algo_utils.o base64.o $? $(CFLAGS)

static: abd_client.c abd_server.c abd_reader.c abd_writer.c
	$(CC) -static -c  -o abd_client.o  abd_client.c  -DASLIBRARY #$(CFLAGS)
	$(CC) -static -c  -o abd_server.o  abd_server.c  -DASLIBRARY #$(CFLAGS)
	ar -cvq libabd.a  abd_server.o abd_client.o

libabd.so: $(OBJS)
	$(CC) -shared -fPIC -o $@  $?  $(CFLAGS)

clean:
	rm -f abd abd_client abd_server $(OBJS)  *.o  *.so
