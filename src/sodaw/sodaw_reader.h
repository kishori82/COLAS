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
#include "../codes/rlnc_rs.h"
#include "../baseprocess/client.h"

//   write_value_phase(obj_name, writer_id,  op_num, sock_to_servers, servers, num_servers, port, payload, size, max_tag);

int get_encoded_info(zhash_t *received_data, char *decodeableKey, EncodeData *encoding_info) ;

char *SODAW_read(char *obj_name, unsigned int op_num, EncodeData *encoding_info, ClientArgs *client_args);

#endif
