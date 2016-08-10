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
)


type Nodes struct{
    readers map[string]bool
    servers map[string]bool
    writers map[string]bool

    params map[string]float32
		writer_delta float32
		reader_delta float32
}

var data Nodes


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

  ips := strings.Split(ip,"s")
  
  for i := range ips {
	    fmt.Println("add ed")
			data.servers[ips[i]]=true
	}

	fmt.Println("added")
  for k := range data.servers {
     fmt.Println("map ", k)
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
  configClients(r, clients) 
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
  configClients(r, clients) 
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
func configClients(r *http.Request, clients map[string]bool)  {


	vars := mux.Vars(r)
	ip := vars["ip"]

  ips := strings.Split(ip,"s")
  

  for i := range ips {
			clients[ips[i]]=true
	}

  var serverList string 
	i:=0

  for e:= range data.servers {
	    if i==0 {
        serverList = serverList +  e
		  } else {
        serverList = serverList + "s" +  e
			}
			i = i + 1
	}

  for e:= range clients {
	   fmt.Println(e)
	   url := "http://" + e + ":8080" + "/SetServers/" + serverList

		 fmt.Println(url)
	   _, err := http.Get(url)
	   if err != nil {
		    log.Fatal(err)
	   }
	}
}


func SetParams(w http.ResponseWriter, r *http.Request)  {

	log.Println("Config Params")
	vars := mux.Vars(r)
	ip := vars["params"]
	
  ips := strings.Split(ip,"s")
	
	if len(ips)!=2 {
		 fmt.Printf("%s\n", ip)
	   fmt.Printf("Expected 2 parameters, found %d\n", len(ips))
	}

  f, _ := strconv.ParseFloat(ips[0], 32)
  data.params["read_freq"] = float32(f) 

  s, _ := strconv.ParseFloat(ips[1], 32)
  data.params["write_freq"] = float32(s) 

	fmt.Println("command", len(data.readers))

  commandAll(data.readers, ip)
  commandAll(data.writers, ip)

}


//send the command to all processes
func commandAll(processes map[string]bool, mesg string) {

   
  for e:= range processes {
	   fmt.Println(e)
	   url := "http://" + e + ":8080" + "/SetParams/" + mesg
		 fmt.Println(url)
	   _, err := http.Get(url)
	   if err != nil {
		    log.Fatal(err)
	   }
	}
}

func GetParams(w http.ResponseWriter, r *http.Request)  {
	log.Println("Get Params")
  fmt.Fprintf(w, "%g %g\n", data.params["read_freq"], data.params["write_freq"])
  fmt.Printf("%g %g\n", data.params["read_freq"], data.params["write_freq"])
}



// KillSelf will end this server
func KillSelf(w http.ResponseWriter, r *http.Request)  {
	fmt.Fprintf(w, "Controller going down...")
	defer log.Fatal("Controller Exiting")
}

func Controller_process() {
	log.Println("Starting Controller")

	router := mux.NewRouter()

	router.HandleFunc("/GetLogs/{ip}", GetLogs)
	router.HandleFunc("/StopProcess/{ip}", StopProcess)
	router.HandleFunc("/StartProcess/{ip}", StartProcess)
	router.HandleFunc("/KillProcess/{ip}", KillProcess)
	router.HandleFunc("/KillSelf", KillSelf)
	router.HandleFunc("/SetReaders/{ip:[0-9.s]+}", SetReaders)
	router.HandleFunc("/SetWriters/{ip:[0-9.s]+}", SetWriters)
	router.HandleFunc("/SetServers/{ip:[0-9.s]+}", SetServers)

	router.HandleFunc("/GetReaders", GetReaders)
	router.HandleFunc("/GetWriters", GetWriters)
	router.HandleFunc("/GetServers", GetServers)


  router.HandleFunc("/SetParams/{params:[0-9.s]+}", SetParams)
  router.HandleFunc("/GetParams", GetParams)


  data.readers = make(map[string]bool)
  data.servers = make(map[string]bool)
  data.writers = make(map[string]bool)

  data.params = make(map[string]float32)
	data.params["read_freq"] = 0.0
	data.params["write_freq"] = 0.0
   

  data.writer_delta = 10
  data.reader_delta = 2

	http.ListenAndServe(":8080", router)

}
