ALL = abdprocess 

all: $(ALL)

ABD = abdprocess.go

SRCS = controller/controller.go abd/go/utils.go abd/go/writer.go abd/go/reader.go abd/go/server.go utilities/utilities.go

CSRCS = abd/C/abd_client.c abd/C/abd_server.c abd/C/abd_client.h abd/C/abd_server.h  abd/C/algo_utils.h abd/C/algo_utils.c

abdprocess: $(ABD) $(SRCS)  $(CSRCS)
	go build -o abdprocess $(ABD)

clean:
	rm -rf logs abdprocess 
