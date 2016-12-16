#ifndef _ABD_READER
#define _ABD_READER

#include <czmq.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <algo_utils.h>
#include <base64.h>
#include "../baseprocess/client.h"
#include "abd_client.h"


RawData * ABD_read(
                char *obj_name,
                unsigned int op_num,
                ClientArgs *client_args
               );


#endif
