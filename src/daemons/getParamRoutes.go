package daemons

import (
	"fmt"
	"log"
	"net/http"
)

func GetFileSize(w http.ResponseWriter, r *http.Request) {
	log.Println("INFO\tGet Params")
	fmt.Println("INFO\tGet Params")

	fmt.Fprintf(w, "%g\n", data.file_size)
}

//get seed
func GetSeed(w http.ResponseWriter, r *http.Request) {
	log.Println("INFO\tGet Params")
	fmt.Println("INFO\tGet Params")

	fmt.Fprintf(w, "%d\n", data.rand_seed)
}

func GetParams(w http.ResponseWriter, r *http.Request) {
	log.Println("INFO\tGet Params")
	fmt.Println("INFO\tGet Params")

	fmt.Fprintf(w, "Algorithm\t%s\n", data.algorithm)
	fmt.Fprintf(w, "Random Seed\t%d\n", data.rand_seed)
	fmt.Fprintf(w, "File Size\t%g KB\n", data.file_size)
	fmt.Fprintf(w, "Run Id\t%s\n", data.run_id)
	fmt.Fprintf(w, "Port\t%s\n", data.port)
	fmt.Fprintf(w, "WriteTo\t%s\n", data.writeto)
	numparams := len(data.inter_read_wait_distribution) - 1

	fmt.Fprintf(w, "Read Rate Distribution\t%s", data.inter_read_wait_distribution)
	for i := 1; i < numparams; i++ {
		fmt.Fprintf(w, "\t%s", data.inter_read_wait_distribution[i])
	}
	fmt.Fprintf(w, "\n")

	fmt.Fprintf(w, "Write Rate Distribution\t%s", data.inter_write_wait_distribution)
	for i := 1; i < numparams; i++ {
		fmt.Fprintf(w, "\t%s", data.inter_write_wait_distribution[i])
	}
	fmt.Fprintf(w, "\n")

	/*  fmt.Fprintf(w, "Read Rate\t%g\n", data.file_size)
	    fmt.Fprintf(w, "%d %g %g %g\n", data.rand_seed, data.file_size, data.read_rate, data.write_rate)
	*/
}
