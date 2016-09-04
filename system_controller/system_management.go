package main

import (
	"errors"
	"fmt"
	"github.com/urfave/cli"
	"io/ioutil"
	"log"
	"net/http"
	"os"
	"os/exec"
	"regexp"
	"strings"
)

const delim string = "_"

func main() {
	type OPTS struct {
		start, stop, reset, setup, getlogs bool
		seed                               int64
		readrate, writerate, filesize      float64
		status                             bool
	}

	app := cli.NewApp()
	app.Version = "0.0.1"
	app.Name = "Controller for the atomicity setup"

	app.Commands = []cli.Command{
		{
			Name:   "setup",
			Usage:  "sets up reader, writers, servers and a controller",
			Action: setup,
		},
		{
			Name:   "status",
			Usage:  "shows the status of the system",
			Action: status,
		},
		{
			Name: "setread_dist",
			Usage: "the inter read wait time distribution (i) \"erlang k m\",\n" +
				"                   k is shape, and m is scale parameter (inverse of rate)\n" +
				"                   (ii) \"const k\" k is the inter read wait time in milliseconds\n",
			Action: setReadRateDistribution,
		},
		{
			Name: "setwrite_dist",
			Usage: "the inter write wait time distribution (i) \"erlang k m\",\n" +
				"                   k is shape, and m is scale parameter (inverse of rate)\n" +
				"                   (ii) \"const k\" k is the inter read wait time in milliseconds\n",
			Action: setWriteRateDistribution,
		},
		{
			Name:   "setwriteto",
			Usage:  "\"disk\" or \"mem\" either write to disk or memory",
			Action: setWriteTo,
		},
		{
			Name:   "setfile_size",
			Usage:  "const file size in KB",
			Action: setFileSize,
		},

		{
			Name:   "stop",
			Usage:  "pause reads and writes",
			Action: stop,
		},
		{
			Name:   "start",
			Usage:  "pause reads and writes",
			Action: start,
		},

		{
			Name:   "reset",
			Usage:  "reset the random generator",
			Action: reset,
		},
		{
			Name:   "setseed",
			Usage:  "set the random number seed provided",
			Action: setseed,
		},
		{
			Name:   "getparams",
			Usage:  "show the parameters set for the run",
			Action: getParameters,
		},
		{
			Name:   "setinitvaluesize",
			Usage:  "-----sets the intial value size stored in the data",
			Action: setInitialValueSize,
		},
		{
			Name:   "setrunid",
			Usage:  "sets the run id",
			Action: setRunId,
		},
		{
			Name:   "getlogs",
			Usage:  "get the logs from the clients and servers to the controller",
			Action: getlogs,
		},
		{
			Name:   "flushlogs",
			Usage:  "flush the logs from the clients and servers to the controller",
			Action: flushlogs,
		},
	}

	app.Run(os.Args)
}

// shows the status of the systems
func status(c *cli.Context) error {

	readers, writers, servers, controllers := getIPAddresses()

	fmt.Println("Number readers : ", len(readers))
	fmt.Println("Number writers : ", len(writers))
	fmt.Println("Number servers : ", len(servers))
	fmt.Println("Number controller : ", len(controllers))
	return nil
}

// stop the reader, writers and servers
func stop(c *cli.Context) error {

	if !isSystemRunning() {
		return nil
	}
	_, _, _, controllers := getIPAddresses()
	sendCommandToControllers(controllers, "StopReaders", "")
	sendCommandToControllers(controllers, "StopWriters", "")
	//sendCommandToControllers(controllers, "StopServers", "")
	return nil
}

// start the reader, writers and servers
func start(c *cli.Context) error {
	if !isSystemRunning() {
		return nil
	}

	_, _, _, controllers := getIPAddresses()
	sendCommandToControllers(controllers, "StartReaders", "")
	sendCommandToControllers(controllers, "StartWriters", "")
	//	sendCommandToControllers(controllers, "StartServers", "")
	return nil
}


//set the read rate
func setReadRateDistribution(c *cli.Context) error {
	if !isSystemRunning() {
		return nil
	}

	_, _, _, controllers := getIPAddresses()
	var rateParametersString string
	if len(c.Args()) == 0 {
		fmt.Println("No distribution provided")
		return nil
	}
	rateParametersString = c.Args().First()
	for i := 1; i < len(c.Args()); i++ {
		rateParametersString += "_" + c.Args()[i]
	}
	sendCommandToControllers(controllers, "SetReadRateDistribution", rateParametersString)

	return nil
}

