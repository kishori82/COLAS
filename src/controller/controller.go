//package main
package controller 

import (
	"log"
	"net/http"
	"github.com/gorilla/mux"
	"io/ioutil"
	"fmt"
	"strings"
	"strconv"
	utilities  "../utilities/GO"
	"math/rand"
	"os"
)


type Nodes struct{
    readers map[string]bool
    servers map[string]bool
    writers map[string]bool
/*
		writer_delta float32
	reader_delta float32
		*/
	  write_rate float64
		read_rate float64
		file_size float64
		rand_seed int64

}

var data Nodes
var DELIM string ="_"

// GetLogs will send the current logfiles back through the HTTP request.
func GetLogs(w http.ResponseWriter, r *http.Request) {

  // Open the file and dump it into the request as a byte array.
  buf, err := ioutil.ReadFile("logs/logs.txt") 
  if err != nil {
    log.Fatal(err)
  }
  fmt.Fprintf(w, string(buf))
}

/*

// GetLogs will send HTTP GET request to the designated ip_address to reap the logs
func GetLogs(w http.ResponseWriter, r *http.Request)  {
	log.Println("GETLOGS")

	vars := mux.Vars(r)
	ip := vars["ip"]

	url := "http://" + ip + ":8080" + "/GetLogs"
	resp, err := http.Get(url)
	if err != nil {
		log.Fatal(err)
	}

	contents, err := ioutil.ReadAll(resp.Body)
	defer resp.Body.Close()
	if err != nil {
		log.Fatal(err)
	}
	fmt.Printf("%s", contents)
}
*/

// GetProcessLogs will send HTTP GET request to the designated ip_address to reap the logs
func FlushLogs(w http.ResponseWriter, r *http.Request)  {
	log.Println("INFO", "FLUSH LOGS")

	for ip := range data.readers {
	    url := "http://" + ip + ":8080" + "/FlushLogs"
	    _, err := http.Get(url)
	    if err != nil {
		     log.Fatal(err)
	    }
	}

	for ip := range data.writers {
	    url := "http://" + ip + ":8080" + "/FlushLogs"
	    _, err := http.Get(url)
	    if err != nil {
		     log.Fatal(err)
	    }
	}
}




// GetProcessLogs will send HTTP GET request to the designated ip_address to reap the logs
func GetProcessLogs(w http.ResponseWriter, r *http.Request)  {
	log.Println("GETPROCESSLOGS")

	vars := mux.Vars(r)
	ip := vars["ip"]

	url := "http://" + ip + ":8080" + "/GetLogs"
	resp, err := http.Get(url)
	if err != nil {
		log.Fatal(err)
	}

	contents, err := ioutil.ReadAll(resp.Body)
	defer resp.Body.Close()
	if err != nil {
		log.Fatal(err)
	}
	fmt.Printf("%s", contents)
}


// StopProcess will send a HTTP GET request to the designated ip address to stop the process
func StopProcess(w http.ResponseWriter, r *http.Request)  {
	log.Println("STOPMACHINE")

	vars := mux.Vars(r)
	ip := vars["ip"]

	url := "http://" + ip + ":8080" + "/StopProcess"
	resp, err := http.Get(url)
	if err != nil {
		log.Fatal(err)
	}

	contents, err := ioutil.ReadAll(resp.Body)
	defer resp.Body.Close()
	if err != nil {
		log.Fatal(err)
	}
	fmt.Printf("%s", contents)
}


// StopReader will send a stop process message to all readers
func StopReaders(w http.ResponseWriter, r *http.Request)  {
	log.Println("STOPALLMACHINE")
  for ipaddr := range data.readers {
     send_command_to_process(ipaddr, "StopProcess") 
	}
}

// StopWriters will send a stop process message to all writers
func StopWriters(w http.ResponseWriter, r *http.Request)  {
	log.Println("STOPALLMACHINE")
  for ipaddr := range data.writers {
     send_command_to_process(ipaddr, "StopProcess") 
	}
}


