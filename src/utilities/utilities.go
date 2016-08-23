package utilities

import (
	//"fmt"
	"math/rand"
)

func Set_random_seed(seed int64) {
	rand.Seed(seed)
}

func Generate_random_data(rand_bytes []byte, size int64) error {

	for i := 0; i < (int)(size); i++ {
		v := rand.Uint32()
		rand_bytes[i] = (byte)(v)
	}

	return nil
}

/*

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
*/
