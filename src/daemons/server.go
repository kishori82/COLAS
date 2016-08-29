package abd_processes

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
	active_chan = make(chan bool)

	data.active = true
	data.name = "server_1"

	C.ABD_server_process( C.CString(data.name), C.CString(data.port) )
}

func Server_process() {
	fmt.Println("Starting server\n")
	f := SetupLogging()
	defer f.Close()
	// Run the server for now
	go HTTP_Server()
	InitializeParameters()
   server_daemon()
}