// StopServers will send a stop process message to all servers
func StopServers(w http.ResponseWriter, r *http.Request)  {
	log.Println("STOPALLMACHINE")
  for ipaddr := range data.servers {
     send_command_to_process(ipaddr, "StopProcess") 
	}
}

// StartReaders will send a start process message to all readers
func StartReaders(w http.ResponseWriter, r *http.Request)  {
	log.Println("CMD STARTING READERS")
  for ipaddr := range data.readers {
     send_command_to_process(ipaddr, "StartProcess") 
	}
}

// StartWriters will send a start process message to all writers
func StartWriters(w http.ResponseWriter, r *http.Request)  {
	log.Println("STOPALLMACHINE")
  for ipaddr := range data.writers {
     send_command_to_process(ipaddr, "StartProcess") 
	}
}


// StartServers will send a start process message to all servers
func StartServers(w http.ResponseWriter, r *http.Request)  {
	log.Println("STOPALLMACHINE")
  for ipaddr := range data.servers {
     send_command_to_process(ipaddr, "StartProcess") 
	}
}


// sends a http request to an ipaddress with a route
func send_command_to_process(ipaddr string, route string)   {
     url:="http://" + ipaddr + ":8080" + "/" + route
		 log.Println(url)
		 resp, err:= http.Get(url)
		 if err!=nil {
        log.Fatal(err)
		 }
		 _, err = ioutil.ReadAll(resp.Body)
		 defer resp.Body.Close()
		 if err!=nil {
        log.Fatal(err)
		 }

}


// StartProcess will send a HTTP GET request to the designated ip address to start the process
func StartProcess(w http.ResponseWriter, r *http.Request)  {
	log.Println("STARTPROCESS")

	vars := mux.Vars(r)
	ip := vars["ip"]

	url := "http://" + ip + ":8080" + "/StartProcess"
	resp, err := http.Get(url)
	if err != nil {
		log.Fatal(err)
	}

	contents, err := ioutil.ReadAll(resp.Body)
	defer resp.Body.Close()
	if err != nil {
		log.Fatal(err)
	}
	fmt.Printf("%s", contents)
}

// KillProcess will send a HTTP GET request to the designated ip address to kill the process
func KillProcess(w http.ResponseWriter, r *http.Request)  {
	log.Println("KILLPROCESS")

	vars := mux.Vars(r)
	ip := vars["ip"]

	url := "http://" + ip + ":8080" + "/KillProcess"
	resp, err := http.Get(url)
	if err != nil {
		log.Fatal(err)
	}

	contents, err := ioutil.ReadAll(resp.Body)
	defer resp.Body.Close()
	if err != nil {
		log.Fatal(err)
	}
	fmt.Printf("%s", contents)
}


// this process registers the servers in the controller
func SetServers(w http.ResponseWriter, r *http.Request)  {

	log.Println("SetServers")

	vars := mux.Vars(r)
	ip := vars["ip"]

  ips := strings.Split(ip,DELIM)
  
  for i := range ips {
			data.servers[ips[i]]=true
	}
	j:=0
  for e:= range data.servers {
     tag := "server" + "_" + fmt.Sprintf("%d",j)

     url := "http://" + e + ":8080" + "/SetName/" + tag
		 fmt.Println(url)
	   _, err := http.Get(url)
	   if err != nil {
		    log.Fatal(err)
	   }
     j+=1
	}

}

// returns the set of servers 
func GetServers(w http.ResponseWriter, r *http.Request)  {
	log.Println("Get Servers")
  ipstr := ""
	for ip := range data.servers {
	   ipstr += " " + ip
	}
  fmt.Fprintf(w, "%s", ipstr)
}
// this process registers the readers in the controller and then 
// registers the servers in each readers
func SetReaders(w http.ResponseWriter, r *http.Request)  {
	log.Println("SetReaders")
  var clients map[string]bool  = data.readers

  fmt.Println("num readers", len(clients))
  configClients(r, clients, "reader") 
}

// returns the list of readers
func GetReaders(w http.ResponseWriter, r *http.Request)  {
	log.Println("Get Readers")
  ipstr := ""
	for ip := range data.readers {
	   ipstr += " " + ip
	}
  fmt.Fprintf(w, "%s", ipstr)
}


