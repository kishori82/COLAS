#ifndef _ABD_SERVER
#define _ABD_SERVER
#include "server.h"


void algorithm_ABD_WRITE_VALUE( zhash_t *frames, void *worker) ;
void algorithm_ABD_GET_TAG( zhash_t *frames, void *worker) ;
void algorithm_ABD_GET_TAG_VALUE( zhash_t *frames, void *worker) ;
void algorithm_ABD(zhash_t *frames, void *worker, void *server_args) ;

#endif
