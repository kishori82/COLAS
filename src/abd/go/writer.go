package abd_processes

import (
//  "fmt"
  "container/list"
  "time"
	"log"
	"math/rand"
	utilities "../../utilities/"
)
type Writer struct {
  Id int
}

/**
* Write a value to the set of nodes
* @param objectId the id of the object to be written
* @param value the value to be written
*/
func (w Writer) Write(objectId, value int) {
  PrintHeader("Writing")

  defer wg.Done()

  var stateVariables []StateVariable
  queryAll(objectId, &stateVariables)
  stateVariable := getMaxTag(stateVariables)
  newTag := Tag{w.Id, stateVariable.Tag.Value + 1}
  newStateVariable := StateVariable{newTag, value}
  sendAll(objectId, newStateVariable)

  PrintFooter()
}


func Writer_process(ip_addrs *list.List) {
	// This should become part of the standard init function later when we refactor...
  active_chan =make(chan bool)
	SetupLogging() 
  log.Println("INFO", data.name, "Starting")

	// Keep running the server for now
	go HTTP_Server()

  for {
    select {
		    case active := <- active_chan:
           data.active = active
		    case active := <- reset_chan:
           data.active = active
					 data.write_counter=0
				default:
				  if data.active==true {
		        rand_wait :=1000*int64(rand.ExpFloat64()/data.write_rate)
						time.Sleep(time.Duration(rand_wait)*1000 * time.Microsecond)
						rand_data_file_size :=   int64(rand.ExpFloat64()/data.file_size)
						rand_data:= utilities.Generate_random_data(rand_data_file_size*1024)
            log.Println("WRITE", data.name, data.write_counter, rand_wait, len(rand_data))
						data.write_counter +=1

					}else {
		        time.Sleep(5*1000000 * time.Microsecond)
				  }
		}
	}
}
