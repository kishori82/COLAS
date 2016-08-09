//package main
package controller 

import (
	"log"
	"net/http"
	"github.com/gorilla/mux"
	"io/ioutil"
	"fmt"
	"strings"
)


type Nodes struct{
    readers map[string]bool
    servers map[string]bool
    writers map[string]bool
}

var data Nodes


// GetLogs will send HTTP GET request to the designated ip_address to reap the logs
func GetLogs(w http.ResponseWriter, r *http.Request)  {
	log.Println("GETLOGS")

	vars := mux.Vars(r)
	ip := vars["ip"]

	url := "http://" + ip + ":8081" + "/GetLogs"
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

	url := "http://" + ip + ":8081" + "/StopProcess"
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

	url := "http://" + ip + ":8081" + "/StartProcess"
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

	url := "http://" + ip + ":8081" + "/KillProcess"
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
func ConfigServers(w http.ResponseWriter, r *http.Request)  {
  if data.servers == nil {
     data.servers = make(map[string]bool)
	}

	log.Println("ConfigServers")

	vars := mux.Vars(r)
	ip := vars["ip"]

  ips := strings.Split(ip,"s")
  
  for i := range ips {
			data.servers[ips[i]]=true
	}

	fmt.Println("servers added controller")
  for k := range data.servers {
     fmt.Println("    ", k)
	}

}

// this process registers the readers in the controller and then 
// registers the servers in each readers
func ConfigReaders(w http.ResponseWriter, r *http.Request)  {
  if data.readers == nil {
     data.readers = make(map[string]bool)
	}

	log.Println("ConfigReaders")

	vars := mux.Vars(r)
	ip := vars["ip"]

  ips := strings.Split(ip,"s")
  
  for i := range ips {
			data.readers[ips[i]]=true
	}


  var serverList string 
	i:=0

  for e:= range data.readers {
	    if i==0 {
        serverList = serverList +  e
		  } else {
        serverList = serverList + e
			}
			i = i + 1
	}

  for e:= range data.readers {
	   fmt.Println(e)
	   url := "http://" + e + ":8081" + "/ConfigServers/" + serverList

		 fmt.Println(url)
	   _, err := http.Get(url)
	   if err != nil {
		    log.Fatal(err)
	   }
	   //fmt.Printf("%s", contents)
	}
}

// this process registers the writers in the controller and then 
// registers the servers in each writer
func ConfigWriters(w http.ResponseWriter, r *http.Request)  {
  if data.writers == nil {
     data.writers = make(map[string]bool)
	}

	log.Println("ConfigWriter")

	vars := mux.Vars(r)
	ip := vars["ip"]

  ips := strings.Split(ip,"s")
  
  for i := range ips {
			data.writers[ips[i]] =true
	}

}


/*


	url := "http://" + ip + ":8081" + "/ConfigServers"
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
	*/


// KillSelf will end this server
func KillSelf(w http.ResponseWriter, r *http.Request)  {
	fmt.Fprintf(w, "Controller going down...")
	defer log.Fatal("Controller Exiting")
}

//func main() {
func Controller_process() {
	log.Println("Starting Controller")

	router := mux.NewRouter()

	router.HandleFunc("/GetLogs/{ip}", GetLogs)
	router.HandleFunc("/StopProcess/{ip}", StopProcess)
	router.HandleFunc("/StartProcess/{ip}", StartProcess)
	router.HandleFunc("/KillProcess/{ip}", KillProcess)
	router.HandleFunc("/KillSelf", KillSelf)
	router.HandleFunc("/ConfigReaders/{ip:[0-9.s]+}", ConfigReaders)
	router.HandleFunc("/ConfigWriters/{ip:[0-9.s]+}", ConfigWriters)
	router.HandleFunc("/ConfigServers/{ip:[0-9.s]+}", ConfigServers)

	http.ListenAndServe(":8080", router)
}
