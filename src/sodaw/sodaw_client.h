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

#define WRITE_GET "WRITE_GET"
#define WRITE_PUT "WRITE_PUT"
#define READ_COMPLETE "READ_COMPLETE"
#define READ_GET "READ_GET"
#define READ_VALUE "READ_VALUE"



//   write_value_phase(obj_name, writer_id,  op_num, sock_to_servers, servers, num_servers, port, payload, size, max_tag);


zhash_t *receive_message_frames_from_server_SODAW(zmsg_t *msg)  ;


char *number_responses_at_least(zhash_t *received_data, unsigned int atleast);


TAG *SODAW_write_get_or_read_get_phase(
        char *obj_name, 
        const char *_phase, 
        unsigned int op_num, 
        zsock_t *sock_to_servers,  
        char **servers, 
        unsigned int num_servers, 
        char *port);

#endif