//set the write rate distribution
func setWriteRateDistribution(c *cli.Context) error {
	if !isSystemRunning() {
		return nil
	}
	_, _, _, controllers := getIPAddresses()

	var rateParametersString string

	if len(c.Args()) == 0 {
		fmt.Println("No distribution provided")
		return nil
	}

	rateParametersString = c.Args().First()
	for i := 1; i < len(c.Args()); i++ {
		rateParametersString += "_" + c.Args()[i]
	}
	sendCommandToControllers(controllers, "SetWriteRateDistribution", rateParametersString)
	return nil
}

//set the write option either to disk or mem (virtual memory)
func setWriteTo(c *cli.Context) error {
	if !isSystemRunning() {
		return nil
	}
	_, _, _, controllers := getIPAddresses()

	writeToOption := c.Args().First()
	sendCommandToControllers(controllers, "SetWriteTo", writeToOption)
	return nil
}

//set the write rate
/*
func setWriteRate(c *cli.Context) error {
	if !isSystemRunning() {
		return nil
	}
	rate := c.Args().First()
	_, _, _, controllers := getIPAddresses()
	rateString := rate
	sendCommandToControllers(controllers, "SetWriteRate", rateString)
	return nil
}
*/

//set the file size
func setFileSize(c *cli.Context) error {
	if !isSystemRunning() {
		return nil
	}
	size := c.Args().First()
	_, _, _, controllers := getIPAddresses()
	sizeString := size

	sendCommandToControllers(controllers, "SetFileSize", sizeString)
	return nil
}

//set the seed
func setseed(c *cli.Context) error {
	if !isSystemRunning() {
		return nil
	}
	seed := c.Args().First()
	_, _, _, controllers := getIPAddresses()
	seedString := fmt.Sprintf("%d", seed)
	sendCommandToControllers(controllers, "SetSeed", seedString)

	sendCommandToControllers(controllers, "GetSeed", "")
	return nil

}

//set the seed
func getlogs(c *cli.Context) error {
	if !isSystemRunning() {
		return nil
	}

	folder := c.Args().First()
	_, err := os.Stat(folder)
	if os.IsNotExist(err) {
		err = os.Mkdir(folder, 0700)
		if err != nil {
			log.Fatal(err)
		}
	}

	readers, writers, servers, _ := getIPAddresses()

	// pulll logs from the readers
	for _, e := range readers {
		_name := getName(e)
		name := strings.TrimSpace(_name)
		logstr := getLogFile(e)
		f, err := os.Create(folder + "/" + name + ".log")
		if err != nil {
			log.Fatal(err)
		}
		_, err = f.WriteString(logstr)
		if err != nil {
			log.Fatal(err)
		}
		f.Close()
	}

	// pulll logs from the writers
	for _, e := range writers {
		_name := getName(e)
		name := strings.TrimSpace(_name)
		logstr := getLogFile(e)
		f, err := os.Create(folder + "/" + name + ".log")
		if err != nil {
			log.Fatal(err)
		}
		_, err = f.WriteString(logstr)
		if err != nil {
			log.Fatal(err)
		}
		f.Close()
	}

	// pulll logs from the servers
	for _, e := range servers {
		_name := getName(e)
		name := strings.TrimSpace(_name)
		logstr := getLogFile(e)
		f, err := os.Create(folder + "/" + name + ".log")
		if err != nil {
			log.Fatal(err)
		}
		_, err = f.WriteString(logstr)
		if err != nil {
			log.Fatal(err)
		}
		f.Close()
	}


	return nil
}

//set the seed
func flushlogs(c *cli.Context) error {
	if !isSystemRunning() {
		return nil
	}
	_, _, _, controllers := getIPAddresses()
	sendCommandToControllers(controllers, "FlushLogs", "")
	return nil
}

//reset the seed
func reset(c *cli.Context) error {
	if !isSystemRunning() {
		return nil
	}
	_, _, _, controllers := getIPAddresses()
	sendCommandToControllers(controllers, "Reset", "")
	return nil
}

func isSystemRunning() bool {
	_, _, _, controllers := getIPAddresses()

	if len(controllers) == 0 {
		fmt.Println("System does not seem to be up!")
		return false
	}
	return true
}

//get the parameters from controller
func getParameters(c *cli.Context) error {
	if !isSystemRunning() {
		return nil
	}
	_, _, _, controllers := getIPAddresses()

	params := sendCommandToControllers(controllers, "GetParams", "")
	fmt.Println(params)

	return nil
}

//set the intial value size
func setInitialValueSize(c *cli.Context) error {
	if !isSystemRunning() {
		return nil
	}
	fmt.Println("yet to implement")
	_, _, _, controllers := getIPAddresses()

	sendCommandToControllers(controllers, "SetInitialValueSize", "")
	return nil
}

