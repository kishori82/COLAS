//package main
package daemons

import (
	_ "fmt"
	"log"
)

func Controller_process() {
	f := SetupLogging()
	defer f.Close()
	log.Println("Starting Controller")
	data.processType = 3

	InitializeParameters()

	HTTP_Server()

}
