package main

import (
	daemons "./daemons/"
	"strings"
	"fmt"
	"math"
	"math/rand"
	"time"
	"os"
	"strconv"
)

/*
#cgo CFLAGS: -Iabd  -Isodaw  -Iutilities
#cgo LDFLAGS: -Labd  -labd -Lsodaw -lsodaw -lzmq  -Lcodes -lreed -Wl,-rpath=codes
#include "helpers.h"
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
	fmt.Println("Usage : abdprocess --process-type [0(reader), 1(writer), 2(server), 3(controller)] --filesize s [in KB]")
}

func main() {

	args := os.Args

	var parameters daemons.Parameters
	daemons.SetDefaultParameters(&parameters)

	// reader, writer and servers are 0, 1 and 2
	//  var proc_type string="--process-type"

	var usage_err bool = false

	for i := 1; i < len(args); i++ {
		if args[i] == "--process-type" {
			if i < len(args)+1 {
				_type, err := strconv.ParseUint(args[i+1], 10, 64)
				if err == nil {
					parameters.Processtype = _type
				} else {
					fmt.Println("Incorrect Process type [0-reader, 1-writer, 2-server, 3-controller] ", parameters.Processtype)
				}
				i++
			}
		} else if args[i] == "--filesize" {
			if i < len(args)+1 {
				_size, err := strconv.ParseFloat(args[i+1], 64)
				if err == nil {
					parameters.Filesize_kb = float64(_size * 1024)
				} else {
					fmt.Println("Incorrect file size type")
				}
			}
			i++
		} else if args[i] == "--ip" {
			if i < len(args)+1 {
				parameters.Ip_list= append(parameters.Ip_list, args[i+1])
			}
			i++
 	  } else if args[i] == "--algorithm" {
			if i < len(args)+1 {
				parameters.Algorithm = args[i+1]
			}
			i++
 	  } else if args[i] == "--wait" {
			if i < len(args)+1 {
				_wait, err := strconv.ParseUint(args[i+1], 10, 64)
				if err == nil {
					parameters.Wait = _wait
				}
			}
			i++
	 } else if args[i] == "--code" {
			if i < len(args)+1 {
				parameters.Coding_algorithm = args[i+1]
			}
			i++
		} else {
			fmt.Println("Unrecognized parameter : %s", args[i])
			usage_err = true
		}
	}

	if usage_err == true {
		usage()
		os.Exit(1)
	}


  parameters.Ipaddresses=strings.Join(parameters.Ip_list, " ")
  parameters.Num_servers = uint(len(parameters.Ip_list))

  s1 := rand.NewSource(time.Now().UnixNano())
  ran := rand.New(s1)

	if parameters.Processtype == 0 {
	  parameters.Server_id = "reader-" + strconv.Itoa(ran.Intn(10000000000))
		daemons.Reader_process(&parameters)
	} else if parameters.Processtype == 1 {
	  parameters.Server_id = "writer-" + strconv.Itoa(ran.Intn(10000000000))
		daemons.Writer_process(&parameters)
	} else if parameters.Processtype == 2 {
	  parameters.Server_id = "server-" + strconv.Itoa(ran.Intn(10000000000))
		daemons.Server_process(&parameters)
	} else if parameters.Processtype == 3 {
	  parameters.Server_id = "controller-" + strconv.Itoa(ran.Intn(10000000000))
		daemons.Controller_process()
	} else {
		fmt.Println("unknown type\n")
	}
	daemons.PrintFooter()
}

