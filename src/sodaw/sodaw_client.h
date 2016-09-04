#ifndef _SODAW_CLIENT
#define _SODAW_CLIENT

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

bool SODAW_write(char *obj_name, 
               char *writer_id, 
               unsigned int op_num,  
               char *payload, 
               unsigned int size, 
               char *servers, 
               char *port
             ); 
char *SODAW_read(char *obj_name, 
               char *writer_id, 
               unsigned int op_num,  
               //char *payload, 
               char *servers, 
               char *port
             );

zhash_t *receive_message_frames_from_server_SODAW(zmsg_t *msg)  ;

#endif
