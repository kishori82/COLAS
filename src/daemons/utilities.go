package daemons

/*
#cgo CFLAGS: -I../abd -I../sodaw -I../utilities -I..
#cgo LDFLAGS: -L../abd  -labd  -L../sodaw -lsodaw -lm
#include <abd_client.h>
#include <sodaw_reader.h>
#include <helpers.h>
#include <math.h>

#define SYMBOL_SIZE 1024

EncodeData *create_EncodeData(Parameters parameters) {
    unsigned int filesize = (unsigned int) (parameters.filesize_kb*1024);
    EncodeData *encoding_info  = (EncodeData *)malloc(sizeof(EncodeData));
    encoding_info->N = parameters.num_servers;
    encoding_info->K= ceil((float)parameters.num_servers + 1)/2;
    encoding_info->symbol_size = SYMBOL_SIZE;
    encoding_info->raw_data_size = filesize;
    encoding_info->num_blocks = ceil( (float)filesize/(encoding_info->K*SYMBOL_SIZE));
    encoding_info->algorithm= parameters.coding_algorithm;
    encoding_info->offset_index=0;

    return encoding_info;
}
char *get_servers_str(Parameters parameters) {
    int i;
    char *q = (char *) malloc( 16*parameters.num_servers*sizeof(char));

    char *p = q;
    strncpy(p, parameters.ipaddresses[0], strlen(parameters.ipaddresses[0]));
    p += strlen(parameters.ipaddresses[0]);
    for(i=1; i < parameters.num_servers; i++) {
        *p = ' ';
        p++;
        strncpy(p, parameters.ipaddresses[i], strlen(parameters.ipaddresses[i]));
        p += strlen(parameters.ipaddresses[i]);
    }
    *p = '\0';
    return q;
}


ClientArgs *create_ClientArgs(Parameters parameters) {

    char *servers_str = get_servers_str(parameters);

    ClientArgs *client_args  = (ClientArgs *)malloc(sizeof(ClientArgs));

    strcpy(client_args->client_id, parameters.server_id);
    strcpy(client_args->port, parameters.port);

    client_args->servers_str = (char *)malloc( (strlen(servers_str) +1)*sizeof(char));
    strcpy(client_args->servers_str, servers_str);

    return client_args;
}

void printParameters(Parameters parameters) {
    int i;

    printf("Parameters [C]\n");
    printf("\tName  \t\t\t\t: %s\n", parameters.server_id);;

    switch(parameters.processtype) {
    case reader:
        printf("\tprocesstype\t\t\t: %s\n", "reader");
        break;
    case writer:
        printf("\tprocesstype\t\t\t: %s\n", "writer");
        break;
    case server:
        printf("\tprocesstype\t\t\t: %s\n", "server");
        break;
    default:
        break;
    }

    printf("\tnum servers\t\t\t: %d\n", parameters.num_servers);
    for(i=0; i < parameters.num_servers; i++) {
        printf("\t\tserver %d\t\t: %s\n",i, parameters.ipaddresses[i] );
    }

    switch(parameters.algorithm) {
    case sodaw:
        printf("\talgorithm\t\t\t: %s\n", SODAW   );
        break;
    case abd:
        printf("\talgorithm\t\t\t: %s\n", ABD );
        break;
    default:
        break;
    }

    switch(parameters.coding_algorithm) {
    case full_vector:
        printf("\tcoding algorithm\t\t: %s\n", "RLNC"   );
        break;
    case reed_solomon:
        printf("\tcoding algorithm\t\t: %s\n", "REED-SOLOMON" );
        break;
    default:
        break;
    }
    printf("\tinter op wait (ms)\t\t: %d\n", parameters.wait);
    printf("\tfile size (KB)\t\t\t: %.2f\n", parameters.filesize_kb);
}

*/
import "C"

import (
	"fmt"
	"io/ioutil"
	"math/rand"
	"strconv"
	"strings"
	"time"
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

func getCPUSample() (idle, total uint64) {
	contents, err := ioutil.ReadFile("/proc/stat")
	if err != nil {
		return
	}
	lines := strings.Split(string(contents), "\n")
	for _, line := range lines {
		fields := strings.Fields(line)
		if fields[0] == "cpu" {
			numFields := len(fields)
			for i := 1; i < numFields; i++ {
				val, err := strconv.ParseUint(fields[i], 10, 64)
				if err != nil {
					fmt.Println("Error: ", i, fields[i], err)
				}
				total += val // tally up all the numbers to get total ticks
				if i == 4 {  // idle is the 5th field in the cpu line
					idle = val
				}
			}
			return
		}
	}
	return
}

func CpuUsage() float64 {
	idle0, total0 := getCPUSample()
	time.Sleep(3 * time.Second)
	idle1, total1 := getCPUSample()

	idleTicks := float64(idle1 - idle0)
	totalTicks := float64(total1 - total0)
	cpuUsage := 100 * (totalTicks - idleTicks) / totalTicks

	//fmt.Printf("CPU usage is %f%% [busy: %f, total: %f]\n", cpuUsage, totalTicks-idleTicks, totalTicks)
	return cpuUsage
}
