package abd_processes

import (
	//	"encoding/gob"
	"fmt"
	//	"os"
)

func Server_process() {

	fmt.Println("Starting server\n")
	f := SetupLogging()
	defer f.Close()
	//go HTTP_Server()
	// Run the server for now
	HTTP_Server()

}
