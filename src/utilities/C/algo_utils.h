#ifndef _ALGO_UTILS
#define _ALGO_UTILS

#include <czmq.h>
#include <zmq.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>

#define WRITE_VALUE "WRITE_VALUE"
#define GET_TAG "GET_TAG"
#define GET_TAG_VALUE "GET_TAG_VALUE"


typedef struct  _TAG {
    int z;
    char id[100];
}  TAG;

typedef struct  _TAG_VALUE {
    TAG tag;
    void *data;
    int size;
}  TAG_VALUE;


void _zframe_int(zframe_t *f, int *i) ;

void _zframe_uint(zframe_t *f, unsigned int *i) ;

int  get_tag_frame(zhash_t *frames, TAG *tag);

void _zframe_str(zframe_t *f, char *buf) ;

void _zframe_value(zframe_t *f, char *buf) ;

char *create_destinations(char **servers, unsigned int num_servers, char *port, char type) ;

char *create_destination(char *server, char *port) ;


void init_tag(TAG *tag) ;


/*
   returns -1 if a < b
            0 if a = b
           +1 if a > b


*/


int has_object(zhash_t *object_hash,  char *obj_name) ;

int print_object_hash(zhash_t *object_hash) ;

int compare_tag_ptrs(TAG *a, TAG *b) ;

int compare_tags(TAG a, TAG b) ;

// converts a string to a tag
void string_to_tag(char *str, TAG *tag) ;

// converts a tag to a string
void tag_to_string(TAG tag, char *buf) ;

TAG get_max_tag( zlist_t *tag_list) ;
     
void free_items_in_list( zlist_t *list) ;

int  get_object_tag(zhash_t *hash, char * object_name, TAG *tag) ;

char * get_object_value(zhash_t *hash, char * object_name, TAG tag) ;

zframe_t * get_object_frame(zhash_t *hash, char * object_name, TAG tag) ;

char **create_server_names(char *servers_str) ;

unsigned int count_num_servers(char *servers_str) ;



int  get_string_frame(char *buf, zhash_t *frames,  const char *str);

int  get_int_frame(zhash_t *frames, const char *str);

void destroy_frames(zhash_t *frames);

enum SEND_TYPE {SEND_MORE, SEND_FINAL};

void send_frames_at_server(zhash_t *frames, void *worker, enum SEND_TYPE type, int n, ...) ;

void print_out_hash(zhash_t* frames);

void print_out_hash_in_order(zhash_t* frames, zlist_t *names);

zhash_t *receive_message_frames_at_client(zmsg_t *msg, zlist_t *names) ;

zhash_t *receive_message_frames_at_server(zmsg_t *msg, zlist_t *names) ;
#endif

