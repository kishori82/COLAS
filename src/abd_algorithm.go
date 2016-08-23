package main

import (
	abd "./abd/go/"
	controller "./controller/"
	//	utilities "./utilities"
	"container/list"
	"fmt"
	"math"
	"os"
	"strconv"
	//	"sync"
)

/*
#cgo CFLAGS: -Iabd/C
#cgo LDFLAGS: -Labd/C  -labd_shared -lzmq -lczmq
#include <abd_client.h>
*/
import "C"

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

func usage() {
	fmt.Println("Usage : abdprocess --process-type [0(reader), 1(writer), 2(server), 3(controller)] --ip-address ip1 [ --ip-address ip2]")
}

/*
func print_configuration(proc_type uint64, ip_addrs *list.List) {

	fmt.Printf("Process Type: %d\n", proc_type)
	fmt.Println("IP Addresses: ")
	for e := ip_addrs.Front(); e != nil; e = e.Next() {
		fmt.Printf("    %s\n", e.Value)
	}

}
*/

func main() {

	args := os.Args

	fmt.Println(C.ABD_hello(5, 6))

	// reader, writer and servers are 0, 1 and 2
	//  var proc_type string="--process-type"
	var proc_type uint64
	ip_addrs := list.New()
	var usage_err bool = false

	for i := 1; i < len(args); i++ {
		if args[i] == "--process-type" {
			if i < len(args)+1 {
				_type, err := strconv.ParseUint(args[i+1], 10, 64)

				if err == nil {
					proc_type = _type
				} else {
					fmt.Println("Incorrect Process type [0-reader, 1-writer, 2-server, 3-controller] ", proc_type)
				}

				i++
			}
		} else if args[i] == "--ip-address" {
			if i < len(args)+1 {
				ip_addrs.PushBack(args[i+1])
			}
			i++
		} else {
			fmt.Println("Unrecognized parameter : %s", args[i])
			usage_err = true
		}
	}

	if usage_err == true {
		usage()
		os.Exit(9)
	}

	_ = proc_type

	if proc_type == 0 {
		abd.Reader_process(ip_addrs)
	} else if proc_type == 1 {
		abd.Writer_process(ip_addrs)
	} else if proc_type == 2 {
		abd.Server_process()
	} else if proc_type == 3 {
		controller.Controller_process()
	} else {
		fmt.Println("unknown type\n")
	}

	abd.PrintFooter()
	os.Exit(0)
}
