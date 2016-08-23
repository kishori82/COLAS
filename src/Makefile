ALL = abdprocess 

all: $(ALL)

ABD = abd_algorithm.go

SRCS = controller/controller.go abd/go/utils.go abd/go/writer.go abd/go/reader.go abd/go/server.go

abdprocess: $(ABD) $(SRCS)
	go build -o abdprocess $(ABD)

clean:
	rm -rf logs abdprocess 
