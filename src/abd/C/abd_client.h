#ifndef _ABD_CLIENT
#define _ABD_CLIENT

#include "czmq.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>

#include "algo_utils.h"

//  This is our client task
//  It connects to the server, and then sends a request once per second
//  It collects responses as they arrive, and it prints them out. We will
//  run several client tasks in parallel, each with a different random ID.

TAG get_max_tag_phase(char *obj_name, unsigned int op_num, 
                      zsock_t *sock_to_servers,  char **servers, 
                          unsigned int num_servers, char *port) ;

//  This is our client task
//  It connects to the server, and then sends a request once per second
//  It collects responses as they arrive, and it prints them out. We will
//  run several client tasks in parallel, each with a different random ID.

//   write_value_phase(obj_name, writer_id,  op_num, sock_to_servers, servers, num_servers, port, payload, size, max_tag);

TAG write_value_phase(  
                      char *obj_name,
                      char *writer_id, 
                      unsigned int op_num, 
                      zsock_t *sock_to_servers,  
                      char **servers, 
                      unsigned int num_servers, 
                      char *port, byte *payload, 
                      int size, 
                      TAG max_tag
                   ) ;

bool ABD_write(char *obj_name, char *writer_id, unsigned int op_num,  byte *payload, unsigned int size, char **servers, unsigned int num_servers, char *port);

bool ABD_read(byte *payload, unsigned int size, char **servers, unsigned int num_servers, char *port);

int ABD_hello(int a, int b) ;

#endif
