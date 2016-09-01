#ifndef _SODAW_SERVER
#define _SODAW_SERVER
#include "server.h"


//void algorithm_ABD_WRITE_VALUE( zmsg_t *msg, void *worker, char *object_name) ;
//void algorithm_ABD_GET_TAG( zmsg_t *msg, void *worker, char *object_name) ;
//void algorithm_ABD_GET_TAG_VALUE( zmsg_t *msg, void *worker, char *object_name) ;
void algorithm_SODAW( zmsg_t *msg, void *worker, char *object_name) ;

#endif
