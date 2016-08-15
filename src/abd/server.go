package abd_processes

import (
	"encoding/gob"
	"fmt"
	"os"
)

type Server struct {
	Id            int
	stateVariable StateVariable
	isAlive       bool
	filePath      string
	readCache     map[int]StateVariable
}

/**
 * Reads the state variable associated with the id from cache
 * If the state variable is not in cache, it is read from storage
 * and stored in cache
 * @param id the id of the object for which we are querying
 * @return stateVariable the state variable associated with the id
 */
func (node Server) readFromCache(id int) (stateVariable StateVariable) {
	stateVariable, inCache := node.readCache[id]
	if !inCache {
		stateVariable = node.readFromStorage(id)
		node.readCache[id] = stateVariable
	}
	return
}

/**
 * Writes a state variable, with its corresponding object id
 * to storage
 * @param id the object id of the state variable
 * @param stateVariable the state variable to be written
 */
func (node Server) writeToStorage(id int, stateVariable StateVariable) {
	var stateVariables = make(map[int]StateVariable)

	file, err := os.Open(node.filePath)
	dec := gob.NewDecoder(file)
	err = dec.Decode(&stateVariables)
	defer file.Close()

	file, err = os.Create(node.filePath)
	defer file.Close()

	stateVariables[id] = stateVariable

	fmt.Println("State variables after update:", stateVariables)

	enc := gob.NewEncoder(file)
	err = enc.Encode(stateVariables)

	if err != nil {
		fmt.Println(err)
	}

	return
}

/**
 * Reads and returns the state variable associated with an
 * object id from storage
 * @param id the state variable's object id
 * @return stateVariable the state variable associated with
 * the object id
 */
func (node Server) readFromStorage(id int) (stateVariable StateVariable) {
	var stateVariables = make(map[int]StateVariable)

	file, err := os.Open(node.filePath)
	defer file.Close()

	dec := gob.NewDecoder(file)
	err = dec.Decode(&stateVariables)

	stateVariable = stateVariables[id]

	if err != nil {
		fmt.Println("Error", err)
	}

	return
}

/**
 * Update the node's cache to include an entry with the
 * state variable's object id and the state variable
 * @param id the state variable's object id
 * @param stateVariable the stateVariable to be inserted
 * into the cache or to be updated
 */
func (node Server) updateCache(id int, stateVariable StateVariable) {
	node.readCache[id] = stateVariable
	return
}

/**
 * Kill the node so that it doesn't respond to
 * any requests from readers or writers
 */
func (node *Server) kill() {
	node.isAlive = false
}


func Server_process() {

  fmt. Println("Starting server\n");
	f := SetupLogging() 
	defer f.Close()
	//go HTTP_Server()
	// Run the server for now
	HTTP_Server()

}
