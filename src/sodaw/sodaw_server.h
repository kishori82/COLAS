#ifndef _SODAW_SERVER
#define _SODAW_SERVER

#include "server.h"
#include <algo_utils.h>

typedef struct _R_C {
    Tag t_r;
    char reader_opnum[BUFSIZE];
    char reader_id[BUFSIZE];
} RegReader;


typedef struct _METADATA {
    Tag t_r;
    char reader_opnum[BUFSIZE];
    char serverid[BUFSIZE];
} MetaData;


void  destroy_regreader(RegReader *r_tr);

void  destroy_metadata(MetaData *r_tr);

//void algorithm_ABD_WRITE_VALUE( zmsg_t *msg, void *worker, char *object_name) ;
//void algorithm_ABD_GET_TAG( zmsg_t *msg, void *worker, char *object_name) ;
//void algorithm_ABD_GET_TAG_VALUE( zmsg_t *msg, void *worker, char *object_name) ;
void algorithm_SODAW(zhash_t *frames, void *worker, void *server_args) ;

void SODAW_initialize();

char *create_a_key_from_metadata(MetaData *m) ;

char *create_a_key_from_readerc(RegReader *m);

void  metadata_remove_keys(zhash_t *metadata, zlist_t *Hr) ;

void  regreader_remove_key(zhash_t *readerc, char *reader) ;

zlist_t *metadata_with_reader(zhash_t *metadata, char *reader) ;

zlist_t *metadata_with_tag_reader(zhash_t *metadata, Tag tag, char *reader) ;

void metadata_disperse(char *object_namae, char *algorithm, MetaData *h);

void create_metadata_sending_sockets() ;
#endif