// this process registers the writers in the controller and then 
// registers the servers in each readers

func SetWriters(w http.ResponseWriter, r *http.Request)  {
	log.Println("SetReaders")
  var clients map[string]bool  = data.writers

  fmt.Println(len(clients))
  configClients(r, clients, "writer") 
}

// returns the list of writers
func GetWriters(w http.ResponseWriter, r *http.Request)  {
	log.Println("Get Writers")
  ipstr := ""
	for ip := range data.writers {
	   ipstr += " " + ip
	}
  fmt.Fprintf(w, "%s", ipstr)
}

// It can configure any client (reader/writer) with the set of servers
func configClients(r *http.Request, clients map[string]bool, client string)  {
	vars := mux.Vars(r)
	ip := vars["ip"]

  ips := strings.Split(ip,DELIM)
  
  for i := range ips {
			clients[ips[i]]=true
	}

  var serverList string 
	i:=0

  for e:= range data.servers {
	    if i==0 {
        serverList = serverList +  e
		  } else {
        serverList = serverList + DELIM +  e
			}
			i = i + 1
	}
  
	j:=0
  for e:= range clients {
	   url := "http://" + e + ":8080" + "/SetServers/" + serverList
	   _, err := http.Get(url)
	   if err != nil {
		    log.Fatal(err)
	   }

     tag := client + "_" + fmt.Sprintf("%d",j)
     url = "http://" + e + ":8080" + "/SetName/" + tag
		 fmt.Println(url)
	   _, err = http.Get(url)
	   if err != nil {
		    log.Fatal(err)
	   }
     j+=1
	}
}



//set seed
func SetSeed(w http.ResponseWriter, r *http.Request)  {
	log.Println("Set Seed")
	vars := mux.Vars(r)
	ip := vars["seed"]
  ips := strings.Split(ip,DELIM)
	
	log.Println("Set Seedo")
	
	if len(ips)!=1 {
		 fmt.Printf("%s\n", ip)
	   fmt.Printf("Expected 1 parameters, found %d\n", len(ips))
	}

  s, _ := strconv.ParseInt(ips[0],10, 64)
  data.rand_seed = s 

	utilities.Set_random_seed(s)

	for key, _ := range data.readers {
	    seed := rand.Intn(1000000000)
		  seed_str := fmt.Sprintf("%d", seed)
	    url := "http://" + key + ":8080" + "/SetSeed/" + seed_str
      fmt.Fprintf(w, "seed %s\n", url)
		  fmt.Println(url)
	      _, err := http.Get(url)
	     if err != nil {
		      log.Fatal(err)
	    }
	}


	for key, _ := range data.writers {
	    seed := rand.Intn(1000000000)
		  seed_str := fmt.Sprintf("%d", seed)
	    url := "http://" + key + ":8080" + "/SetSeed/" + seed_str
      fmt.Fprintf(w, "seed %s\n", url)
		  fmt.Println(url)
	      _, err := http.Get(url)
	     if err != nil {
		      log.Fatal(err)
	    }
	}
}

//set seed
func SetReadRate(w http.ResponseWriter, r *http.Request)  {
	log.Println("Set ReadRate")
	vars := mux.Vars(r)
	ip := vars["param"]
  ips := strings.Split(ip,DELIM)
	
	if len(ips)!=1 {
		 fmt.Printf("%s\n", ip)
	   fmt.Printf("Expected 1 parameters, found %d\n", len(ips))
	}
  s, _ := strconv.ParseFloat(ips[0], 64)
  data.read_rate = s 
  commandAll(data.readers, "SetReadRate", ip)
}


func GetReadRate(w http.ResponseWriter, r *http.Request)  {
	log.Println("Get Params")
  fmt.Fprintf(w, "%g\n", data.read_rate)
}


// set write rate
func SetWriteRate(w http.ResponseWriter, r *http.Request)  {
	log.Println("Set WriteRate")
	vars := mux.Vars(r)
	ip := vars["param"]
  ips := strings.Split(ip,DELIM)
	
	if len(ips)!=1 {
		 fmt.Printf("%s\n", ip)
	   fmt.Printf("Expected 1 parameters, found %d\n", len(ips))
	}
  s, _ := strconv.ParseFloat(ips[0], 64)
  data.write_rate = s 
  commandAll(data.writers, "SetWriteRate", ip)
}


