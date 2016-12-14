package daemons

import (
	//	"encoding/base64"
	"fmt"
	"log"
	"math/rand"
	"time"
	"unsafe"
)

/*
#cgo CFLAGS: -I../abd  -I../sodaw -I../utilities
#cgo LDFLAGS: -L../abd  -labd  -L../sodaw -lsodaw  -lzmq -lczmq

#include <helpers.h>
#include <algo_utils.h>

#include <abd_client.h>
#include <client.h>
#include <abd_writer.h>
#include <sodaw_writer.h>
*/
import "C"

func writer_daemon(cparameters *C.Parameters, parameters *Parameters) {
	active_chan = make(chan bool, 2)

	var client_args *C.ClientArgs = C.create_ClientArgs(*cparameters)
	var encoding_info *C.EncodeData = C.create_EncodeData(*cparameters)
	var abd_data *C.RawData = C.create_RawData(*cparameters)

	s1 := rand.NewSource(time.Now().UnixNano())
	ran := rand.New(s1)

	var payload *C.char
	var opnum int = 0
	var payload_size uint

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
				rand_wait := int64(parameters.Wait) * int64(time.Millisecond)
				time.Sleep(time.Duration(rand_wait))

				fmt.Printf("%s  %d %d %s %s\n", parameters.Server_id, opnum, rand_wait, C.GoString(client_args.servers_str), parameters.port)

				start := time.Now()

				payload_size = uint((parameters.Filesize_kb + float64(ran.Intn(100000000)%5)) * 1024)
				fmt.Printf("payload %d\n", payload_size)
				payload = C.get_random_data(C.uint(payload_size))

				//log.Println(len( C.GoString(rawdata)), servers_str)
				if data.algorithm == "ABD" {
					abd_data.data = unsafe.Pointer(payload)
					abd_data.data_size = C.int(payload_size)
					C.ABD_write(C.CString("atomic_object"), C.uint(opnum), abd_data, client_args);
				}

				if data.algorithm == "SODAW" {
					C.SODAW_write(C.CString("atomic_object"), C.uint(opnum), payload, C.uint(payload_size), encoding_info, client_args)
				}

				elapsed := time.Since(start)

				log.Println(data.run_id, "WRITE", string(data.name), data.write_counter,
					rand_wait/int64(time.Millisecond), elapsed)
        C.free(payload)
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

/*
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

				C.free(unsafe.Pointer(rawdata))

*/
