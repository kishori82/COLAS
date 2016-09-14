#ifndef _SODAW_READER
#define _SODAW_READER

#include "czmq.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>

#include <algo_utils.h>
#include <base64.h>
#include "client.h"

//   write_value_phase(obj_name, writer_id,  op_num, sock_to_servers, servers, num_servers, port, payload, size, max_tag);

char *SODAW_read(char *obj_name, 
               char *writer_id, 
               unsigned int op_num,  
               //char *payload, 
               char *servers, 
               char *port,
               ENCODED_DATA *encoding_info
             );

#endif
