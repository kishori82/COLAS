ALL = abdprocess 

all: $(ALL)

ABD = src/abd.go

SRCS = src/controller/controller.go src/abd/abd_processes/utils.go src/abd/abd_processes/writer.go src/abd/abd_processes/reader.go src/abd/abd_processes/server.go

abdprocess: $(ABD) $(SRCS)
	go build -o abdprocess $(ABD)

clean:
	rm -rf logs abdprocess 
