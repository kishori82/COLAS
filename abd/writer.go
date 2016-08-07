package abd

import (
//	"encoding/gob"
	"fmt"
	"container/list"
//	"math"
//	"math/rand"
//	"os"
//	"strconv"
//	"sync"
	"time"
)

/**************************************

			STRUCTS

**************************************/

type Writer struct {
	Id int
}

/**************************************

			MAIN FUNCTIONS

**************************************/

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
   for i:=0; i < 10; i++ {
	    time.Sleep(3*time.Second)
      fmt.Println("Write ",i)
	 }
}
