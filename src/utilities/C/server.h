#ifndef __SERVER__
#define __SERVER__

#include "czmq.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include "algo_utils.h"
#include "../../codes/rlnc_rs.h"

#define PAYLOADBUF_SIZE 100000
#define BUFSIZE 100


typedef struct _SERVER_STATUS {
    float network_data, metadata_memory, data_memory, cpu_load;
    int time_point;
} SERVER_STATUS;

typedef struct _SERVER_ARGS {
    char *init_data;
    char *server_id;
    char *servers_str;
    char *port;
    void *sock_to_servers; 
    int num_servers;
    int symbol_size;
    unsigned int coding_algorithm; // 0 if full-vector and 1 is reed-solomon
    int K;
    int N;
    SERVER_STATUS *status;
} SERVER_ARGS;



int server_process(
               char *server_id, 
               char *servers_str, 
               char *port,
               char *init_data,
               SERVER_STATUS *status
             );


int create_object(zhash_t *object_hash, char *obj_name, char *algorithm, char *init_data, SERVER_STATUS *status) ;

#endif
