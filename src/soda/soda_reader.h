#ifndef _SODA_READER
#define _SODA_READER

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
#include "../codes/rlnc_rs.h"

char *SODA_read(char *obj_name, unsigned int op_num, EncodeData *encoding_info, ClientArgs *client_args);

#endif
