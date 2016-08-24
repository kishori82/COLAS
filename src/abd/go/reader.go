package abd_processes

import (
	//	utilities "../../utilities"
	"container/list"
	//	"encoding/base64"
	"fmt"
	"log"
	"math/rand"
	"time"
	//"unsafe"
	//utilities "../utilities/"
)

/*
#cgo CFLAGS: -I../C
#cgo LDFLAGS: -L../C  -labd_shared -lzmq -lczmq
#include <abd_client.h>
*/
import "C"

var active_chan chan bool
var reset_chan chan bool

func reader_daemon() {
	active_chan = make(chan bool)

	data.active = true
	data.write_rate = 0.6
	data.name = "reader_1"

	data.servers["172.17.0.2"] = true

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

				//decoded, _ := base64.StdEncoding.DecodeString(encoded)
				//fmt.Println("Decoded data", decoded)
				//		rawdata := (*C.byte)(unsafe.Pointer(&rand_data))

				fmt.Println("READ", data.name, data.write_counter, "RAND TIME INT", rand_wait)
				data_read := C.GoString(C.ABD_read(
					C.CString(object_name),
					C.CString(data.name),
					(C.uint)(data.write_counter),
					C.CString(servers_str),
					C.CString(data.port)))

				//	fmt.Println(rand_wait, len(rand_data), data.active)

				fmt.Println("\t\t\tREAD DATA : ", data_read)
				log.Println("READ", data.name, data.write_counter, rand_wait, data_read)
				data.write_counter += 1
			} else {
				time.Sleep(5 * 1000000 * time.Microsecond)
			}
		}
	}

}

func Reader_process(ip_addrs *list.List) {

	// This should become part of the standard init function later when we refactor...
	SetupLogging()
	fmt.Println("Starting reader\n")

	InitializeParameters()

	go HTTP_Server()

	reader_daemon()

	for {
		select {
		case active := <-active_chan:
			data.active = active
		case active := <-reset_chan:
			data.active = active
			data.read_counter = 0
		default:
			if data.active == true {
				rand_wait := exponential_wait(data.write_rate)
				time.Sleep(time.Duration(rand_wait) * 1000 * time.Microsecond)
				log.Println("READ", data.name, data.read_counter, rand_wait)
				data.read_counter += 1

			} else {
				time.Sleep(5 * 1000000 * time.Microsecond)
			}
		}
	}
}
