#ifndef __SERVER__
#define __SERVER__

#include "czmq.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>

typedef struct _SERVER_STATUS {
    float network_data, metadata_memory, data_memory, cpu_load;
    int time_point;
} SERVER_STATUS;

typedef struct _SERVER_ARGS {
    char *init_data;
    SERVER_STATUS *status;
} SERVER_ARGS;

int server_process(
               char *server_id, 
               char *port,
               char *init_data,
               SERVER_STATUS *status
             );



#endif
