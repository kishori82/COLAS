package abd_processes

import (
	utilities "../../utilities"
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
	Id    int
	Value int
}

type StateVariable struct {
	Tag   Tag
	Value int
}

type Nodes struct {
	readers map[string]bool
	servers map[string]bool
	writers map[string]bool

	write_rate float64
	read_rate  float64
	file_size  float64
	rand_seed  int64

	active bool
	port   string

	read_counter, write_counter int64
	name                        string
}

var data Nodes
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

func SetServers(w http.ResponseWriter, r *http.Request) {
	log.Println("SetServers")
	vars := mux.Vars(r)
	ip := vars["ip"]
	ips := strings.Split(ip, DELIM)

	for i := range ips {
		data.servers[ips[i]] = true
	}
}

func SetParams(w http.ResponseWriter, r *http.Request) {

	log.Println("Config Params")
	vars := mux.Vars(r)
	ip := vars["params"]
	ips := strings.Split(ip, DELIM)

	if len(ips) != 4 {
		fmt.Println(ip)
		fmt.Printf("Expect 2 parameters, found %d", len(ips))
	}

	s, _ := strconv.ParseInt(ips[0], 10, 64)
	data.rand_seed = s

	f, _ := strconv.ParseFloat(ips[1], 64)
	data.file_size = f

	f, _ = strconv.ParseFloat(ips[2], 64)
	data.read_rate = float64(f)

	f, _ = strconv.ParseFloat(ips[3], 64)
	data.write_rate = float64(f)

}

func GetParams(w http.ResponseWriter, r *http.Request) {
	log.Println("Get Params")
	fmt.Fprintf(w, "%d %g %g %g\n", data.rand_seed, data.file_size, data.read_rate, data.write_rate)
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

// FlushLogs will send the current logfiles back through the HTTP request.
func FlushLogs(w http.ResponseWriter, r *http.Request) {
	// Open the file and dump it into the request as a byte array.
	err := os.Truncate("logs/logs.txt", 0)

	if err != nil {
		log.Fatal(err)
	}
}

// GetLogs will send the current logfiles back through the HTTP request.
func GetLogs(w http.ResponseWriter, r *http.Request) {

	// Open the file and dump it into the request as a byte array.
	buf, err := ioutil.ReadFile("logs/logs.txt")
	if err != nil {
		log.Fatal(err)
	}
	fmt.Fprintf(w, string(buf))
}

// GetServers will send the current list of servers through the HTTP request.
func GetServers(w http.ResponseWriter, r *http.Request) {
	fmt.Println("GetServers called")
	// Open the file and dump it into the request as a byte array.
	buf := ""

	for e := range data.servers {
		buf = buf + " " + e
		fmt.Println(buf)
	}

	fmt.Fprintf(w, buf)
}

func StopProcess(w http.ResponseWriter, r *http.Request) {
	active_chan <- false
	fmt.Println("StopProcess called")
}

func StartProcess(w http.ResponseWriter, r *http.Request) {
	active_chan <- true
	fmt.Println("StartProcess called")
}

func KillProcess(w http.ResponseWriter, r *http.Request) {
	log.Fatal("KillProcess called... Shutting down.")
}

//set seed
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

	utilities.Set_random_seed(s)
}

func GetSeed(w http.ResponseWriter, r *http.Request) {
	log.Println("Get Params")
	fmt.Fprintf(w, "%d\n", data.rand_seed)
}

//set seed
func SetReadRate(w http.ResponseWriter, r *http.Request) {
	log.Println("Set ReadRate")
	vars := mux.Vars(r)
	ip := vars["param"]
	ips := strings.Split(ip, DELIM)

	if len(ips) != 1 {
		fmt.Printf("%s\n", ip)
		fmt.Printf("Expected 1 parameters, found %d\n", len(ips))
	}

	s, _ := strconv.ParseFloat(ips[0], 64)
	data.read_rate = s

}

