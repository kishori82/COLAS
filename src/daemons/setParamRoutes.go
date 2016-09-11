package daemons

import (
	utilities "../utilities/GO"
	"fmt"
	"github.com/gorilla/mux"
	"log"
	"math/rand"
	"net/http"
	"strconv"
	"strings"
)

// this process registers the readers in the controller and then
// registers the servers in each readers
func SetServers(w http.ResponseWriter, r *http.Request) {
	log.Println("INFO\tSet Servers")
	fmt.Println("INFO\tSet Servers")

	vars := mux.Vars(r)
	ip := vars["ip"]
	ips := strings.Split(ip, DELIM)

	for i := range ips {
		data.servers[ips[i]] = true
	}

	fmt.Println(" servers  ", data.processType)
	if data.processType == 3 {
		var clients map[string]bool = data.servers
		serverListStr := create_server_list_string()
		j := 0
		for ipaddr := range clients {
			send_command_to_process(ipaddr, "SetServers", serverListStr)
			name := "server_" + fmt.Sprintf("%d", j)
			fmt.Println(name, ipaddr)
			send_command_to_process(ipaddr, "SetName", name)
			j = j + 1
		}
	}
}

// this process registers the writers in the controller and then
// registers the servers in each writer
func SetWriters(w http.ResponseWriter, r *http.Request) {
	log.Println("INFO\tSet Writers")
	fmt.Println("INFO\tSet Writers")

	vars := mux.Vars(r)
	ip := vars["ip"]

	ips := strings.Split(ip, DELIM)

	for i := range ips {
		data.writers[ips[i]] = true
	}

	if data.processType == 3 {
		var clients map[string]bool = data.writers
		serverListStr := create_server_list_string()
		j := 0
		for ipaddr := range clients {
			send_command_to_process(ipaddr, "SetServers", serverListStr)
			name := "writer_" + fmt.Sprintf("%d", j)
			send_command_to_process(ipaddr, "SetName", name)
			j = j + 1
		}
	}

}

// this process registers the writers in the controller and then
// registers the servers in each writer
func SetReaders(w http.ResponseWriter, r *http.Request) {
	log.Println("INFO\tSetReaders")
	fmt.Println("INFO\tSetReaders")

	vars := mux.Vars(r)
	ip := vars["ip"]
	ips := strings.Split(ip, DELIM)
	for i := range ips {
		data.readers[ips[i]] = true
	}

	if data.processType == 3 {
		fmt.Println("readers", data.readers)
		var clients map[string]bool = data.readers
		serverListStr := create_server_list_string()
		j := 0
		for ipaddr := range clients {
			send_command_to_process(ipaddr, "SetServers", serverListStr)
			name := "reader_" + fmt.Sprintf("%d", j)
			send_command_to_process(ipaddr, "SetName", name)
			j = j + 1
		}
	}
}

//set run id
func SetRunId(w http.ResponseWriter, r *http.Request) {
	log.Println("INFO\tSet Run ID")
	fmt.Println("INFO\tSet Run ID")

	vars := mux.Vars(r)
	id := vars["id"]

	data.run_id = id

	if data.processType == 3 {
		send_command_to_processes(data.readers, "SetRunId", data.run_id)
		send_command_to_processes(data.writers, "SetRunId", data.run_id)
		send_command_to_processes(data.servers, "SetRunId", data.run_id)
	}
}

//set run id
func SetAlgorithm(w http.ResponseWriter, r *http.Request) {
	log.Println("INFO\tSet Algorithm ID")
	fmt.Println("INFO\tSet Algorithm ID")

	vars := mux.Vars(r)
	id := vars["id"]

	data.algorithm = id

	if data.processType == 3 {
		send_command_to_processes(data.readers, "SetAlgorithm", data.algorithm)
		send_command_to_processes(data.writers, "SetAlgorithm", data.algorithm)
		send_command_to_processes(data.servers, "SetAlgorithm", data.algorithm)
	}
}

