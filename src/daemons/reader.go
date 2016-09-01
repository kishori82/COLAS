package daemons

import (
	"container/list"
	"fmt"
	"log"
	//	"math/rand"
	"time"
	//	"unsafe"
)

/*
#cgo CFLAGS: -I../abd -I../sodaw -I../utilities/C
#cgo LDFLAGS: -L../abd  -labd  -L../sodaw -lsodaw
#include <abd_client.h>
#include <sodaw_client.h>
*/
import "C"

func reader_daemon() {
	active_chan = make(chan bool, 2)

	var object_name string = "atomic_object"

	for {
		select {
		case active := <-active_chan:
			data.active = active
		case active := <-reset_chan:
			data.active = active
			data.write_counter = 0
		default:
			if data.active == true && len(data.servers) > 0 {

				fmt.Println(data.algorithm)
				rand_wait := rand_wait_time()*int64(time.Millisecond) + int64(time.Millisecond)
				//				log.Println("OPERATION\tSLEEP (millisecond)\t", rand_wait/int64(time.Millisecond))

				time.Sleep(time.Duration(rand_wait))

				//fmt.Println("OPERATION\tREAD", data.name, data.write_counter, "RAND TIME INT", rand_wait)
				//log.Println("OPERATION\tREAD", data.name, data.write_counter, "RAND TIME INT", rand_wait/int64(time.Millisecond))
				servers_str := create_server_string_to_C()
				//				log.Println("INFO\tUsing Servers\t" + servers_str)

				// call the ABD algorithm
				var data_read string
				var data_read_c *C.char

				start := time.Now()

				//log.Println(data.run_id, "READ", string(data.name), data.write_counter)
				if data.algorithm == "ABD" {

					data_read_c = C.ABD_read(
						C.CString(object_name),
						C.CString(data.name),
						(C.uint)(data.write_counter),
						C.CString(servers_str),
						C.CString(data.port))

					data_read = C.GoString(data_read_c)
					//            C.free(unsafe.Pointer(&data_read_c))
				}

				// call the ABD algorithm
				if data.algorithm == "SODAW" {
					data_read_c = C.SODAW_read(
						C.CString(object_name),
						C.CString(data.name),
						(C.uint)(data.write_counter),
						C.CString(servers_str),
						C.CString(data.port))
				}

				elapsed := time.Since(start)
				log.Println(data.run_id, "READ", string(data.name), data.write_counter,
					rand_wait/int64(time.Millisecond), elapsed, len(data_read))

				data.write_counter += 1
			} else {
				time.Sleep(5 * 1000 * time.Microsecond)
			}
		}
	}
}

func Reader_process(ip_addrs *list.List) {

	// This should become part of the standard init function later when we refactor...
	data.processType = 0
	SetupLogging()
	fmt.Println("INFO\tStarting reader\n")

	InitializeParameters()

	go HTTP_Server()

	log.Println("INFO\tStarting reader process\n")
	log.Println("INFO\tTIME in Milliseconds\n")
	reader_daemon()
}
