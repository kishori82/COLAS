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
   void *data;
   int data_size;
   Tag *tag;
} RawData;

// this is the write tag value phase of ABD
void ABD_write_value_phase(
    char *obj_name,
    unsigned int op_num,
    zsock_t *sock_to_servers,
    unsigned int num_servers,
    RawData *raw_data,
    Tag max_tag   // for read it is max and for write it is new
);

#endif
