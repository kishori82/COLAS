#ifndef __MD_PRIMITIVE__
#define __MD_PRIMITIVE__

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <czmq.h>
#include <zmq.h>
#include <czmq_library.h>
#include <string.h>
#include "algo_utils.h"

void md_meta_send(void *sock_to_servers, int num_servers, char *names[],  int n, ...) ;

#endif
