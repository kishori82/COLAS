package daemons

import (
	"fmt"
	"time"
	utilities "../utilities/GO"
	"encoding/base64"
	"log"
	"io/ioutil"
	"strings"
	"strconv"
)

/*
#cgo CFLAGS: -I../abd  -I../soda
#cgo LDFLAGS: -L../abd  -labd  -L../soda -lsoda -lzmq -lczmq
#include <abd_server.h>
*/
import "C"

func getCPUSample() (idle, total uint64) {
    contents, err := ioutil.ReadFile("/proc/stat")
    if err != nil {
        return
    }
    lines := strings.Split(string(contents), "\n")
    for _, line := range lines {
        fields := strings.Fields(line)
        if fields[0] == "cpu" {
            numFields := len(fields)
            for i := 1; i < numFields; i++ {
                val, err := strconv.ParseUint(fields[i], 10, 64)
                if err != nil {
                    fmt.Println("Error: ", i, fields[i], err)
                }
                total += val // tally up all the numbers to get total ticks
                if i == 4 {  // idle is the 5th field in the cpu line
                    idle = val
                }
            }
            return
        }
    }
    return
}

func cpuUsage() float64 {
    idle0, total0 := getCPUSample()
    time.Sleep(3 * time.Second)
    idle1, total1 := getCPUSample()

    idleTicks := float64(idle1 - idle0)
    totalTicks := float64(total1 - total0)
    cpuUsage := 100 * (totalTicks - idleTicks) / totalTicks

    //fmt.Printf("CPU usage is %f%% [busy: %f, total: %f]\n", cpuUsage, totalTicks-idleTicks, totalTicks)
    return cpuUsage
}


func server_logger(status *C.SERVER_STATUS) {
    var cpu_use float64

    cpu_use = cpuUsage()
    for true {
		   if  data.active==true {
		      log.Println("INFO",time.Now(), cpu_use, int(status.metadata_memory), int(status.data_memory), int(status.network_data))
			 }
		   time.Sleep(2 * 1000 * time.Millisecond)
       cpu_use = cpuUsage()
		}

}


func server_daemon() {
	active_chan = make(chan bool, 2)

	data.active = true
	data.name = "server"

	//var object_name string = "atomic_object"
	//	C.HiKodo()
	rand_data := make([]byte, data.init_file_size)
	_ = utilities.Generate_random_data(rand_data, int64(data.init_file_size))
	encoded := base64.StdEncoding.EncodeToString(rand_data)
	init_data := C.CString(encoded)

//	C.HelloKodo()
  var status C.SERVER_STATUS;

  status.network_data = 0;
  status.metadata_memory= 0;
  status.data_memory= 0;
  status.cpu_load = 0;
  status.time_point = 0;


	go C.ABD_server_process(C.CString(data.name), C.CString(data.port), init_data, &status)
	go server_logger(&status);

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

func Server_process(init_file_size uint64) {
	fmt.Println("Starting server\n")
	data.processType = 2
	data.init_file_size = init_file_size

	f := SetupLogging()
	defer f.Close()
	// Run the server for now
  fmt.Println("newroks ")
	go HTTP_Server()
  fmt.Println("newroks ")
	InitializeParameters()
  fmt.Println("newroks ")
	server_daemon()
}
