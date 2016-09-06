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
void algorithm_SODAW(zhash_t *frames, void *worker, void *server_args) ;

void SODAW_initialize();


char *create_a_key_from_metadata(METADATA *m) ;

char *create_a_key_from_readerc(REGREADER *m);

void  metadata_remove_keys(zhash_t *metadata, zlist_t *Hr) ;

zlist_t *metadata_with_reader(zhash_t *metadata, char *sender) ;

void metadata_disperse(char *object_namae, char *algorithm, METADATA *h);

#endif
