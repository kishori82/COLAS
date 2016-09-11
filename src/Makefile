ALL = abdprocess 

all: $(ALL)

TARGET= abdprocess.go

SRCS = daemons/params.go daemons/logger.go daemons/controllerRoutes.go daemons/getParamRoutes.go  daemons/setParamRoutes.go daemons/httpServer.go daemons/routes.go daemons/controller.go daemons/utils.go daemons/writer.go daemons/reader.go daemons/server.go utilities/GO/utilities.go

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


rebuild:
	make -C codes clean
	make -C abd clean
	make -C sodaw clean
	rm -rf  abdprocess
	make -C codes 
	make -C abd 
	make -C sodaw
	go build -x -o abdprocess  $(TARGET)

clean:
	rm -rf logs abdprocess 
