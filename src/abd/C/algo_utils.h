#ifndef _ALGO_UTILS
#define _ALGO_UTILS

#include <czmq.h>
#include <zmq.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void _zframe_int(zframe_t *f, int *i) ;


void _zframe_str(zframe_t *f, char *buf) ;

char *create_destinations(char **servers, unsigned int num_servers, char *port, char type) ;

char *create_destination(char *server, char *port) ;

typedef struct  _TAG {
    int z;
    char id[64];
}  TAG;

typedef struct  _TAG_VALUE {
    TAG tag;
    void *data;
    int size;
}  TAG_VALUE;


/*
   returns -1 if a < b
            0 if a = b
           +1 if a > b


*/

int compare_tag_ptrs(TAG *a, TAG *b) ;


int compare_tags(TAG a, TAG b) ;

void string_to_tag(char *str, TAG *tag) ;

void tag_to_string(TAG tag, char *buf) ;


TAG get_max_tag( zlist_t *tag_list) ;
     
void free_items_in_list( zlist_t *list) ;

int  get_object_tag(zhash_t *hash, char * object_name, TAG *tag) ;

#endif

