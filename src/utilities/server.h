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
#include "../codes/rlnc_rs.h"

#include "client.h"
#include "../sodaw/sodaw_server.h"
#include "../abd/abd_server.h"

#define DEBUG_MODE  1

typedef struct _Server_Status {
    float network_data;
    float  metadata_memory;
    float data_memory;
    float  cpu_load;
    int time_point;
    float process_memory;
} Server_Status;

typedef struct _Server_Args {
    char *init_data;
    unsigned int init_data_size;

    char server_id[BUFSIZE];
    char *servers_str;
    char port[BUFSIZE];
    char port1[BUFSIZE];
    void *sock_to_servers;
    int num_servers;
    int symbol_size;
    unsigned int coding_algorithm; // 0 if full-vector and 1 is reed-solomon
    unsigned int K;
    unsigned int N;
    Server_Status *status;
} Server_Args;

int server_process(
    Server_Args *server_args,
    /*
                   char *server_id,
                   char *servers_str,
                   char *port,
                   char *init_data,
    */
    Server_Status *status
);


zhash_t *receive_message_frames_at_server(zmsg_t *msg, zlist_t *names) ;

int store_payload(zhash_t *object_hash,
                  char *obj_name, Tag tag,
                  zframe_t *payload,
                  enum INSERT_DATA_POLICY policy) ;

int create_object(zhash_t *object_hash, char *obj_name,
                  char *algorithm, char *init_data,
                  Server_Status *status) ;

void send_frames_at_server(zhash_t *frames, void *worker,
                           enum SEND_TYPE type, int n, ...) ;
#endif
