package abd_processes

import (
	utilities "../../utilities"
	"container/list"
	"encoding/base64"
	"fmt"
	"log"
	"math/rand"
	"time"
	"unsafe"
)

/*
#cgo CFLAGS: -I../C
#cgo LDFLAGS: -L../C  -labd_shared -lzmq -lczmq
#include <abd_client.h>
*/
import "C"

type Writer struct {
	Id int
}

/**
* Write a value to the set of nodes
* @param objectId the id of the object to be written
* @param value the value to be written
 */
func (w Writer) Write(objectId, value int) {
	PrintHeader("Writing")

	defer wg.Done()

	var stateVariables []StateVariable
	queryAll(objectId, &stateVariables)
	stateVariable := getMaxTag(stateVariables)
	newTag := Tag{w.Id, stateVariable.Tag.Value + 1}
	newStateVariable := StateVariable{newTag, value}
	sendAll(objectId, newStateVariable)

	PrintFooter()
}

func writer_deamon() {
	fmt.Println(C.ABD_hello(5, 7))
	active_chan = make(chan bool)

	data.active = true
	data.write_rate = 0.6
	data.name = "writer_1"
	data.servers["127.1.1.45"] = true
	data.servers["127.1.1.46"] = true
	var servers_str string = ""
	i := 0
	for key, _ := range data.servers {
		if i > 0 {
			servers_str += " "
		}
		servers_str += key
		i++
	}

	var object_name string = "atomic_object"

	for {
		select {
		case active := <-active_chan:
			data.active = active
		case active := <-reset_chan:
			data.active = active
			data.write_counter = 0
		default:
			if data.active == true {
				rand_wait := int64(1000 * rand.ExpFloat64() / data.write_rate)

				time.Sleep(time.Duration(rand_wait) * 1000 * time.Microsecond)

				rand_data_file_size := int64(1024 * rand.ExpFloat64() / data.file_size)

				rand_data := make([]byte, rand_data_file_size)
				_ = utilities.Generate_random_data(rand_data, rand_data_file_size)

				encoded := base64.StdEncoding.EncodeToString(rand_data)
				fmt.Println("Encoded data   : ", encoded)

				//decoded, _ := base64.StdEncoding.DecodeString(encoded)
				//fmt.Println("Decoded data", decoded)

				//		rawdata := (*C.byte)(unsafe.Pointer(&rand_data))
				rawdata := C.CString(encoded)
				defer C.free(unsafe.Pointer(&rawdata))

				C.ABD_write1(
					C.CString(object_name),
					C.CString(data.name),
					(C.uint)(data.write_counter), rawdata, (C.uint)(len(encoded)),
					C.CString(servers_str), C.CString(data.port))

				//	fmt.Println(rand_wait, len(rand_data), data.active)

				log.Println("WRITE", data.name, data.write_counter, rand_wait, len(rand_data))
				data.write_counter += 1
			} else {
				time.Sleep(5 * 1000000 * time.Microsecond)
			}
		}
	}

}

func Writer_process(ip_addrs *list.List) {
	// This should become part of the standard init function later when we refactor...
	SetupLogging()
	log.Println("INFO", data.name, "Starting")

	//Initialize the parameters
	InitializeParameters()

	// Keep running the server for now
	go HTTP_Server()

	writer_deamon()
}
