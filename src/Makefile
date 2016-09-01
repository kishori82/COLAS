ALL = abdprocess 

all: $(ALL)

TARGET= abdprocess.go

SRCS = daemons/controller.go daemons/utils.go daemons/writer.go daemons/reader.go daemons/server.go utilities/GO/utilities.go

CSRCS = abd/abd_client.c abd/abd_server.c abd/abd_client.h abd/abd_server.h  abd/algo_utils.h abd/algo_utils.c

abdprocess: $(TARGET) $(SRCS)  #$(CSRCS)
	go build -x -o abdprocess $(TARGET)

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

clean:
	rm -rf logs abdprocess 
