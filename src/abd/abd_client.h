#ifndef _ABD_CLIENT
#define _ABD_CLIENT

#include <czmq.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <algo_utils.h>
#include <base64.h>
#include "client.h"

typedef struct _RawData {
   char *data;
   int data_size;
} RawData;

// this is the write tag value phase of ABD
Tag write_value_phase(
    char *obj_name,
    char *writer_id,
    unsigned int op_num,
    zsock_t *sock_to_servers,
    char **servers,
    unsigned int num_servers,
    char *port,
    char *payload,
    int size,
    Tag max_tag   // for read it is max and for write it is new
);

#endif
