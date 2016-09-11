//package main
package daemons

import (
	"log"
)

func Controller_process() {
	f := SetupLogging()
	defer f.Close()
	log.Println("Starting Controller")
	data.processType = 3

	InitializeParameters()
	LogParameters()

	HTTP_Server()

}
