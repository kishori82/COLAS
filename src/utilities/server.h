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
#include "../codes/rlnc_rs.h"

#include "client.h"
#include "../sodaw/sodaw_server.h"
#include "../abd/abd_server.h"

#define DEBUG_MODE  1

zhash_t *receive_message_frames_at_server(zmsg_t *msg, zlist_t *names) ;
int store_payload(zhash_t *object_hash, char *obj_name, Tag tag, zframe_t *payload, enum INSERT_DATA_POLICY policy) ;
int create_object(zhash_t *object_hash, char *obj_name, char *algorithm, char *init_data, Server_Status *status) ;

void send_frames_at_server(zhash_t *frames, void *worker, enum SEND_TYPE type, int n, ...) ;
#endif
