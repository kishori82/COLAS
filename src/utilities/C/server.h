#ifndef __SERVER__
#define __SERVER__

#include "czmq.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include "algo_utils.h"


int server_process(
               char *server_id, 
               char *servers_str, 
               char *port,
               char *init_data,
               SERVER_STATUS *status
             );



#endif
