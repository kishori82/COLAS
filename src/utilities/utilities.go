package utilities

import (
		 "math/rand"
		 "fmt"
 )
 
 
func Set_random_seed(seed int64) {
	rand.Seed(seed)
}


func Generate_random_data(size int64) [][]byte {
	var bytes [][]byte
	
	var i int64
	for  i = 0; i < size; i++ {
		number := rand.Intn(256)
		v := fmt.Sprintf("%x", number)
		b := []byte(v)
	//	log.Println(number, v, b)
		bytes = append(bytes, b)
	}
	return bytes
}
