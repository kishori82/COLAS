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


//   write_value_phase(obj_name, writer_id,  op_num, sock_to_servers, servers, num_servers, port, payload, size, max_tag);
/*
char * ABD_read(char *obj_name,
                char *writer_id,
                unsigned int op_num,
                //char *payload,
                char *servers,
                char *port
               );
*/

char * ABD_read(char *obj_name,
                unsigned int op_num,
                EncodeData *encoding_info,
                ClientArgs *client_args,
               );


#endif
