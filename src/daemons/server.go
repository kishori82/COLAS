package daemons

import (
	utilities "../utilities/GO"
	"encoding/base64"
	"fmt"
	"log"
	"time"
)

/*
#cgo CFLAGS: -I../abd  -I../sodaw
#cgo LDFLAGS: -L../abd  -labd  -L../sodaw -lsodaw -lzmq -lczmq
#include <abd_server.h>
*/
import "C"

func server_logger(status *C.SERVER_STATUS) {
	var cpu_use float64

	cpu_use = utilities.CpuUsage()
	for true {
		if data.active == true {
			log.Println("INFO", time.Now(), cpu_use, int(status.metadata_memory), int(status.data_memory), int(status.network_data))
		}
		time.Sleep(2 * 1000 * time.Millisecond)
		cpu_use = utilities.CpuUsage()
	}

}

func server_daemon() {
	active_chan = make(chan bool, 2)

	data.active = true
	//	data.name = "server_x"

	//var object_name string = "atomic_object"
	//	C.HiKodo()
	rand_data := make([]byte, (uint64)(1024*data.init_file_size))

	fmt.Println("init file ", 1024*data.init_file_size)
	_ = utilities.Generate_random_data(rand_data, int64(1024*data.init_file_size))
	encoded := base64.StdEncoding.EncodeToString(rand_data)
	init_data := C.CString(encoded)

	//	C.HelloKodo()
	var status C.SERVER_STATUS

	status.network_data = 0
	status.metadata_memory = 0
	status.data_memory = 0
	status.cpu_load = 0
	status.time_point = 0

	go C.server_process(C.CString(data.name), C.CString(data.port), init_data, &status)
	go server_logger(&status)

	time.Sleep(time.Second)

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

func Server_process(init_file_size float64) {
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
