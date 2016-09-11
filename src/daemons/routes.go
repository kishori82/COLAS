package daemons

import (
	"fmt"
	"github.com/gorilla/mux"
	"io/ioutil"
	"log"
	"math"
	"math/rand"
	"net/http"
	"os"
	"strings"
)

const (
	DELIM string = "_"
)

var data Params
var active_chan chan bool
var reset_chan chan bool

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

	fmt.Println(url)
	resp, err := http.Get(url)
	if err != nil {
		log.Println(err)
		return ""
	}

	defer resp.Body.Close()
	contents, err := ioutil.ReadAll(resp.Body)
	if err != nil {
		log.Println(err)
		return ""
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
	log.Println("INFO\tGetLog")
	fmt.Println("INFO\tGetLog")

	buf, err := ioutil.ReadFile("logs/logs.txt")
	if err != nil {
		log.Println(err)
		return
	}
	fmt.Fprintf(w, string(buf))
}

// GetProcessLogs will send HTTP GET request to the designated ip_address to reap the logs
func GetLogs(w http.ResponseWriter, r *http.Request) {
	log.Println("GET LOGS")
}

// FlushProcess Log will send HTTP GET request to the designated ip_address to reap the logs
func FlushLog(w http.ResponseWriter, r *http.Request) {
	log.Println("INFO\tFlushLog")
	fmt.Println("INFO\tFlushLog")

	err := os.Truncate("logs/logs.txt", 0)
	if err != nil {
		w.WriteHeader(http.StatusBadRequest)
		log.Println(err)
		return
	}

	fmt.Fprintf(w, "Log flushed")
}
func StopProcess(w http.ResponseWriter, r *http.Request) {
	log.Println("INFO\tStopProcess")
	fmt.Println("INFO\tStopProcess")

	active_chan <- false

	fmt.Fprintf(w, "Stopped")
}

func StartProcess(w http.ResponseWriter, r *http.Request) {
	log.Println("INFO\tStartProcess called")
	fmt.Println("INFO\tStartProcess called")

	active_chan <- true

	fmt.Fprintf(w, "Started")
}

func KillProcess(w http.ResponseWriter, r *http.Request) {
	log.Fatal("INFO\tKillProcess called... Shutting down.")
}

// StartReaders will send a start process message to all readers
func StartReaders(w http.ResponseWriter, r *http.Request) {
	clusterCommand("StartProcess", "readers")
}

// StartWriters will send a start process message to all writers
func StartWriters(w http.ResponseWriter, r *http.Request) {
	clusterCommand("StartProcess", "writers")
}

// StartServers will send a start process message to all servers
func StartServers(w http.ResponseWriter, r *http.Request) {
	clusterCommand("StartProcess", "servers")
}

// StopReader will send a stop process message to all readers
func StopReaders(w http.ResponseWriter, r *http.Request) {
	clusterCommand("StopReaders", "readers")
}

// StopWriters will send a stop process message to all writers
func StopWriters(w http.ResponseWriter, r *http.Request) {
	clusterCommand("StopWriters", "writers")
}

// StopServers will send a stop process message to all servers
func StopServers(w http.ResponseWriter, r *http.Request) {
	clusterCommand("StopServers", "servers")
}

func clusterCommand(url, daemons string) {
	log.Println("INFO\t" + url)
	fmt.Println("INFO\t" + url)

	switch {
	case daemons == "readers":
		for ipaddr := range data.readers {
			send_command_to_process(ipaddr, url, "")
		}
	case daemons == "writers":
		for ipaddr := range data.writers {
			send_command_to_process(ipaddr, url, "")
		}
	case daemons == "servers":
		for ipaddr := range data.servers {
			send_command_to_process(ipaddr, url, "")
		}
	default:
		log.Printf("Unacceptable daemons provided: %s\n", daemons)
	}
}

// set  name
func SetName(w http.ResponseWriter, r *http.Request) {
	log.Println("INFO\tSetName")
	fmt.Println("INFO\tSetName")

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
	log.Println("INFO\tGetName")
	fmt.Println("INFO\tGetName")

	fmt.Fprintf(w, "%s", data.name)
}

func getDaemons(url, daemons string) string {
	log.Println("INFO\t" + url)
	fmt.Println("INFO\t" + url)

	ipstr := ""
	switch {
	case daemons == "readers":
		for ip := range data.readers {
			ipstr += " " + ip
		}
	case daemons == "writers":
		for ip := range data.writers {
			ipstr += " " + ip
		}
	case daemons == "servers":
		for ip := range data.servers {
			ipstr += " " + ip
		}
	default:
		log.Println("Unacceptable daemon provided")
	}
	return ipstr
}

// returns the list of readers
func GetReaders(w http.ResponseWriter, r *http.Request) {
	ipstr := getDaemons("GetReaders", "readers")
	fmt.Fprintf(w, "%s", ipstr)
}

// returns the set of servers
func GetServers(w http.ResponseWriter, r *http.Request) {
	ipstr := getDaemons("GetServers", "servers")
	fmt.Fprintf(w, "%s", ipstr)
}

// returns the list of writers
func GetWriters(w http.ResponseWriter, r *http.Request) {
	ipstr := getDaemons("GetWriters", "writers")
	fmt.Fprintf(w, "%s", ipstr)
}

// KillSelf will end this server
func KillSelf(w http.ResponseWriter, r *http.Request) {
	log.Println("INFO\tController going down...")
	fmt.Println("INFO\tController going down...")

	fmt.Fprintf(w, "Controller going down...")
	defer log.Fatal("Controller Exiting")
}