func GetReadRate(w http.ResponseWriter, r *http.Request) {
	log.Println("Get Params")
	fmt.Fprintf(w, "%g\n", data.read_rate)
}

// set write rate
func SetWriteRate(w http.ResponseWriter, r *http.Request) {
	log.Println("Set WriteRate")
	vars := mux.Vars(r)
	ip := vars["param"]
	ips := strings.Split(ip, DELIM)

	if len(ips) != 1 {
		fmt.Printf("%s\n", ip)
		fmt.Printf("Expected 1 parameters, found %d\n", len(ips))
	}
	s, _ := strconv.ParseFloat(ips[0], 64)
	data.write_rate = s
}

// get read rate
func GetWriteRate(w http.ResponseWriter, r *http.Request) {
	log.Println("Get Params")
	fmt.Fprintf(w, "%g\n", data.write_rate)
}

// set file size
func SetFileSize(w http.ResponseWriter, r *http.Request) {
	log.Println("Set File Size")
	vars := mux.Vars(r)
	ip := vars["param"]
	ips := strings.Split(ip, DELIM)

	if len(ips) != 1 {
		fmt.Printf("%s\n", ip)
		fmt.Printf("Expected 1 parameters, found %d\n", len(ips))
	}
	s, _ := strconv.ParseFloat(ips[0], 64)
	data.file_size = s
}

// get file size
func GetFileSize(w http.ResponseWriter, r *http.Request) {
	log.Println("Get Params")
	fmt.Fprintf(w, "%g\n", data.file_size)
}

// set file name
func SetName(w http.ResponseWriter, r *http.Request) {
	vars := mux.Vars(r)
	ip := vars["param"]
	ips := strings.Split(ip, DELIM)

	if len(ips) != 1 {
		fmt.Printf("%s\n", ip)
		fmt.Printf("Expected 1 parameters, found %d\n", len(ips))
	}
	data.name = ip
}

// get name
func GetName(w http.ResponseWriter, r *http.Request) {
	fmt.Fprintf(w, "%s", data.name)
}

func InitializeParameters() {
	data.readers = make(map[string]bool)
	data.servers = make(map[string]bool)
	data.writers = make(map[string]bool)

	data.write_rate = 0.1
	data.read_rate = 0.1
	data.file_size = 10
	data.rand_seed = 1
	data.read_counter = 0
	data.write_counter = 0
	data.active = false

	data.port = "8081"
}

//http server
func HTTP_Server() {
	// Setup HTTP functionality

	router := mux.NewRouter()

	fmt.Println("Running http server")

	router.HandleFunc("/GetLogs", GetLogs)
	router.HandleFunc("/FlushLogs", FlushLogs)

	router.HandleFunc("/StopProcess", StopProcess)
	router.HandleFunc("/StartProcess", StartProcess)

	router.HandleFunc("/KillProcess", KillProcess)
	router.HandleFunc("/SetServers/{ip:[0-9._]+}", SetServers)
	router.HandleFunc("/GetServers", GetServers)

	router.HandleFunc("/SetParams/{params:[0-9._]+}", SetParams)

	router.HandleFunc("/SetSeed/{seed:[0-9]+}", SetSeed)
	router.HandleFunc("/GetSeed", GetSeed)

	router.HandleFunc("/GetParams", GetParams)

	router.HandleFunc("/SetReadRate/{param:[0-9.]+}", SetReadRate)
	router.HandleFunc("/GetReadRate", GetReadRate)

	router.HandleFunc("/SetWriteRate/{param:[0-9.]+}", SetWriteRate)
	router.HandleFunc("/GetWriteRate", GetWriteRate)

	router.HandleFunc("/SetFileSize/{param:[0-9.]+}", SetFileSize)
	router.HandleFunc("/GetFileSize", GetFileSize)

	router.HandleFunc("/SetName/{param:[a-zA-Z0-9_]+}", SetName)
	router.HandleFunc("/GetName", GetName)

	http.ListenAndServe(":8080", router)
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
