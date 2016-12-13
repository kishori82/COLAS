package daemons

import (
	"math"
	"container/list"
)

const PORT = "8081"
const PORT1 = "8082"
const sodaw = "SODAW"
const abd = "ABD"
const full_vector = "full_vector"
const reed_solomon = "reed_solomon"

type Parameters struct {
    Ipaddresses string
    Ip_list []string
    num_servers uint
    server_id string 
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
    parameters.num_servers = 0
    parameters.Ipaddresses = ""
    parameters.Algorithm = sodaw
    parameters.Coding_algorithm = full_vector
    parameters.Wait = 100
    parameters.Filesize_kb = 1.1
    parameters.Processtype = 2
    parameters.port = PORT
    parameters.port1 = PORT1
}



func printParameters(parameters Parameters) {
    var i  int 

    fmt.Printf("Parameters\n");
    fmt.Printf("\tName  \t\t\t\t: %s\n", parameters.server_id);;

    switch(parameters.processtype) {
    case reader:
        fmt.Printf("\tprocesstype\t\t\t: %s\n", "reader");
        break;
    case writer:
        fmt.Printf("\tprocesstype\t\t\t: %s\n", "writer");
        break;
    case server:
        fmt.Printf("\tprocesstype\t\t\t: %s\n", "server");
        break;
    default:
        break;
    }

    fmt.Printf("\tnum servers\t\t\t: %d\n", parameters.num_servers);
    for(i=0; i < parameters.num_servers; i++) {
        fmt.Printf("\t\tserver %d\t\t: %s\n",i, parameters.ipaddresses[i] );
    }

    switch(parameters.algorithm) {
    case sodaw:
        fmt.Printf("\talgorithm\t\t\t: %s\n", SODAW   );
        break;
    case abd:
        fmt.Printf("\talgorithm\t\t\t: %s\n", ABD );
        break;
    default:
        break;
    }

    switch(parameters.coding_algorithm) {
    case full_vector:
        fmt.Printf("\tcoding algorithm\t\t: %s\n", "RLNC"   );
        break;
    case reed_solomon:
        fmt.Printf("\tcoding algorithm\t\t: %s\n", "REED-SOLOMON" );
        break;
    default:
        break;
    }
    fmt.Printf("\tinter op wait (ms)\t\t: %d\n", parameters.wait);
    fmt.Printf("\tfile size (KB)\t\t\t: %.2f\n", parameters.filesize_kb);
}


