package daemons

import (
	"encoding/base64"
	"fmt"
	"log"
	//	"math/rand"
	"time"
	"unsafe"
)

/*
#cgo CFLAGS: -I../abd  -I../sodaw -I../utilities
#cgo LDFLAGS: -L../abd  -labd  -L../sodaw -lsodaw  -lzmq -lczmq
#include <abd_client.h>
#include <sodaw_writer.h>
#include <helpers.h>
#include <client.h>

#include <sodaw_reader.h>
#include <abd_reader.h>
*/
import "C"

func writer_daemon(cparameters *C.Parameters, parameters *Parameters) {
	active_chan = make(chan bool, 2)


	var client_args *C.ClientArgs
	client_args = C.create_ClientArgs(*cparameters)
	_= client_args
	var encoding_info *C.EncodeData
	encoding_info = C.create_EncodeData(*cparameters)
	_ = encoding_info

	var opnum int = 0
	var object_name string = "atomic_object"

	for {
		select {
		case active := <-active_chan:
			data.active = active
			ReinitializeParameters()
			LogParameters()
		case active := <-reset_chan:
			data.active = active
			data.write_counter = 0
		default:
			if data.active == true && len(data.servers) > 0 {
        opnum++
				//rand_wait := rand_wait_time()*int64(time.Millisecond) + int64(time.Millisecond)
				rand_wait := int64(parameters.Wait)*int64(time.Millisecond)
				time.Sleep(time.Duration(rand_wait))

				fmt.Printf("%s  %d %d %s %s\n", parameters.Server_id, opnum, rand_wait, C.GoString(client_args.servers_str), parameters.port)

				//rand_data_file_size := int64(1024 * rand.ExpFloat64() / data.file_size)
				rand_data_file_size := int64(1024 * data.file_size)

				rand_data := make([]byte, rand_data_file_size)
				_ = Generate_random_data(rand_data, rand_data_file_size)

				encoded := base64.StdEncoding.EncodeToString(rand_data)

				_ = encoded
				_ = object_name

				rawdata := C.CString(encoded)

				//	defer C.free(unsafe.Pointer(&rawdata))
				//servers_str := create_server_string_to_C()
				defer C.free(unsafe.Pointer(&rawdata))
				start := time.Now()
				//log.Println(len( C.GoString(rawdata)), servers_str)
				if data.algorithm == "ABD" {
					/*
						C.ABD_write(
							C.CString(object_name),
							C.CString(data.name),
							(C.uint)(data.write_counter),
							rawdata,
							(C.uint)(len(encoded)),
							C.CString(servers_str),
							C.CString(data.port))
					*/
				}

				if data.algorithm == "SODAW" {
					/*
						C.SODAW_write(
							C.CString(object_name),
							C.CString(data.name),
							(C.uint)(data.write_counter),
							rawdata,
							(C.uint)(len(encoded)),
							C.CString(servers_str),
							C.CString(data.port))
					*/
				}

				elapsed := time.Since(start)

				log.Println(data.run_id, "WRITE", string(data.name), data.write_counter,
					rand_wait/int64(time.Millisecond), elapsed, len(encoded))

				C.free(unsafe.Pointer(rawdata))

				data.write_counter += 1
			} else {
				time.Sleep(5 * 1000 * time.Microsecond)
			}
		}
	}
}

func Writer_process(parameters *Parameters) {
	// This should become part of the standard init function later when we refactor...
	SetupLogging()
	log.Println("INFO", data.name, "Starting")

	data.processType = 1
	//Initialize the parameters
	InitializeParameters()

	printParameters(parameters)
	// Keep running the server for now
	go HTTP_Server()

	log.Println("INFO\tStarting writer process\n")
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
   
	writer_daemon(&cparameters, parameters)
}
