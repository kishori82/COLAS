ALL = abdprocess 

all: $(ALL)

ABD = src/abd.go

SRCS = src/controller/controller.go src/abd/utils.go src/abd/writer.go src/abd/reader.go src/abd/server.go

abdprocess: $(ABD) $(SRCS)
	go build -o abdprocess $(ABD)

clean:
	rm -rf logs abdprocess 
