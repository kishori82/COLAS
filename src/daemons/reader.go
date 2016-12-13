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
#include <client.h>
#include <sodaw_reader.h>
#include <abd_reader.h>
#include <helpers.h>
*/
import "C"

func reader_daemon(cparameters *C.Parameters, parameters *Parameters) {
	active_chan = make(chan bool, 2)

	//	var object_name string = "atomic_object"

	var client_args *C.ClientArgs
	client_args = C.create_ClientArgs(*cparameters)
	var encoding_info *C.EncodeData
	encoding_info = C.create_EncodeData(*cparameters)
	_ = encoding_info

	var opnum int = 0

	for {
		select {
		case active := <-active_chan: //start
			data.active = active
			ReinitializeParameters()
			LogParameters()
			//opnum = 0
		case active := <-reset_chan:
			data.active = active
			data.write_counter = 0
		default:
			if data.active == true && len(data.servers) > 0 {
			  opnum++

				//rand_wait := rand_wait_time()*int64(time.Millisecond) + int64(time.Millisecond)
				rand_wait := int64(parameters.Wait)*int64(time.Millisecond)
				time.Sleep(time.Duration(rand_wait))

				fmt.Printf("%s %d %d %s %s\n", parameters.Server_id, opnum, rand_wait, C.GoString(client_args.servers_str), parameters.port)

				//servers_str := create_server_string_to_C()

				// call the ABD algorithm
				var data_read string
				var data_read_c *C.char

				start := time.Now()
				if data.algorithm == "ABD" {
					_ = C.ABD_read( C.CString("atomic_object"), C.uint(opnum), client_args)
					data_read_c = nil /* C.ABD_read(
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

	var cparameters C.Parameters
	copyGoParamToCParam(&cparameters, parameters)
	if parameters.Num_servers > 0 {
		data.active = true
		for i := 0; i < int(parameters.Num_servers); i++ {
			data.servers[parameters.Ip_list[i]] = true
		}

	}
	C.printParameters(cparameters)

	reader_daemon(&cparameters, parameters)
}
