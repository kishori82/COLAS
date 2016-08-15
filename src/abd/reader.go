package abd_processes

import (
	"fmt"
	"log"
	"math"
	"math/rand"
  "container/list"
	"sync"
	"time"
	//utilities "../utilities/"
)


var (
	readers        []Reader
	writers        []Writer
	nodes          []Server
	nextId         int
	numReaders     int = 1
	numWriters     int = 1
	numNodes       int = 3
	numNodesToKill int = 0
	wg             sync.WaitGroup
	path           string = "./tmp/"
	maxDelay       int    = 100
)

type Reader struct {
	Id int
	ip_addresses []string
}

var active_chan  chan bool
var reset_chan  chan bool




/**
* Read a value from a set of nodes
* @param objectId the id of the object to be read
* @return value the value of the object
*/
func (r Reader) Read(objectId int) (value int) {
	PrintHeader("Reading")

	//	defer wg.Done()

	var stateVariables []StateVariable
	queryAll(objectId, &stateVariables)
	stateVariable := getMaxTag(stateVariables)

	sendAll(objectId, stateVariable)
	value = stateVariable.Value

	PrintFooter()
	return value
}

/**
* Queries all nodes for their state variables associated with an objectId
* @param objectId the id of the object for which we are querying
* @stateVariables the slice to which the state variable is added
*/
func queryAll(objectId int, stateVariables *[]StateVariable) {
	fmt.Println("Querying for Tag...")
	majority := int(math.Ceil(float64((len(nodes) + 1)) / float64(2)))
	c := make(chan int, len(nodes))
	for i := 0; i < len(nodes); i++ {
		fmt.Println("	Querying node", nodes[i].Id)
		go getStateVariable(nodes[i], objectId, stateVariables, c)
	}
	for j := 0; j < majority; j++ {
		<-c
		fmt.Println("	Received Tag", j)
	}
	fmt.Println("Majority received")
}

/**
* Gets the state variable stored in a single node for an object
* @param node the node being queried
* @param objectId the id of the object for which we are querying
* @param stateVariables the slice to which the state variable is added
* @param c the channel used to wait for all nodes to complete
*/
func getStateVariable(node Server, objectId int, stateVariables *[]StateVariable, c chan int) {
	if node.isAlive {
		r := rand.Intn(1000)
		time.Sleep(time.Duration(r) * time.Millisecond)
		fmt.Println("		Server", node.Id, "is alive")
		stateVariable := node.readFromCache(objectId)
		*stateVariables = append(*stateVariables, stateVariable)
		c <- 1
		return
	} else {
		fmt.Println("		Server", node.Id, "is dead")
	}
}

/**
* Sends the state variable with a certain id to all nodes
* to be written to storage and cache
* @param objectId the state variable's object id
* @param stateVariable the state variable to be sent
*/
func sendAll(objectId int, stateVariable StateVariable) {
	fmt.Println("Sending to nodes...")
	majority := int(math.Ceil(float64((len(nodes) + 1)) / float64(2)))
	c := make(chan int, len(nodes))
	for i := 0; i < len(nodes); i++ {
		fmt.Println("	Sending to node", nodes[i].Id, "the state variable", stateVariable)
		go sendStateVariable(nodes[i], objectId, stateVariable, c)
	}
	for j := 0; j < majority; j++ {
		<-c
		fmt.Println("	ACK Received", j)
	}
	fmt.Println("Majority received")
}

/**
* Sends the state variable to a certain node to be written
* to storage and cache
* @param node the node to which the state variable is sent
* @param objectId the state variable's objectId
* @param stateVariable the state variable to be sent
* @param c the channel used to wait for all nodes to complete
*/
func sendStateVariable(node Server, objectId int, stateVariable StateVariable, c chan int) {
	r := rand.Intn(maxDelay - 1)
	time.Sleep(time.Duration(r) * time.Millisecond)
	if !node.isAlive {
		fmt.Println("		Server", node.Id, "is dead")
		return
	}
	fmt.Println("		Server", node.Id, "is alive")
	node.writeToStorage(objectId, stateVariable)
	node.updateCache(objectId, stateVariable)
	c <- 1
}

/**
* Calculate the maximum tag among a slice of state variables
* @param stateVariables the slice of state variables
* @return maxStateVariable the state variable containing the
* maximum tag
*/
func getMaxTag(stateVariables []StateVariable) (maxStateVariable StateVariable) {
	fmt.Println("Calculating Maximum Tag")
	maxStateVariable = stateVariables[0]

	for i := 1; i < len(stateVariables); i++ {
		currValue := stateVariables[i].Tag.Value
		maxValue := maxStateVariable.Tag.Value
		currId := stateVariables[i].Tag.Id
		maxId := maxStateVariable.Tag.Id
		if (currValue > maxValue) || (currValue == maxValue && currId > maxId) {
			maxStateVariable = stateVariables[i]
		}
	}

	return maxStateVariable 
}


func Reader_process(ip_addrs *list.List ) {

	// This should become part of the standard init function later when we refactor...
  active_chan =make(chan bool, 10)
  fmt.Println("Starting reader\n");
	SetupLogging() 

	go HTTP_Server()

  for {
    select {
		    case active := <- active_chan:
           data.active = active
		    case active := <- reset_chan:
           data.active = active
					 data.read_counter=0
				default:
				  if data.active==true {
		        rand_wait :=exponential_wait(data.write_rate)
						time.Sleep( time.Duration(rand_wait)*1000 * time.Microsecond)
            log.Println("READ", data.name,  data.read_counter, rand_wait)
						data.read_counter +=1

					}else {
		        time.Sleep(5*1000000 * time.Microsecond)
				  }
		}
	}
}
