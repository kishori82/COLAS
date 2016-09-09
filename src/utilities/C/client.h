#ifndef __CLIENT__
#define __CLIENT__

#include "czmq.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>


void s_signal_handler(int signal_value);

void s_catch_signals();

void send_multicast_servers(void *sock_to_servers, int num_servers, char *names[],  int n, ...) ;
void send_multisend_servers(void *sock_to_servers, int num_servers, char **multipart, int size, char *names[],  int n, ...) ;
#endif
