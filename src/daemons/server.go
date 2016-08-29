package daemons

import (
	"fmt"
)
/*
#cgo CFLAGS: -I../abd  -I../soda
#cgo LDFLAGS: -L../abd  -labd  -L../soda -lsoda -lzmq -lczmq
#include <abd_server.h>
*/
import "C"

func server_daemon() {
	active_chan = make(chan bool, 2)

	data.active = true
	data.name = "server"

	C.ABD_server_process( C.CString(data.name), C.CString(data.port) )
}

func Server_process() {
	fmt.Println("Starting server\n")
	data.processType=2

	f := SetupLogging()
	defer f.Close()
	// Run the server for now
	go HTTP_Server()
	InitializeParameters()
   server_daemon()
}
