package daemons

import (
	"container/list"
	"fmt"
	"math"
	"strconv"
)

type Tag struct {
	z  int
	id string
}

func PrintHeader(title string) {
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
func PrintFooter() {
	fmt.Println("***********************************************")
}

func Print_configuration(proc_type uint64, ip_addrs *list.List) {

	if proc_type == 0 {
		//fmt.Printf("Process Type: %d\n", proc_type)
		fmt.Printf("Process Reader \n")
	}

	if proc_type == 1 {
		fmt.Printf("Process Writer \n")
	}
	if proc_type == 2 {
		fmt.Printf("Process Server \n")
	}

	fmt.Println("IP Addresses: ")
	for e := ip_addrs.Front(); e != nil; e = e.Next() {
		fmt.Printf("    %s\n", e.Value)
	}
}

func rand_wait_time() int64 {

	var rand_dur int64
	if data.processType == 0 {
		rand_wait_time_const := func(distrib []string) int64 {
			k, err := strconv.ParseInt(distrib[1], 10, 64)
			if err != nil {
				return 100
			}
			return k
		}
		rand_dur = rand_wait_time_const(data.inter_read_wait_distribution)
	}

	return rand_dur
}
