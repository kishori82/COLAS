GOROOT := /home/docker/go
GOPATH := /home/docker/COLAS/AB

all: abdprocess

abdprocess: abdprocess.go
    # always format code
	GOPATH=$(GOPATH) go fmt $^
    # binary
	GOPATH=$(GOPATH) go build -o abdprocess -v $^
