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


#define TRUE 1
#define FALSE 0


//  ABD
#define WRITE_VALUE "WRITE_VALUE"
#define GET_TAG "GET_TAG"
#define GET_TAG_VALUE "GET_TAG_VALUE"


/// SODAW
#define WRITE_GET "WRITE_GET"
#define WRITE_PUT "WRITE_PUT"
#define READ_COMPLETE "READ_COMPLETE"
#define READ_GET "READ_GET"
#define READ_VALUE "READ_VALUE"

#define READ_COMPLETE "READ_COMPLETE"
#define READ_DISPERSE "READ_DISPERSE"
#define META_TAG "META_TAG"
#define META_SERVERID "META_SERVERID"
#define META_READERID "META_READERID"



// GENERAL
#define OBJECT "OBJECT"
#define ALGORITHM "ALGORITHM"
#define OPNUM "OPNUM"
#define PAYLOAD "PAYLOAD"
#define PHASE "PHASE"
#define SENDER "SENDER"
#define TAG "TAG"

#define SODAW "SODAW"
#define ABD "ABD"

// Parameters
#define BUFSIZE 100
#define PAYLOADBUF_SIZE 3000
#define SYMBOL_SIZE 1024

int s_interrupted;

typedef struct  _TAG {
    int z;
    char id[100];
}  Tag;

typedef struct  _TAG_VALUE {
    Tag tag;
    void *data;
    int size;
}  Tag_Valuu;



enum INSERT_DATA_POLICY{
   force, yield
};


void s_signal_handler(int signal_value);

void s_catch_signals ();

enum ProcessType { server=2, reader=0, writer=1  };

enum Algorithm {abd=0, sodaw=1};


bool is_equal(char *payload1, char*payload2, unsigned int size);

void _zframe_int(zframe_t *f, int *i) ;

void _zframe_uint(zframe_t *f, unsigned int *i) ;

int  get_tag_frame(zhash_t *frames, Tag *tag);

void _zframe_str(zframe_t *f, char *buf) ;

void _zframe_value(zframe_t *f, char *buf) ;

char *create_destinations(char **servers, unsigned int num_servers, char *port, char type) ;

char *create_destination(char *server, char *port) ;


void init_tag(Tag *tag) ;


/*
   returns -1 if a < b
            0 if a = b
           +1 if a > b
*/


int has_object(zhash_t *object_hash,  char *obj_name) ;

int print_object_hash(zhash_t *object_hash) ;

int compare_tag_ptrs(Tag *a, Tag *b) ;

int compare_tags(Tag a, Tag b) ;

// converts a string to a tag
void string_to_tag(char *str, Tag *tag) ;

// converts a tag to a string
void tag_to_string(Tag tag, char *buf) ;

Tag *get_max_tag(zlist_t *tag_list) ;
     
void free_items_in_list( zlist_t *list) ;

int  get_object_tag(zhash_t *hash, char * object_name, Tag *tag) ;

char * get_object_value(zhash_t *hash, char * object_name, Tag tag) ;

zframe_t * get_object_frame(zhash_t *hash, char * object_name, Tag tag) ;

char **create_server_names(char *servers_str) ;

void  destroy_server_names(char **servers, int num_servers) ;
unsigned int count_num_servers(char *servers_str) ;


int  get_string_frame(char *buf, zhash_t *frames,  const char *str);

int  get_int_frame(zhash_t *frames, const char *str);

void destroy_frames(zhash_t *frames);

enum SEND_TYPE {SEND_MORE, SEND_FINAL};


void print_out_hash(zhash_t* frames);

void print_out_hash_in_order(zhash_t* frames, zlist_t *names);


#endif