// get read rate
func GetWriteRate(w http.ResponseWriter, r *http.Request)  {
	log.Println("Get Params")
  fmt.Fprintf(w, "%g\n", data.write_rate)
}

// set file size
func SetFileSize(w http.ResponseWriter, r *http.Request)  {
	log.Println("Set File Size")
	vars := mux.Vars(r)
	ip := vars["param"]
  ips := strings.Split(ip,DELIM)
	
	if len(ips)!=1 {
		 fmt.Printf("%s\n", ip)
	   fmt.Printf("Expected 1 parameters, found %d\n", len(ips))
	}
  s, _ := strconv.ParseFloat(ips[0], 64)
  data.file_size = s 
  commandAll(data.writers, "SetFileSize", ip)
}


// get file size
func GetFileSize(w http.ResponseWriter, r *http.Request)  {
	log.Println("Get Params")
  fmt.Fprintf(w, "%g\n", data.file_size)
}

//get seed
func GetSeed(w http.ResponseWriter, r *http.Request)  {
	log.Println("Get Params")
  fmt.Fprintf(w, "%d\n", data.rand_seed)
}


//send the command to all processes
//  commandAll(data.writers, ip)

func commandAll(processes map[string]bool, route string,  mesg string) {
  for e:= range processes {
	   url := "http://" + e + ":8080" + "/" + route + "/" + mesg
		 fmt.Println(url)
	   _, err := http.Get(url)
	   if err != nil {
		    log.Fatal(err)
	   }
	}
}

func GetParams(w http.ResponseWriter, r *http.Request)  {
	log.Println("Get Params")
  fmt.Fprintf(w, "%d %g %g %g\n", data.rand_seed, data.file_size, data.read_rate, data.write_rate)
}


// KillSelf will end this server
func KillSelf(w http.ResponseWriter, r *http.Request)  {
	fmt.Fprintf(w, "Controller going down...")
	defer log.Fatal("Controller Exiting")
}

// Panic/error handling file closing
func SetupLogging() (f *os.File) {
  // If the log directory doesnt exist, create it
  _, err := os.Stat("logs") 
  if 	os.IsNotExist(err) {
    os.Mkdir("logs", 0700)	
  }

  // open the log file
  f, err = os.OpenFile("logs/logs.txt", os.O_APPEND | os.O_CREATE | os.O_RDWR, 0666)
  if err != nil {
    fmt.Printf("error opening file: %v", err)
  }
  //defer f.Close()
  log.SetOutput(f)
  return 
}



func Controller_process() {
  f := SetupLogging() 

	log.Println("Starting Controller")
	router := mux.NewRouter()

	defer f.Close()

	//router.HandleFunc("/GetLogs/{ip}", GetProcessLogs)
	router.HandleFunc("/GetLogs", GetLogs)
	router.HandleFunc("/FlushLogs", FlushLogs)

	router.HandleFunc("/StopProcess/{ip}", StopProcess)
	router.HandleFunc("/StartProcess/{ip}", StartProcess)
	router.HandleFunc("/KillProcess/{ip}", KillProcess)

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

  router.HandleFunc("/SetReadRate/{param:[0-9.]+}", SetReadRate)
  router.HandleFunc("/GetReadRate", GetReadRate)

  router.HandleFunc("/SetWriteRate/{param:[0-9.]+}", SetWriteRate)
  router.HandleFunc("/GetWriteRate", GetWriteRate)

  router.HandleFunc("/SetFileSize/{param:[0-9.]+}", SetFileSize)
  router.HandleFunc("/GetFileSize", GetFileSize)

  data.readers = make(map[string]bool)
  data.servers = make(map[string]bool)
  data.writers = make(map[string]bool)

  data.write_rate = 2
  data.read_rate = 1
	data.file_size = 1
	data.rand_seed = 1

	http.ListenAndServe(":8080", router)

}
