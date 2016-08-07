package abd

import (
     "math"
		 "fmt"
		 "container/list"

)

type Tag struct {
	Id    int
	Value int
}

type StateVariable struct {
	Tag   Tag
	Value int
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

  if proc_type==0 {
	   //fmt.Printf("Process Type: %d\n", proc_type)
	   fmt.Printf("Process Reader \n")
	}

  if proc_type==1 {
	   fmt.Printf("Process Writer \n")
	}
  if proc_type==2 {
	   fmt.Printf("Process Server \n")
	}


	fmt.Println("IP Addresses: ")
	for e := ip_addrs.Front(); e != nil; e = e.Next() {
		fmt.Printf("    %s\n", e.Value)
	}

}
