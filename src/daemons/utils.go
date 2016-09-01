package daemons

import (
	utilities "../utilities/GO"
	"container/list"
	"fmt"
	"github.com/gorilla/mux"
	"io/ioutil"
	"log"
	"math"
	"math/rand"
	"net/http"
	"os"
	"strconv"
	"strings"
)

type Tag struct {
	z  int
	id string
}

type Params struct {
	processType int8
	init_file_size  uint64

	readers map[string]bool
	servers map[string]bool
	writers map[string]bool

	inter_read_wait_distribution  []string
	inter_write_wait_distribution []string

	write_rate float64
	read_rate  float64
	file_size  float64
	rand_seed  int64

	active bool
	port   string

	read_counter, write_counter int64
	name                        string

	algorithm string
	run_id    string
	writeto   string
}

var data Params
var DELIM string = "_"

func PrintHeader(title string) {
	length := len(title)
	numSpaces := 22
	leftHalf := numSpaces + int(math.Ceil(float64(length)/2))
	rightHalf := numSpaces - int(math.Ceil(float64(length)/2))
	fmt.Println("***********************************************")
	fmt.Println("*                                             *")
	fmt.Print("*")
	fmt.Printf("%*s", int(leftHalf), title)
	fmt.Printf("%*s", (int(rightHalf) + 1), " ")
	fmt.Println("*")
	fmt.Println("*                                             *")
	fmt.Println("***********************************************")
}

/**
* Print out a footer to the screen
 */
func PrintFooter() {
	fmt.Println("***********************************************")
}

func Print_configuration(proc_type uint64, ip_addrs *list.List) {

	if proc_type == 0 {
		//fmt.Printf("Process Type: %d\n", proc_type)
		fmt.Printf("Process Reader \n")
	}

	if proc_type == 1 {
		fmt.Printf("Process Writer \n")
	}
	if proc_type == 2 {
		fmt.Printf("Process Server \n")
	}

	fmt.Println("IP Addresses: ")
	for e := ip_addrs.Front(); e != nil; e = e.Next() {
		fmt.Printf("    %s\n", e.Value)
	}
}

func create_server_list_string() string {
	var serverList string
	i := 0

	for e := range data.servers {
		if i == 0 {
			serverList = serverList + e
		} else {
			serverList = serverList + DELIM + e
		}
		i = i + 1
	}
	return serverList

}

// sends a http request to an ipaddress with a route
func send_command_to_process(ipaddr string, route string, param string) string {
	var url string
	if len(param) > 0 {
		url = "http://" + ipaddr + ":8080" + "/" + route + "/" + param
	} else {
		url = "http://" + ipaddr + ":8080" + "/" + route
	}


	log.Println(url)
	resp, err := http.Get(url)
	if err != nil {
		log.Fatal(err)
	}
	contents, err := ioutil.ReadAll(resp.Body)
	defer resp.Body.Close()
	if err != nil {
		log.Fatal(err)
	}
	return string(contents)
}

// send command to all processes
func send_command_to_processes(processes map[string]bool, route string, mesg string) {
  //fmt.Println(route, mesg)
	for ipaddr := range processes {
		send_command_to_process(ipaddr, route, mesg)
	}
}

func exponential_wait(lambda float64) (interval int64) {

	unif := rand.Float64()
	//fmt.Println(unif, lambda)
	ln := math.Log(unif)
	_dur := -1000 * (ln) / lambda

	dur := int64(_dur)
	fmt.Println(unif, lambda, ln, dur)
	return dur
}

func create_server_string_to_C() string {
	var servers_str string = ""
	i := 0
	for key, _ := range data.servers {
		if i > 0 {
			servers_str += " "
		}
		servers_str += key
		i++
	}
	return servers_str
}

func GetLog(w http.ResponseWriter, r *http.Request) {
	// Open the file and dump it into the request as a byte array.
	log.Println("GET LOGS")
	buf, err := ioutil.ReadFile("logs/logs.txt")
	if err != nil {
		log.Fatal(err)
	}
	fmt.Fprintf(w, string(buf))
}

