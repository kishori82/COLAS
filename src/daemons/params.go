package daemons

/*
#cgo CFLAGS: -I../abd -I../sodaw -I../utilities -I..
#cgo LDFLAGS: -L../abd  -labd  -L../sodaw -lsodaw
#include <abd_client.h>
#include <sodaw_reader.h>
#include <helpers.h>

char **get_memory_for_ipaddresses(int num_ips) {
    char **ipaddresses =  (char **)malloc(num_ips *sizeof(char *));
    int i;
    for( i =0; i < num_ips; i++)  {
        ipaddresses[i] = (char *)malloc(16*sizeof(char));
    }
    return ipaddresses;
}


char *get_begin_location(char **a, int i){
    return a[i];
}
*/
import "C"


import (
	"math"
	"container/list"
	"fmt"
)

const PORT = "8081"
const PORT1 = "8082"
const sodaw = "SODAW"
const abd = "ABD"
const full_vector = "full_vector"
const reed_solomon = "reed_solomon"
const reader = 0
const writer = 1
const server = 2
const controller = 3

type Parameters struct {
    Ipaddresses string
    Ip_list []string
    Num_servers uint
    Server_id string 
    port string 
    port1 string 
    Algorithm string
    Coding_algorithm string 
    Wait uint64
    Filesize_kb float64
    Processtype uint64
} 



type ClientArgs struct {
    Algorithm string
	  Ip_addrs  list.List
	  Wait uint64
	  Filesize float64 
}

type EncodingInfo struct {
    Code string
}


type Params struct {
	processType    int8
	init_file_size float64

	readers map[string]bool
	servers map[string]bool
	writers map[string]bool

	inter_read_wait_distribution  []string
	inter_write_wait_distribution []string

	file_size float64
	rand_seed int64

	active bool
	port   string

	read_counter, write_counter int64
	name                        string

	algorithm string
	run_id    string
	writeto   string

	K                uint64
	N                uint64
	coding_algorithm uint32
	symbol_size      int
}

func InitializeParameters() {
	data.readers = make(map[string]bool)
	data.servers = make(map[string]bool)
	data.writers = make(map[string]bool)

	//data.inter_read_wait_distribution = []string{"erlang", "1", "1"}
	//data.inter_write_wait_distribution = []string{"erlang", "1", "1"}

	data.inter_read_wait_distribution = []string{"const", "100"}
	data.inter_write_wait_distribution = []string{"const", "100"}

	//	data.write_rate = 0.6
	//	data.read_rate = 0.6
	data.file_size = 0.1
	data.init_file_size = 0.4
	data.rand_seed = 1
	data.read_counter = 0
	data.write_counter = 0
	data.active = false
	data.port = "8081"

	data.N = 1
	data.K = uint64(math.Ceil((float64(data.N) + 1) / 2.0))

	data.coding_algorithm = 0
	data.symbol_size = 1024
	//data.algorithm = "ABD"
	data.algorithm = "SODAW"
	data.run_id = "DEFULT_RUN"
	data.writeto = "ram"
	data.name = "default"
	if data.processType == 0 {
		data.name = "reader_0"
	}
	if data.processType == 1 {
		data.name = "writer_0"
	}
	if data.processType == 2 {
		data.name = "server_0"
	}

	data.active = false

	/*
		data.active = true
		data.algorithm = "SODAW"
		data.servers["172.17.0.2"] = true
		data.servers["172.17.0.3"] = true
		data.servers["172.17.0.4"] = true
	*/
}

func ReinitializeParameters() {
	data.N = uint64(len(data.servers))
	data.K = uint64(math.Ceil((float64(data.N) + 1) / 2.0))
}




func SetDefaultParameters(parameters *Parameters) {
    parameters.Num_servers = 0
    parameters.Ipaddresses = ""
    parameters.Algorithm = sodaw
    parameters.Coding_algorithm = full_vector
    parameters.Wait = 100
    parameters.Filesize_kb = 1.1
    parameters.Processtype = 2
    parameters.port = PORT
    parameters.port1 = PORT1
}



func printParameters(parameters *Parameters) {

    fmt.Printf("Parameters [GO]\n");
    fmt.Printf("\tName  \t\t\t\t: %s\n", parameters.Server_id);;

    switch parameters.Processtype  {
       case reader:
          fmt.Printf("\tprocesstype\t\t\t: %s\n", "reader")
       case writer:
           fmt.Printf("\tprocesstype\t\t\t: %s\n", "writer")
       case server:
           fmt.Printf("\tprocesstype\t\t\t: %s\n", "server")
       default:
          panic("Do not recognize the process type know what it is")
    }

    fmt.Printf("\tnum servers\t\t\t: %d\n", parameters.Num_servers)

    for i:=0; i < len(parameters.Ip_list) ; i++  {
        fmt.Printf("\t\tserver %d\t\t: %s\n",i, parameters.Ip_list[i] )
    }

    switch parameters.Algorithm  {
       case sodaw:
          fmt.Printf("\talgorithm\t\t\t: %s\n", sodaw   )
       case abd:
           fmt.Printf("\talgorithm\t\t\t: %s\n", abd )
       default:
           panic("Cannot recognize algorithm")
    }

    switch parameters.Coding_algorithm  {
       case full_vector:
          fmt.Printf("\tcoding algorithm\t\t: %s\n", "RLNC" )
       case reed_solomon:
           fmt.Printf("\tcoding algorithm\t\t: %s\n", "REED-SOLOMON" )
       default:
           panic("Cannot recognize the coding algorithm")
    }
    fmt.Printf("\tinter op wait (ms)\t\t: %d\n", parameters.Wait)
    fmt.Printf("\tfile size (KB)\t\t\t: %.2f\n", parameters.Filesize_kb)
}

func copyGoParamToCParam(cparameters *C.Parameters, parameters *Parameters) {
   cparameters.num_servers = C.uint(parameters.Num_servers)

   fmt.Printf("lenth of ipaddress %d\n", len(parameters.Ip_list))

   cparameters.ipaddresses = C.get_memory_for_ipaddresses( C.int(parameters.Num_servers))
	 
   for i :=0; i < int(parameters.Num_servers); i++  {
        C.strcpy(C.get_begin_location(cparameters.ipaddresses, C.int(i) ),  C.CString(parameters.Ip_list[i]))
    }

    switch  parameters.Algorithm {
		    case abd:
             cparameters.algorithm = 0
		    case sodaw:
             cparameters.algorithm = 1
				default:
				    panic("Unknown choice for algorithm")
		}

    switch  parameters.Coding_algorithm {
		    case full_vector:
             cparameters.coding_algorithm = 0
		    case reed_solomon:
             cparameters.algorithm = 1
				default:
				    panic("Unknown choice for coding algorithm")
		}


    cparameters.wait = C.int(parameters.Wait)
    cparameters.filesize_kb = C.float(parameters.Filesize_kb)

    switch  parameters.Processtype {
		    case 0:
             cparameters.processtype = 0
		    case 1:
             cparameters.processtype = 1
		    case 2:
             cparameters.processtype = 2
		    case 3:
             cparameters.processtype = 3
				default:
				    panic("Unknown choice for process typ")
		}
    C.strcpy(&cparameters.port[0],  C.CString(parameters.port))
    C.strcpy(&cparameters.port1[0],  C.CString(parameters.port1))
}

