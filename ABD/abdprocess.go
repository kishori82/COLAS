package main

import (
	"encoding/gob"
	"fmt"
	"math"
	"math/rand"
	"os"
	"strconv"
	"sync"
	"time"
)

/**************************************

			VARIABLES

**************************************/

var (
	readers        []Reader
	writers        []Writer
	nodes          []Node
	nextId         int
	numReaders     int = 1
	numWriters     int = 1
	numNodes       int = 3
	numNodesToKill int = 0
	wg             sync.WaitGroup
	path           string = "./tmp/"
	maxDelay       int    = 100
)

/**************************************

			STRUCTS

**************************************/

type Tag struct {
	Id    int
	Value int
}

type StateVariable struct {
	Tag   Tag
	Value int
}

type Reader struct {
	Id int
}

type Writer struct {
	Id int
}

type Node struct {
	Id            int
	stateVariable StateVariable
	isAlive       bool
	filePath      string
	readCache     map[int]StateVariable
}

/**************************************

			MAIN FUNCTIONS

**************************************/

/**
 * Read a value from a set of nodes
 * @param objectId the id of the object to be read
 * @return value the value of the object
 */
func (r Reader) read(objectId int) (value int) {
	printHeader("Reading")

	defer wg.Done()

	var stateVariables []StateVariable
	queryAll(objectId, &stateVariables)
	stateVariable := getMaxTag(stateVariables)
	sendAll(objectId, stateVariable)
	value = stateVariable.Value

	printFooter()
	return
}

/**
 * Write a value to the set of nodes
 * @param objectId the id of the object to be written
 * @param value the value to be written
 */
func (w Writer) write(objectId, value int) {
	printHeader("Writing")

	defer wg.Done()

	var stateVariables []StateVariable
	queryAll(objectId, &stateVariables)
	stateVariable := getMaxTag(stateVariables)
	newTag := Tag{w.Id, stateVariable.Tag.Value + 1}
	newStateVariable := StateVariable{newTag, value}
	sendAll(objectId, newStateVariable)

	printFooter()
	return
}

/**************************************

			HELPER FUNCTIONS

**************************************/

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
func getStateVariable(node Node, objectId int, stateVariables *[]StateVariable, c chan int) {
	if node.isAlive {
		r := rand.Intn(1000)
		time.Sleep(time.Duration(r) * time.Millisecond)
		fmt.Println("		Node", node.Id, "is alive")
		stateVariable := node.readFromCache(objectId)
		*stateVariables = append(*stateVariables, stateVariable)
		c <- 1
		return
	} else {
		fmt.Println("		Node", node.Id, "is dead")
	}
}

/**
 * Reads the state variable associated with the id from cache
 * If the state variable is not in cache, it is read from storage
 * and stored in cache
 * @param id the id of the object for which we are querying
 * @return stateVariable the state variable associated with the id
 */
func (node Node) readFromCache(id int) (stateVariable StateVariable) {
	stateVariable, inCache := node.readCache[id]
	if !inCache {
		stateVariable = node.readFromStorage(id)
		node.readCache[id] = stateVariable
	}
	return
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
func sendStateVariable(node Node, objectId int, stateVariable StateVariable, c chan int) {
	r := rand.Intn(maxDelay - 1)
	time.Sleep(time.Duration(r) * time.Millisecond)
	if !node.isAlive {
		fmt.Println("		Node", node.Id, "is dead")
		return
	}
	fmt.Println("		Node", node.Id, "is alive")
	node.writeToStorage(objectId, stateVariable)
	node.updateCache(objectId, stateVariable)
	c <- 1
	return
}

/**
 * Writes a state variable, with its corresponding object id
 * to storage
 * @param id the object id of the state variable
 * @param stateVariable the state variable to be written
 */
func (node Node) writeToStorage(id int, stateVariable StateVariable) {
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
func (node Node) readFromStorage(id int) (stateVariable StateVariable) {
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
func (node Node) updateCache(id int, stateVariable StateVariable) {
	node.readCache[id] = stateVariable
	return
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

	return
}

/**
 * Add a new reader to the system
 */
func addReader() {
	readers = append(readers, Reader{nextId})
	nextId++
}

/**
 * Add a new writer to the system
 */
func addWriter() {
	writers = append(writers, Writer{nextId})
	nextId++
}

/**
 * Add a new node to the system
 */
func addNode() {
	filename := strconv.Itoa(nextId) + ".gob"
	fullFilename := path + filename
	storageFile, err := os.Create(fullFilename)
	defer storageFile.Close()
	if err != nil {
		fmt.Println("Error creating file", nextId)
	}
	newNode := Node{
		nextId,
		StateVariable{Tag{0, 0}, 0},
		true,
		fullFilename,
		make(map[int]StateVariable),
	}
	nodes = append(nodes, newNode)
	initialStateVariable := StateVariable{Tag{0, 0}, 0}
	stateVariables := make(map[int]StateVariable)
	stateVariables[1] = initialStateVariable
	enc := gob.NewEncoder(storageFile)
	_ = enc.Encode(stateVariables)
	nextId++
}

/**
 * Kill the node so that it doesn't respond to
 * any requests from readers or writers
 */
func (node *Node) kill() {
	node.isAlive = false
}

/**
 * Print out the components to the screen
 */
func printComponents() {
	printHeader("Components")
	fmt.Println("Readers")
	for i := 0; i < len(readers); i++ {
		fmt.Println("	", readers[i])
	}
	fmt.Println("Writers")
	for i := 0; i < len(writers); i++ {
		fmt.Println("	", writers[i])
	}
	fmt.Println("Nodes")
	for i := 0; i < len(nodes); i++ {
		fmt.Println("	", nodes[i])
		fmt.Println("      Cache:", nodes[i].readCache)
	}
	printFooter()
}

/**
 * Print out a header containing the title string
 * @param title the title to be contained in the header
 */
func printHeader(title string) {
	length := len(title)
	numSpaces := 22
	leftHalf := numSpaces + int(math.Ceil(float64(length)/2))
	rightHalf := numSpaces - int(math.Ceil(float64(length)/2))
	fmt.Println("***********************************************")
	fmt.Println("*                                             *")
	fmt.Print("*")
	fmt.Printf("%*s", int(leftHalf), title)
	fmt.Printf("%*s", (int(rightHalf) + 1), " ")
	fmt.Println("*")
	fmt.Println("*                                             *")
	fmt.Println("***********************************************")
}

/**
 * Print out a footer to the screen
 */
func printFooter() {
	fmt.Println("***********************************************")
}

/**************************************

				MAIN

**************************************/

func main() {
	rand.Seed(time.Now().UTC().UnixNano())
	r := rand.Intn(numNodes - 1)

	for i := 0; i < numReaders; i++ {
		addReader()
	}
	for i := 0; i < numWriters; i++ {
		addWriter()
	}
	for i := 0; i < numNodes; i++ {
		addNode()
	}

	for i := 0; i < numNodesToKill; i++ {
		for !nodes[r].isAlive {
			r = rand.Intn(numNodes - 1)
		}
		nodes[r].kill()
	}

	wg.Add(4)
	go writers[1].write(1, 7)
	go writers[0].write(2, 99)
	go writers[1].write(1, 8)
	go writers[0].write(1, 9)
	wg.Wait()

	wg.Add(2)
	printComponents()
	readers[0].read(1)
	readers[1].read(2)
	wg.Wait()

	printComponents()
}
