ALL = abd-process 

all: $(ALL)

ABD = src/abd.go

abd-process: $(ABD)
	go build -o abd-process $(ABD)

clean:
	rm -rf logs abd-process 
