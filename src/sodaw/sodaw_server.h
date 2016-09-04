#ifndef _SODAW_SERVER
#define _SODAW_SERVER
#include "server.h"
#include <algo_utils.h>

typedef struct _R_C {
   TAG t_r;
   char *readerid;
} REGREADER;


typedef struct _METADATA {
   TAG t_r;
   char *readerid;
   char *serverid;
} METADATA;


//void algorithm_ABD_WRITE_VALUE( zmsg_t *msg, void *worker, char *object_name) ;
//void algorithm_ABD_GET_TAG( zmsg_t *msg, void *worker, char *object_name) ;
//void algorithm_ABD_GET_TAG_VALUE( zmsg_t *msg, void *worker, char *object_name) ;
void algorithm_SODAW(char *ID, zhash_t *frames, zmsg_t *msg, void *worker, char *client,  char *object_name, char *algorithm_name) ;

void SODAW_initialize();


char *create_a_key_from_metadata(METADATA *m) ;

char *create_a_key_from_readerc(REGREADER *m);

#endif
