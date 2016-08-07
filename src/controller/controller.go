//package main
package controller 

import (
	"log"
	"net/http"
	"github.com/gorilla/mux"
	"io/ioutil"
	"fmt"
)

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

	http.ListenAndServe(":8080", router)
}
