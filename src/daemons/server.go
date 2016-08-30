package daemons

import (
	"fmt"
	"time"
	utilities "../utilities/GO"
	"encoding/base64"
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

	//var object_name string = "atomic_object"
	//	C.HiKodo()
	rand_data := make([]byte, data.init_file_size)
	_ = utilities.Generate_random_data(rand_data, int64(data.init_file_size))
	encoded := base64.StdEncoding.EncodeToString(rand_data)
	rawdata := C.CString(encoded)

//	C.HelloKodo()
	go C.ABD_server_process(C.CString(data.name), C.CString(data.port), rawdata )
	for {
		select {
		case active := <-active_chan:
			data.active = active
		case active := <-reset_chan:
			data.active = active
			data.write_counter = 0
		default:
			if data.active == true && len(data.servers) > 0 {
				time.Sleep(5 * 1000 * time.Microsecond)
			} else {
				time.Sleep(5 * 1000 * time.Microsecond)
			}
		}
	}
}

func Server_process(init_file_size uint64) {
	fmt.Println("Starting server\n")
	data.processType = 2
	data.init_file_size = init_file_size

	f := SetupLogging()
	defer f.Close()
	// Run the server for now
	go HTTP_Server()
	InitializeParameters()
	server_daemon()
}
