ALL = abdprocess 

all: $(ALL)

ABD = abdprocess.go

SRCS = controller/controller.go daemons/utils.go daemons/writer.go daemons/reader.go daemons/server.go utilities/GO/utilities.go

CSRCS = abd/abd_client.c abd/abd_server.c abd/abd_client.h abd/abd_server.h  abd/algo_utils.h abd/algo_utils.c

abdprocess: $(ABD) $(SRCS)  #$(CSRCS)
	go build -x -o abdprocess $(ABD)

clean:
	rm -rf logs abdprocess 