//set the run Id
func setRunId(c *cli.Context) error {
	if !isSystemRunning() {
		return nil
	}
	runid := c.Args().First()
	_, _, _, controllers := getIPAddresses()

	sendCommandToControllers(controllers, "SetRunId", runid)
	return nil
}
func setup(c *cli.Context) error {
	fmt.Println("Discovering readers, writers, servers and controllers locally\n")
	readers, writers, servers, controllers := getIPAddresses()
	fmt.Println("Readers      :", readers)
	fmt.Println("Writers      :", writers)
	fmt.Println("Servers      :", servers)
	fmt.Println("Controller/s :", controllers)

	// setup servers to controllers
	fmt.Println("Setting up the Servers\n")

	// send servers to controllers
	serverIPsStack := joinIPs(servers)
	sendCommandToControllers(controllers, "SetServers", serverIPsStack)

	// send reader ids to controllers; and then controllers sends servers ids  to readers
	fmt.Println("Setting up the Readers\n")
	readerIPsStack := joinIPs(readers)
	sendCommandToControllers(controllers, "SetReaders", readerIPsStack)

	// send writer ids to controllers; and then controllers sends servers ids  to writers
	fmt.Println("Setting up the writers\n")
	writerIPsStack := joinIPs(writers)
	sendCommandToControllers(controllers, "SetWriters", writerIPsStack)

	if len(controllers) == 0 {
		return errors.New("Invalid number of controllers")
	}
	fmt.Println("Setting up the controller/s\n")
	return nil
}

func getLogFile(ip string) (logs string) {
	url := "http://" + ip + ":8080" + "/GetLog"
	resp, err := http.Get(url)

	if err != nil {
		log.Fatal(err)
	}

	contents, err := ioutil.ReadAll(resp.Body)
	if err != nil {
		log.Fatal(err)
	}
	logs = string(contents)

	return logs
}

func getName(ip string) (name string) {
	url := "http://" + ip + ":8080" + "/GetName"
	resp, err := http.Get(url)

	if err != nil {
		log.Fatal(err)
	}
	contents, err := ioutil.ReadAll(resp.Body)

	if err != nil {
		log.Fatal(err)
	}
	name = string(contents)
	return
}

func sendCommandToControllers(controllers []string, route string, queryStr string) (ack string) {
	var url string

	if len(controllers) < 1 {
		log.Fatal("No available controllers to send commands to. Exiting...")
	}

	if len(queryStr) > 0 {
		url = "http://" + controllers[0] + ":8080" + "/" + route + "/" + queryStr
	} else {
		url = "http://" + controllers[0] + ":8080" + "/" + route
	}

	resp, err := http.Get(url)
	if err != nil {
		log.Fatal(err)
	}
	contents, err := ioutil.ReadAll(resp.Body)

	defer resp.Body.Close()

	if err != nil {
		log.Fatal(err)
	}
	ack = string(contents)
	return ack
}

func joinIPs(ips []string) string {

	ipstr := ""
	for i, ip := range ips {
		if i > 0 {
			ipstr = ipstr + delim + ip
		} else {
			ipstr = ipstr + ip
		}
	}
	return ipstr
}

func getIPAddresses() ([]string, []string, []string, []string) {
	re := regexp.MustCompile("\"IPAddress\": \"[0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+\"")
	ipaddrpat := regexp.MustCompile("[0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+")

	_out, err := exec.Command("/usr/bin/docker", "ps", "-q").Output()

	out := string(_out)

	var readers []string
	var writers []string
	var servers []string
	var controllers []string

	if err != nil {
		log.Fatal(err)
	}
	ids := strings.Split(string(out), "\n")

	for _, id := range ids {
		if len(string(id)) == 0 {
			continue
		}

		_out, err := exec.Command("/usr/bin/docker", "inspect", string(id)).Output()
		out = string(_out)

		var ipaddr string
		ipaddrs := re.FindAllString(string(out), 1)
		if len(ipaddrs) > 0 {
			_ipaddr := ipaddrpat.FindAllString(string(ipaddrs[0]), 1)
			ipaddr = string(_ipaddr[0])
		}

		_matched0, err := regexp.MatchString("reader", string(out))
		matched0 := bool(_matched0)
		if err != nil {
			log.Fatal(err)
		}

		matched1, err := regexp.MatchString("writer", string(out))
		if err != nil {
			log.Fatal(err)
		}

		matched2, err := regexp.MatchString("server", string(out))
		if err != nil {
			log.Fatal(err)
		}

		matched3, err := regexp.MatchString("_controller_", string(out))
		if err != nil {
			log.Fatal(err)
		}

		if matched0 {
			readers = append(readers, ipaddr)
		} else if matched1 {
			writers = append(writers, ipaddr)
		} else if matched2 {
			servers = append(servers, ipaddr)
		} else if matched3 {
			controllers = append(controllers, ipaddr)
		}
	}
	return readers, writers, servers, controllers
}
