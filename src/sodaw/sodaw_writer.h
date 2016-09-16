#ifndef _SODAW_WRITER
#define _SODAW_WRITER

#include "czmq.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>

#include <algo_utils.h>
#include <base64.h>
#include "../utilities/client.h"
#include "../codes/rlnc_rs.h"

//   write_value_phase(obj_name, writer_id,  op_num, sock_to_servers, servers, num_servers, port, payload, size, max_tag);

bool SODAW_write(char *obj_name, 
               unsigned int op_num,  
               char *payload, 
               unsigned int size, 
               EncodeData *encoded_data, 
               ClientArgs *client_args
             ); 

#endif