// GetProcessLogs will send HTTP GET request to the designated ip_address to reap the logs
func FlushLogs(w http.ResponseWriter, r *http.Request) {
	log.Println("INFO", "FLUSH LOGS")

	if data.processType == 3 {
		send_command_to_processes(data.readers, "FlushLog", "")
		send_command_to_processes(data.writers, "FlushLog", "")
		send_command_to_processes(data.servers, "FlushLog", "")
	}
}

// GetProcessLogs will send HTTP GET request to the designated ip_address to reap the logs
func GetLogs(w http.ResponseWriter, r *http.Request) {
	log.Println("GET LOGS")
}

// FlushProcess Log will send HTTP GET request to the designated ip_address to reap the logs
func FlushLog(w http.ResponseWriter, r *http.Request) {
	log.Println("INFO", "FLUSH LOGS")

	err := os.Truncate("logs/logs.txt", 0)
	if err != nil {
		log.Fatal(err)
	}
}

// StopReader will send a stop process message to all readers
func StopReaders(w http.ResponseWriter, r *http.Request) {
	log.Println("STOP READERS")
	for ipaddr := range data.readers {
		send_command_to_process(ipaddr, "StopProcess", "")
	}
}

// StopWriters will send a stop process message to all writers
func StopWriters(w http.ResponseWriter, r *http.Request) {
	log.Println("STOP WRITERS")
	for ipaddr := range data.writers {
		send_command_to_process(ipaddr, "StopProcess", "")
	}
}

// StopServers will send a stop process message to all servers
func StopServers(w http.ResponseWriter, r *http.Request) {
	log.Println("STOP SERVERS")
	for ipaddr := range data.servers {
		send_command_to_process(ipaddr, "StopProcess", "")
	}
}

func StopProcess(w http.ResponseWriter, r *http.Request) {
	active_chan <- false
	fmt.Println("StopProcess called")
	log.Println("INFO\tProcess Stopped")
}

func StartProcess(w http.ResponseWriter, r *http.Request) {
	active_chan <- true
	fmt.Println("StartProcess called")
	log.Println("INFO\tProcess Started")
}

func KillProcess(w http.ResponseWriter, r *http.Request) {
	log.Fatal("INFO\tKillProcess called... Shutting down.")
}

//////////////////////////////////////////////////////////////////

// StartReaders will send a start process message to all readers
func StartReaders(w http.ResponseWriter, r *http.Request) {
	log.Println("CMD STARTING READERS")
	for ipaddr := range data.readers {
		send_command_to_process(ipaddr, "StartProcess", "")
	}
}

// StartWriters will send a start process message to all writers
func StartWriters(w http.ResponseWriter, r *http.Request) {
	log.Println("STOPALLMACHINE")
	for ipaddr := range data.writers {
		send_command_to_process(ipaddr, "StartProcess", "")
	}
}

// StartServers will send a start process message to all servers
func StartServers(w http.ResponseWriter, r *http.Request) {
	log.Println("STOPALLMACHINE")
	for ipaddr := range data.servers {
		send_command_to_process(ipaddr, "StartProcess", "")
	}
}

// StartProcess will send a HTTP GET request to the designated ip address to start the process
func StartAProcess(w http.ResponseWriter, r *http.Request) {
	log.Println("START PROCESS")

	if data.processType == 3 {
		vars := mux.Vars(r)
		ipaddr := vars["ip"]
		send_command_to_process(ipaddr, "StartProcess", "")
	}
}

// StopProcess will send a HTTP GET request to the designated ip address to start the process
func StopAProcess(w http.ResponseWriter, r *http.Request) {
	log.Println("STOP PROCESS")

	if data.processType == 3 {
		vars := mux.Vars(r)
		ipaddr := vars["ip"]
		send_command_to_process(ipaddr, "StopProcess", "")
	}
}

// KillProcess will send a HTTP GET request to the designated ip address to kill the process
func KillAProcess(w http.ResponseWriter, r *http.Request) {
	log.Println("KILL PROCESS")

	if data.processType == 3 {
		vars := mux.Vars(r)
		ipaddr := vars["ip"]
		send_command_to_process(ipaddr, "KillProcess", "")
	}
}

