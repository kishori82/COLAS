package daemons

import (
	"container/list"
	"fmt"
	"log"
	"math/rand"
	"time"
)

/*
#cgo CFLAGS: -I../abd -I../soda -I../utilities/C 
#cgo LDFLAGS: -L../abd  -labd  -L../soda -lsoda -lzmq -lczmq 
#include <abd_client.h>
#include <soda_client.h>
*/
import "C"

var active_chan chan bool
var reset_chan chan bool

func reader_daemon() {
	active_chan = make(chan bool)

	var object_name string = "atomic_object"

//	C.HiKodo()
//	C.HelloKodo()

	for {
		select {
		case active := <-active_chan:
			data.active = active
		case active := <-reset_chan:
			data.active = active
			data.write_counter = 0
		default:
			if data.active == true && len(data.servers) > 0 {
				rand_wait := int64(1000 * rand.ExpFloat64() / data.write_rate)

				time.Sleep(time.Duration(rand_wait) * 1000 * time.Microsecond)

				fmt.Println("OPERATION\tREAD", data.name, data.write_counter, "RAND TIME INT", rand_wait)
				log.Println("OPERATION\tREAD", data.name, data.write_counter, "RAND TIME INT", rand_wait)
				servers_str := create_server_string_to_C()
				log.Println("INFO\tUsing Servers\t" + servers_str)

         // call the ABD algorithm
				var data_read string
				if data.algorithm == "ABD" {
					data_read = C.GoString(C.ABD_read(
						C.CString(object_name),
						C.CString(data.name),
						(C.uint)(data.write_counter),
						C.CString(servers_str),
						C.CString(data.port)))
				}

         // call the ABD algorithm
				if data.algorithm == "SODA" {
					data_read = "SODA" //C.HelloKodo()
				}

				fmt.Println("\t\t\tREAD DATA : ", data_read)
				log.Println("OPERATION\tREAD", data.name, data.write_counter, rand_wait, data_read)
				data.write_counter += 1
			} else {
				time.Sleep(5 * 1000000 * time.Microsecond)
			}
		}
	}
}

func Reader_process(ip_addrs *list.List) {

	// This should become part of the standard init function later when we refactor...
	data.processType=0
	SetupLogging()
	fmt.Println("Starting reader\n")

	InitializeParameters()

	go HTTP_Server()

	log.Println("INFO\tStarting reader process\n")
	reader_daemon()
}
