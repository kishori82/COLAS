ALL = abdprocess 
CC=gcc -g  -DDEBUG

CFLAGS= -Iutilities/ -Iabd/ -Icodes/ -Isodaw  -IZMQ/include
LDFLAGS= -lm -LZMQ/zmqlibs  -LZMQ/czmqlibs  -lzmq -lczmq  -Labd/ -Lcodes/ -Lsodaw -lsodaw -lrlnc_rs -labd -Wimplicit-function-declaration -fPIC -Wall 


all: $(ALL)

TARGET= abdprocess

SRCS = daemons/params.go daemons/logger.go daemons/controllerRoutes.go daemons/getParamRoutes.go  daemons/setParamRoutes.go daemons/httpServer.go daemons/routes.go daemons/controller.go daemons/utils.go daemons/writer.go daemons/reader.go daemons/server.go daemons/utilities.go

CSRCS = abd/abd_client.c abd/abd_server.c abd/abd_client.h abd/abd_server.h  abd/algo_utils.h abd/algo_utils.c

abdprocess: $(TARGET) $(SRCS)  #$(CSRCS)
	go fmt $(SRCS)
	go build -x -o abdprocess $(TARGET)

all=CODES ABD SODAW $(TARGET)

ABD:
	@echo "Building ABD"
	make -C abd clean
	make -C abd

SODAW:
	@echo "Building ABD"
	make -C sodaw clean
	make -C sodaw

CODES:
	@echo "Building ABD"
	make -C codes clean
	make -C codes

rebuildc:
	make -C codes clean
	make -C abd clean
	make -C sodaw clean
	rm -rf  abdprocessc
	make -C codes 
	make -C abd 
	make -C sodaw
	$(CC) $(CFLAGS)  abdprocess.c  -o abdprocessc  $(LDFLAGS)



rebuild:
	make -C codes clean
	make -C abd clean
	make -C sodaw clean
	rm -rf  $(TARGET)
	make -C codes 
	make -C abd 
	make -C sodaw
	go build -x -o $(TARGET) abdprocess.go

clean:
	rm -rf logs $(TARGET) abdprocessc
