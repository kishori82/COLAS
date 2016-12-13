package daemons

import (
	"fmt"
	"log"
	//	"math/rand"
	"time"
	"unsafe"
)

/*
#cgo CFLAGS: -I../abd -I../sodaw -I../utilities
#cgo LDFLAGS: -L../abd  -labd  -L../sodaw -lsodaw
#include <abd_client.h>
#include <sodaw_reader.h>
*/
import "C"

func reader_daemon() {
	active_chan = make(chan bool, 2)

//	var object_name string = "atomic_object"

	for {
		select {
		case active := <-active_chan: //start
			data.active = active
			ReinitializeParameters()
			LogParameters()
		case active := <-reset_chan:
			data.active = active
			data.write_counter = 0
		default:
			//		fmt.Println(data.active, data.servers)
			if data.active == true && len(data.servers) > 0 {

				rand_wait := rand_wait_time()*int64(time.Millisecond) + int64(time.Millisecond)

				time.Sleep(time.Duration(rand_wait))

				//servers_str := create_server_string_to_C()

				// call the ABD algorithm
				var data_read string
				var data_read_c *C.char

				start := time.Now()
				if data.algorithm == "ABD" {
					data_read_c = nil /* C.ABD_read(
					C.CString(object_name),
					C.CString(data.name),
					(C.uint)(data.write_counter),
					C.CString(servers_str),
					C.CString(data.port))*/

					data_read = C.GoString(data_read_c)
					//            C.free(unsafe.Pointer(&data_read_c))
				}

				// call the SODAW algorithm
				if data.algorithm == "SODAW" {
					data_read_c = nil /* C.SODAW_read(
					C.CString(object_name),
					C.CString(data.name),
					(C.uint)(data.write_counter),
					C.CString(servers_str),
					C.CString(data.port))
					*/

					data_read = C.GoString(data_read_c)
				}

				elapsed := time.Since(start)
				log.Println(data.run_id, "READ", string(data.name), data.write_counter, rand_wait/int64(time.Millisecond), elapsed, len(data_read))
				time.Sleep(5 * 1000 * time.Microsecond)
				C.free(unsafe.Pointer(data_read_c))

				data.write_counter += 1
			} else {
				time.Sleep(5 * 1000 * time.Microsecond)
			}
		}
	}
}

func Reader_process(parameters *Parameters) {

	// This should become part of the standard init function later when we refactor...
	data.processType = 0
	SetupLogging()
	fmt.Println("INFO\tStarting reader\n")

	InitializeParameters()
  printParameters(parameters)

	go HTTP_Server()

	log.Println("INFO\tStarting reader process\n")
	log.Println("INFO\tTIME in Milliseconds\n")

	reader_daemon()
}