// returns the set of servers
func GetServers(w http.ResponseWriter, r *http.Request) {
	log.Println("Get Servers")
	ipstr := ""
	for ip := range data.servers {
		ipstr += " " + ip
	}
	fmt.Fprintf(w, "%s", ipstr)
}

// this process registers the readers in the controller and then
// registers the servers in each readers
func SetServers(w http.ResponseWriter, r *http.Request) {
	log.Println("Set Servers")
	vars := mux.Vars(r)
	ip := vars["ip"]
	ips := strings.Split(ip, DELIM)

	for i := range ips {
		data.servers[ips[i]] = true
	}

	if data.processType == 3 {
		var clients map[string]bool = data.servers
		serverListStr := create_server_list_string()
		j := 0
		for ipaddr := range clients {
			send_command_to_process(ipaddr, "SetServers", serverListStr)
			name := "server_" + fmt.Sprintf("%d", j)
			send_command_to_process(ipaddr, "SetName", name)
			j = j + 1
		}
	}

}

// this process registers the writers in the controller and then
// registers the servers in each writer
func SetWriters(w http.ResponseWriter, r *http.Request) {
	log.Println("Set Writers")
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
	log.Println("SetReaders")
	vars := mux.Vars(r)
	ip := vars["ip"]
	ips := strings.Split(ip, DELIM)
	for i := range ips {
		data.readers[ips[i]] = true
	}
	fmt.Println("tyin to set readers", ip)

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

// set  name
func SetName(w http.ResponseWriter, r *http.Request) {
	vars := mux.Vars(r)
	ip := vars["param"]
	ips := strings.Split(ip, DELIM)

	if len(ips) != 1 {
		fmt.Printf("%s\n", ip)
		fmt.Printf("Expected 1 parameters, found %d\n", len(ips))
	}
	fmt.Println(ip)

	data.name = ip
}

// get name
func GetName(w http.ResponseWriter, r *http.Request) {
	fmt.Fprintf(w, "%s", data.name)
}

// returns the list of readers
func GetReaders(w http.ResponseWriter, r *http.Request) {
	log.Println("Get Readers")
	ipstr := ""
	for ip := range data.readers {
		ipstr += " " + ip
	}
	fmt.Fprintf(w, "%s", ipstr)
}

// returns the list of writers
func GetWriters(w http.ResponseWriter, r *http.Request) {
	log.Println("Get Writers")
	ipstr := ""
	for ip := range data.writers {
		ipstr += " " + ip
	}
	fmt.Fprintf(w, "%s", ipstr)
}

//set run id
func SetRunId(w http.ResponseWriter, r *http.Request) {
	log.Println("Set Run ID")
	vars := mux.Vars(r)
	id := vars["id"]

	data.run_id = id

	if data.processType == 3 {
		send_command_to_processes(data.readers, "SetRunId", data.run_id)
		send_command_to_processes(data.writers, "SetRunId", data.run_id)
		send_command_to_processes(data.servers, "SetRunId", data.run_id)
	}
}

//set randdom seed to a controller and then set random seed  each
// writer and readers
func SetSeed(w http.ResponseWriter, r *http.Request) {
	log.Println("Set Seed")
	vars := mux.Vars(r)
	ip := vars["seed"]
	ips := strings.Split(ip, DELIM)

	if len(ips) != 1 {
		fmt.Printf("%s\n", ip)
		fmt.Printf("Expected 1 parameters, found %d\n", len(ips))
	}

	s, _ := strconv.ParseInt(ips[0], 10, 64)
	data.rand_seed = s

	if data.processType == 3 {
		utilities.Set_random_seed(s)
		for ipaddr, _ := range data.readers {
			seed := rand.Intn(1000000000)
			seed_str := fmt.Sprintf("%d", seed)
			send_command_to_process(ipaddr, "SetName", seed_str)
		}

		for ipaddr, _ := range data.writers {
			seed := rand.Intn(1000000000)
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

	if len(ips) != 3 {
		fmt.Printf("%s\n", ip)
		fmt.Printf("Expected 1 parameters, found %d\n", len(ips))
	}
	name := ips[0]
	/*
	k, _ := strconv.ParseFloat(ips[0], 64)
	m, _ := strconv.ParseFloat(ips[0], 64)
	*/
	var m string
	k := ips[1]

	if len(ips) >2 {
	  m  = ips[2]
	  data.inter_read_wait_distribution = []string{name, k, m}
	} else {
	  data.inter_read_wait_distribution = []string{name, k}
	}

	log.Println("INFO\tSet Inter  Read Wait Time  Distribution\t" + ip)
	fmt.Println("INFO\tSet Inter  Read Wait Time  Distribution\t" + ip)

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

	if len(ips) != 3 {
		fmt.Printf("%s\n", ip)
		fmt.Printf("Expected 1 parameters, found %d\n", len(ips))
	}
	name := ips[0]


	log.Println("INFO\tSet Inter Write Wait Time  Distribution--\t" + ip)
	var m string
	k := ips[1]

	if len(ips) >2 {
	  m  = ips[2]
	  data.inter_write_wait_distribution = []string{name, k, m}
	} else {
	  data.inter_write_wait_distribution = []string{name, k}
	}

	log.Println("INFO\tSet Inter Write Wait Time  Distribution\t" + ip)
	if data.processType == 3 {
	 fmt.Println(data.inter_write_wait_distribution )
		send_command_to_processes(data.readers, "SetReadWriteDistribution", ip)
		send_command_to_processes(data.writers, "SetReadWriteDistribution", ip)
	}
}

func GetReadRate(w http.ResponseWriter, r *http.Request) {
	log.Println("Get Params")
	fmt.Fprintf(w, "%g\n", data.read_rate)
}

// set write to option disk or mem
func SetWriteTo(w http.ResponseWriter, r *http.Request) {
	log.Println("Set Write To")
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

	if err !=nil {
     data.file_size = 0.1
	}
  data.file_size = k
	log.Println("INFO\tSETTING FILE SIZE (KB) \t" + ip)

	if data.processType == 3 {
		  send_command_to_processes(data.writers, "SetFileSize", ip)
  }
}


func GetFileSize(w http.ResponseWriter, r *http.Request) {
	log.Println("Get Params")
	fmt.Fprintf(w, "%g\n", data.file_size)
}

//get seed
func GetSeed(w http.ResponseWriter, r *http.Request) {
	log.Println("Get Params")
	fmt.Fprintf(w, "%d\n", data.rand_seed)
}

func GetParams(w http.ResponseWriter, r *http.Request) {
	log.Println("INFO\tGet Params")
	fmt.Fprintf(w, "Algorithm\t%s\n", data.algorithm)
	fmt.Fprintf(w, "Random Seed\t%d\n", data.rand_seed)
	fmt.Fprintf(w, "File Size\t%g KB\n", data.file_size)
	fmt.Fprintf(w, "Run Id\t%s\n", data.run_id)
	fmt.Fprintf(w, "Port\t%s\n", data.port)
	fmt.Fprintf(w, "WriteTo\t%s\n", data.writeto)
  numparams := len(data.inter_read_wait_distribution)-1

	fmt.Fprintf(w, "Read Rate Distribution\t%s", data.inter_read_wait_distribution)
	for i:=1; i< numparams; i++ {
	   fmt.Fprintf(w, "\t%s", data.inter_read_wait_distribution[i])
	}
	fmt.Fprintf(w, "\n")


	fmt.Fprintf(w, "Write Rate Distribution\t%s", data.inter_write_wait_distribution)
	for i:=1; i< numparams; i++ {
	   fmt.Fprintf(w, "\t%s", data.inter_write_wait_distribution[i])
	}
	fmt.Fprintf(w, "\n")


	/*  fmt.Fprintf(w, "Read Rate\t%g\n", data.file_size)
	    fmt.Fprintf(w, "%d %g %g %g\n", data.rand_seed, data.file_size, data.read_rate, data.write_rate)
	*/
}

// KillSelf will end this server
func KillSelf(w http.ResponseWriter, r *http.Request) {
	fmt.Fprintf(w, "Controller going down...")
	defer log.Fatal("Controller Exiting")
}

// Panic/error handling file closing
func SetupLogging() (f *os.File) {
	// If the log directory doesnt exist, create it
	_, err := os.Stat("logs")
	if os.IsNotExist(err) {
		os.Mkdir("logs", 0700)
	}

	// open the log file
	f, err = os.OpenFile("logs/logs.txt", os.O_APPEND|os.O_CREATE|os.O_RDWR, 0666)
	if err != nil {
		fmt.Printf("error opening file: %v", err)
	}
	//defer f.Close()
	log.SetOutput(f)
	return
}

func InitializeParameters() {
	data.readers = make(map[string]bool)
	data.servers = make(map[string]bool)
	data.writers = make(map[string]bool)

	//data.inter_read_wait_distribution = []string{"erlang", "1", "1"}
	//data.inter_write_wait_distribution = []string{"erlang", "1", "1"}

	data.inter_read_wait_distribution = []string{"const", "100"}
	data.inter_write_wait_distribution = []string{"const", "100"}

	data.write_rate = 0.6
	data.read_rate = 0.6
	data.file_size = 0.1
	data.rand_seed = 1
	data.read_counter = 0
	data.write_counter = 0
	data.active = false
	data.port = "8081"

	//data.algorithm = "ABD"
	data.algorithm = "SODA"
	data.run_id = "DEFULT_RUN"
	data.writeto = "ram"

}


func rand_wait_time_const( distrib [] string) int64 {
     
	k, err := strconv.ParseInt(distrib[1], 10,  64)
	if err!=nil {
      return 100
	}

	return k
}

func rand_wait_time() int64 {

   var rand_dur int64
   if data.processType==0 {
       rand_dur = rand_wait_time_const(data.inter_read_wait_distribution)
	 }


   return rand_dur
}




//http server
func HTTP_Server() {
	// Setup HTTP functionality
	router := mux.NewRouter()
	fmt.Println("Running http server")

	router.HandleFunc("/SetRunId/{id}", SetRunId)
	router.HandleFunc("/SetWriteTo/{param}", SetWriteTo)

	router.HandleFunc("/GetName", GetName)
	router.HandleFunc("/SetName/{param}", SetName)

	router.HandleFunc("/GetLogs", GetLogs)
	router.HandleFunc("/FlushLogs", FlushLogs)

	router.HandleFunc("/GetLog", GetLog)
	router.HandleFunc("/FlushLog", FlushLog)

	router.HandleFunc("/StopAProcess/{ip}", StopAProcess)
	router.HandleFunc("/StartAProcess/{ip}", StartAProcess)
	router.HandleFunc("/KillAProcess/{ip}", KillAProcess)

	router.HandleFunc("/StopProcess", StopProcess)
	router.HandleFunc("/StartProcess", StartProcess)

	router.HandleFunc("/StopReaders", StopReaders)
	router.HandleFunc("/StopWriters", StopWriters)
	router.HandleFunc("/StopServers", StopServers)

	router.HandleFunc("/StartReaders", StartReaders)
	router.HandleFunc("/StartWriters", StartWriters)
	router.HandleFunc("/StartServers", StartServers)

	router.HandleFunc("/KillSelf", KillSelf)

	router.HandleFunc("/SetReaders/{ip:[0-9._]+}", SetReaders)
	router.HandleFunc("/SetWriters/{ip:[0-9._]+}", SetWriters)
	router.HandleFunc("/SetServers/{ip:[0-9._]+}", SetServers)

	router.HandleFunc("/GetReaders", GetReaders)
	router.HandleFunc("/GetWriters", GetWriters)
	router.HandleFunc("/GetServers", GetServers)

	router.HandleFunc("/SetSeed/{seed:[0-9]+}", SetSeed)
	router.HandleFunc("/GetSeed", GetSeed)
	router.HandleFunc("/GetParams", GetParams)

	router.HandleFunc("/SetReadRateDistribution/{param:[a-zA-Z0-9._]+}", SetReadRateDistribution)
	router.HandleFunc("/SetWriteRateDistribution/{param:[a-zA-Z0-9._]+}", SetWriteRateDistribution)

	router.HandleFunc("/SetFileSize/{size:[0-9.]+}", SetFileSize)
	router.HandleFunc("/GetFileSize", GetFileSize)
	http.ListenAndServe(":8080", router)

}