// writer and readers
func SetSeed(w http.ResponseWriter, r *http.Request) {
	log.Println("INFO\tSet Seed")
	fmt.Println("INFO\tSet Seed")

	vars := mux.Vars(r)
	ip := vars["seed"]
	ips := strings.Split(ip, DELIM)

	if len(ips) != 1 {
		fmt.Printf("%s\n", ip)
		fmt.Printf("Expected 1 parameters, found %d\n", len(ips))
	}

	s, _ := strconv.ParseInt(ips[0], 10, 64)
	data.rand_seed = s

	const RANDSIZE int = 1000000000
	if data.processType == 3 {
		utilities.Set_random_seed(s)
		for ipaddr, _ := range data.readers {
			seed := rand.Intn(RANDSIZE)
			seed_str := fmt.Sprintf("%d", seed)
			send_command_to_process(ipaddr, "SetName", seed_str)
		}

		for ipaddr, _ := range data.writers {
			seed := rand.Intn(RANDSIZE)
			seed_str := fmt.Sprintf("%d", seed)
			send_command_to_process(ipaddr, "SetName", seed_str)
		}
	}
}

//set inter read wait time distribution
func SetReadRateDistribution(w http.ResponseWriter, r *http.Request) {
	vars := mux.Vars(r)
	ip := vars["param"]
	ips := strings.Split(ip, DELIM)

	log.Println("INFO\tSet Inter  Read Wait Time  Distribution\t" + ip)
	fmt.Println("INFO\tSet Inter  Read Wait Time  Distribution\t" + ip)

	if len(ips) != 3 {
		fmt.Printf("%s\n", ip)
		fmt.Printf("Expected 1 parameters, found %d\n", len(ips))
	}
	name := ips[0]
	var m string
	k := ips[1]

	if len(ips) > 2 {
		m = ips[2]
		data.inter_read_wait_distribution = []string{name, k, m}
	} else {
		data.inter_read_wait_distribution = []string{name, k}
	}

	if data.processType == 3 {
		send_command_to_processes(data.readers, "SetReadRateDistribution", ip)
		send_command_to_processes(data.writers, "SetReadRateDistribution", ip)
	}
}

//set inter read wait time distribution
func SetWriteRateDistribution(w http.ResponseWriter, r *http.Request) {
	log.Println("INTO\tSet Inter Write Wait Time Distribution")
	vars := mux.Vars(r)
	ip := vars["param"]
	ips := strings.Split(ip, DELIM)

	fmt.Printf("%s\n", ip)
	name := ips[0]

	log.Println("INFO\tSet Inter Write Wait Time  Distribution--\t" + ip)
	var m string
	k := ips[1]

	if len(ips) > 2 {
		m = ips[2]
		data.inter_write_wait_distribution = []string{name, k, m}
	} else {
		data.inter_write_wait_distribution = []string{name, k}
	}

	log.Println("INFO\tSet Inter Write Wait Time  Distribution\t" + ip)
	if data.processType == 3 {
		fmt.Println(data.inter_write_wait_distribution)
		send_command_to_processes(data.readers, "SetReadWriteDistribution", ip)
		send_command_to_processes(data.writers, "SetReadWriteDistribution", ip)
	}
}

// set write to option disk or mem
func SetWriteTo(w http.ResponseWriter, r *http.Request) {
	log.Println("INFO\tSet Write To")
	fmt.Println("INFO\tSet Write To")

	vars := mux.Vars(r)
	ip := vars["param"]

	data.writeto = ip

	if data.processType == 3 {
		send_command_to_processes(data.writers, "SetWriteTo", ip)
		send_command_to_processes(data.readers, "SetWriteTo", ip)
		send_command_to_processes(data.servers, "SetWriteTo", ip)
	}
}

// set file size
func SetFileSize(w http.ResponseWriter, r *http.Request) {
	vars := mux.Vars(r)
	ip := vars["size"]
	k, err := strconv.ParseFloat(ip, 64)

	log.Println("INFO\tSETTING FILE SIZE (KB) \t" + ip)
	fmt.Println("INFO\tSETTING FILE SIZE (KB) \t" + ip)

	if err != nil {
		data.file_size = 0.1
	}
	data.file_size = k

	if data.processType == 3 {
		send_command_to_processes(data.writers, "SetFileSize", ip)
	}
}
