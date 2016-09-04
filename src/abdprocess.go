package main

import (
	daemons "./daemons/"
	"container/list"
	"fmt"
	"math"
	"os"
	"strconv"
)

/*
#cgo CFLAGS: -Iabd  -Isodaw  -Iutilities/C
#cgo LDFLAGS: -Labd  -labd -Lsodaw -lsodaw -lzmq  -Lcodes -lreed -Wl,-rpath=codes
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

//#cgo LDFLAGS: -Labd  -labd -Lsodaw -lsodaw -lzmq  -LZMQ/zmqlibs/lib -LZMQ/czmqlibs/lib/ -lczmq -Lcodes -lreed -Wl,-rpath=codes
func usage() {
	//fmt.Println("Usage : abdprocess --process-type [0(reader), 1(writer), 2(server), 3(controller)] --ip-address ip1 [ --ip-address ip2]")
	fmt.Println("Usage : abdprocess --process-type [0(reader), 1(writer), 2(server), 3(controller)] --init-file-size s [in KB]")
}

func main() {

	args := os.Args

	// reader, writer and servers are 0, 1 and 2
	//  var proc_type string="--process-type"
	var proc_type uint64
	var init_file_size float64 = 1
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
		} else if args[i] == "--init-file-size" {
			if i < len(args)+1 {
				_size, err := strconv.ParseFloat(args[i+1], 64)
				if err == nil {
					init_file_size = float64(_size * 1024)
				} else {
					fmt.Println("Incorrect file size type")
				}
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

	if proc_type == 0 {
		daemons.Reader_process(ip_addrs)
	} else if proc_type == 1 {
		daemons.Writer_process(ip_addrs)
	} else if proc_type == 2 {
		daemons.Server_process(init_file_size)
	} else if proc_type == 3 {
		daemons.Controller_process()
	} else {
		fmt.Println("unknown type\n")
	}
	daemons.PrintFooter()
}
