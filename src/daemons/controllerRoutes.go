package daemons

import (
	"fmt"
	"github.com/gorilla/mux"
	"log"
	"net/http"
)

// GetProcessLogs will send HTTP GET request to the designated ip_address to reap the logs
func FlushLogs(w http.ResponseWriter, r *http.Request) {
	log.Println("INFO\tFlushLogs")
	fmt.Println("INFO\tFlushLogs")

	if data.processType == 3 {
		send_command_to_processes(data.readers, "FlushLog", "")
		send_command_to_processes(data.writers, "FlushLog", "")
		send_command_to_processes(data.servers, "FlushLog", "")
	}
	fmt.Fprintf(w, "Logs Flushed")
}

// StartProcess will send a HTTP GET request to the designated ip address to start the process
func StartAProcess(w http.ResponseWriter, r *http.Request) {
	log.Println("INFO\tSTART PROCESS")
	fmt.Println("INFO\tSTART PROCESS")

	if data.processType == 3 {
		vars := mux.Vars(r)
		ipaddr := vars["ip"]
		send_command_to_process(ipaddr, "StartProcess", "")
	}
}

// StopProcess will send a HTTP GET request to the designated ip address to start the process
func StopAProcess(w http.ResponseWriter, r *http.Request) {
	log.Println("INFO\tSTOP PROCESS")
	fmt.Println("INFO\tSTOP PROCESS")

	if data.processType == 3 {
		vars := mux.Vars(r)
		ipaddr := vars["ip"]
		send_command_to_process(ipaddr, "StopProcess", "")
	}
}

// KillProcess will send a HTTP GET request to the designated ip address to kill the process
func KillAProcess(w http.ResponseWriter, r *http.Request) {
	log.Println("INFO\tKILL PROCESS")
	fmt.Println("INFO\tKILL PROCESS")

	if data.processType == 3 {
		vars := mux.Vars(r)
		ipaddr := vars["ip"]
		send_command_to_process(ipaddr, "KillProcess", "")
	}
}
