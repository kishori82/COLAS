package utilities

import (
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
