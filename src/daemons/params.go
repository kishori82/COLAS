package daemons

import (
	"math"
)

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
