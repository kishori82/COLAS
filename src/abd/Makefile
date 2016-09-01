CC=gcc

UTILS=../utilities/C
CIFLAGS= -I$(UTILS) 
CFLAGS= -lm -lzmq  -lczmq -Wimplicit-function-declaration -fPIC #-Wall 

#SOURCES: hwserver.c asyncsrv.c
UT_DEPS=$(UTILS)/algo_utils.h $(UTILS)/base64.h  $(UTILS)/client.h $(UTILS)/server.h
UT_SRCS=$(UTILS)/algo_utils.c $(UTILS)/base64.c  $(UTILS)/client.c $(UTILS)/server.c


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

libabd.so: abd_client.o abd_server.o  $(UT_OBJS) 
	$(CC) -shared -fPIC -o $@  $?

clean:
	rm -f abd abd_client abd_server $(UT_OBJS)  *.o  *.so
