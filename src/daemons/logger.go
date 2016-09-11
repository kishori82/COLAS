package daemons

import (
	"fmt"
	"log"
	"os"
)

//LogParameters ...
func LogParameters() {
	log.Printf("INFO\tRUN_NAME\t%s\n", data.run_id)
	log.Printf("INFO\tPROCESS_NAME\t%s\n", data.name)
	log.Printf("INFO\tALGORITHM\t%s\n", data.algorithm)

	if data.algorithm == "SODAW" {
		if data.coding_algorithm == 0 {
			log.Printf("INFO\tCODING_ALGORITHM\tRLNC\n")
		}
		if data.coding_algorithm == 1 {
			log.Printf("INFO\tCODING_ALGORITHM\tREED_SOLOMON\n")
		}
	}

	log.Printf("INFO\tN\t%d\n", data.N)
	log.Printf("INFO\tK\t%d\n", data.K)
	log.Printf("INFO\tFILE_SIZE\t%f KB\n", data.file_size)
	log.Printf("INFO\tRAND_SEED\t%d\n", data.rand_seed)

	if len(data.inter_read_wait_distribution) == 2 {
		log.Printf("INFO\tINTER_READ_WAIT_DISTRIB\t%s\t%s\n",
			data.inter_read_wait_distribution[0], data.inter_read_wait_distribution[1])
	}
	if len(data.inter_read_wait_distribution) == 3 {
		log.Printf("INFO\tINTER_READ_WAIT_DISTRIB\t%s\t%s\t%s\n",
			data.inter_read_wait_distribution[0], data.inter_read_wait_distribution[1],
			data.inter_read_wait_distribution[2])
	}

	if len(data.inter_write_wait_distribution) == 2 {
		log.Printf("INFO\tINTER_WRITE_WAIT_DISTRIB\t%s\t%s\n",
			data.inter_write_wait_distribution[0], data.inter_write_wait_distribution[1])
	}
	if len(data.inter_write_wait_distribution) == 3 {
		log.Printf("INFO\tINTER_WRITE_WAIT_DISTRIB\t%s\t%s\t%s\n",
			data.inter_write_wait_distribution[0], data.inter_write_wait_distribution[1],
			data.inter_write_wait_distribution[2])
	}

	for key, _ := range data.servers {
		log.Printf("INFO\tSERVER\t%s\n", key)
	}

	for key, _ := range data.readers {
		log.Printf("INFO\tREADER\t%s\n", key)
	}

	for key, _ := range data.writers {
		log.Printf("INFO\tWRITER\t%s\n", key)
	}
}

//SetupLogging ...
func SetupLogging() (f *os.File) {
	// If the log directory doesnt exist, create it
	_, err := os.Stat("logs")
	if os.IsNotExist(err) {
		os.Mkdir("logs", 0700)
	}

	// open the log file
	f, err = os.OpenFile("logs/logs.txt", os.O_APPEND|os.O_CREATE|os.O_RDWR, 0666)
	if err != nil {
		fmt.Printf("error opening file: %v", err)
	}
	//defer f.Close()
	log.SetOutput(f)
	return
}
