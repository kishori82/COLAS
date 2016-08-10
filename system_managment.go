package main

import (
	"fmt"
	"log"
	"os/exec"
	"strings"
	"io/ioutil"
	"net/http"
	"regexp"
)

func main() {

	fmt.Println("Discovering readers, writers, servers and controllers locally\n")
	readers, writers, servers, controllers := get_ip_addresses()
	fmt.Println("Readers      :", readers)
	fmt.Println("Writers      :", writers)
	fmt.Println("Servers      :", servers)
	fmt.Println("Controller/s :", controllers)

	// send servers to controllers
	fmt.Println("Setting up the Servers\n")
	server_ips_stack := join_ips(servers)
	send_command_to_controllers(controllers, "SetServers", server_ips_stack)

	// send reader ids to controllers; and then controllers sends servers ids  to readers
	fmt.Println("Setting up the Readers\n")
	reader_ips_stack := join_ips(readers)
	send_command_to_controllers(controllers, "SetReaders", reader_ips_stack)

	// send writer ids to controllers; and then controllers sends servers ids  to writers
	fmt.Println("Setting up the writers\n")
	writer_ips_stack := join_ips(writers)
	send_command_to_controllers(controllers, "SetWriters", writer_ips_stack)

}

func send_command_to_controllers(controllers []string, route string, query_str string) {

	url := "http://" + controllers[0] + ":8080" + "/" + route + "/" + query_str
	fmt.Println(url)
	resp, err := http.Get(url)
	if err != nil {
		log.Fatal(err)
	}
	contents, err := ioutil.ReadAll(resp.Body)
	defer resp.Body.Close()
	if err != nil {
		log.Fatal(err)
	}
	fmt.Printf("%s", contents)
}

func join_ips(ips []string) string {

	var ipstr string = ""
	for i, ip := range ips {
		if i > 0 {
			//     fmt.Println(i, ip)
			ipstr = ipstr + "s" + ip
		} else {
			//      fmt.Println(i, ip)
			ipstr = ipstr + ip
		}
	}

	return ipstr
}

func get_ip_addresses() ([]string, []string, []string, []string) {
	re := regexp.MustCompile("\"IPAddress\": \"[0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+\"")
	ipaddrpat := regexp.MustCompile("[0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+")

	out, err := exec.Command("/usr/bin/docker", "ps", "-q").Output()
	var ipaddr string
	readers := make([]string, 0)
	writers := make([]string, 0)
	servers := make([]string, 0)
	controllers := make([]string, 0)

	if err != nil {
		log.Fatal(err)
	}
	ids := strings.Split(string(out), "\n")

	for _, id := range ids {
		if len(string(id)) == 0 {
			continue

		}
		out, err := exec.Command("/usr/bin/docker", "inspect", string(id)).Output()

		ipaddrs := re.FindAllString(string(out), 1)
		if len(ipaddrs) > 0 {
			_ipaddr := ipaddrpat.FindAllString(string(ipaddrs[0]), 1)
			ipaddr = string(_ipaddr[0])
			//   fmt.Println(ipaddr)
		}

		matched0, err := regexp.MatchString("colas_reader", string(out))
        if err != nil {
            log.Fatal(err)
        }
		matched1, err := regexp.MatchString("colas_writer", string(out))
        if err != nil {
            log.Fatal(err)
        }
		matched2, err := regexp.MatchString("colas_server", string(out))
        if err != nil {
            log.Fatal(err)
        }
		matched3, err := regexp.MatchString("colas_controller", string(out))
        if err != nil {
            log.Fatal(err)
        }

		if matched0 {
			//     fmt.Println("reader");
			readers = append(readers, ipaddr)
		} else if matched1 {
			//    fmt.Println("writer");
			writers = append(writers, ipaddr)
		} else if matched2 {
			//  fmt.Println("server");
			servers = append(servers, ipaddr)
		} else if matched3 {
			//  fmt.Println("controller");
			controllers = append(controllers, ipaddr)
		} else {

		}

		if err != nil {
			log.Fatal(err)
		}
	}
	return readers, writers, servers, controllers
}
