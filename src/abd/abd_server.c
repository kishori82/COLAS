//  Asynchronous client-to-server (DEALER to ROUTER)
//
//  While this example runs in a single process, that is to make
//  it easier to start and stop the example. Each task has its own
//  context and conceptually acts as a separate process.

#include <czmq.h> 
#include <zmq.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WRITE_VALUE "WRITE_VALUE"
#define GET_TAG "GET_TAG"
#define GET_TAG_VALUE "GET_TAG_VALUE"
#include "abd_server.h"
#include "algo_utils.h"

#define DEBUG_MODE 0
extern int s_interrupted;

extern SERVER_STATUS *status;


#ifdef ASLIBRARY
extern zhash_t *hash;


void algorithm_ABD_WRITE_VALUE( zmsg_t *msg, void *worker, char *object_name) {
    char tag_str[100];
    zframe_t *tag_frame= zmsg_pop(msg);
    _zframe_str(tag_frame, tag_str) ;

    //zframe_send(&tag_frame, worker, ZFRAME_REUSE + ZFRAME_MORE);
    //zframe_send(&tag_frame, worker, ZFRAME_REUSE);
   // zframe_destroy(&tag_frame);
    //return;

    if( DEBUG_MODE ) printf("\t\t INSIDE WRITE VALUE\n");
    TAG tag; 
    string_to_tag(tag_str, &tag);
     
    TAG local_tag;
    get_object_tag(hash, object_name, &local_tag);
    if( DEBUG_MODE )printf("\t\t WRITE TAG for COMP (%d, %s)  (%d, %s)\n", local_tag.z, local_tag.id, tag.z, tag.id);

    if( compare_tags(local_tag, tag)==-1 ) {
        if( DEBUG_MODE )printf("\t\tBEHIND\n");

        zframe_t *payload_frame= zmsg_pop (msg);
        int size = zframe_size(payload_frame); 
        void *frame_data = zframe_data(payload_frame); 
        char *data =  (char *)malloc(size + 1);

        memcpy(data, frame_data, size);
        data[size]='\0';

        zhash_t *temp_hash_hash = zhash_lookup(hash, object_name);

       if( DEBUG_MODE ) printf("\t\tIS NULL %p\n", temp_hash_hash);
 
        zlist_t *keys = zhash_keys (temp_hash_hash);

       if( DEBUG_MODE ) printf("\t\t# KEYS  %d\n", (int)zlist_size(keys));

        
        void *key = zlist_first(keys);
        if( key!=NULL) {
            void *item = zhash_lookup(temp_hash_hash,key);
            if( item!= NULL) {
               status->data_memory -= (float)strlen((char *)item);
              status->metadata_memory -=  (float)strlen( (char *)key);
            }
            zhash_delete(temp_hash_hash, key);
            free(item);

           if( DEBUG_MODE ) printf("\t\tDELETED\n");
        }

        while( (key=zlist_next(keys))!=NULL ) {
            printf("\t\tWHILE %s\n", (char *)key);
            void *item = zhash_lookup(temp_hash_hash,key);
            if( item !=NULL ) { 
              status->data_memory -= (float) strlen((char *)item);
              status->metadata_memory -= (float)  strlen(key);
            }
            zhash_delete(temp_hash_hash, key);
            free(item);
        }

       if( DEBUG_MODE ) printf("\t\t# KEYS AFTER DEL %d\n", (int)zhash_size(temp_hash_hash));

        zhash_insert(temp_hash_hash,tag_str, data); 
        status->metadata_memory +=  strlen(tag_str);
        status->data_memory += (float)  size;


        zframe_destroy(&payload_frame);
       if( DEBUG_MODE ) printf("\t\tINSERTING KEY %s data of size  %d\n", tag_str, size);

        zframe_t *ack_frame = zframe_new("SUCCESS", 7);
      //  zframe_send(&tag_frame, worker, ZFRAME_REUSE);
        zframe_destroy(&ack_frame);
       if( DEBUG_MODE ) printf("\t\tSENT SUCCESS\n");
    }

    else {
        zframe_t *ack_frame = zframe_new("BEHIND", 6);
        //zframe_send(&tag_frame, worker, ZFRAME_REUSE);
        zframe_destroy(&ack_frame);
       if( DEBUG_MODE ) printf("\t\tSENT BEHIND\n");
    }
    zframe_send(&tag_frame, worker, ZFRAME_REUSE);
    zframe_destroy(&tag_frame);

    return;
}


void algorithm_ABD_GET_TAG( zmsg_t *msg, void *worker, char *object_name) {
     char buf[100];
     TAG tag;
     get_object_tag(hash, object_name, &tag); 

     tag_to_string(tag, buf);
     zframe_t *tag_frame= zframe_new(buf, strlen(buf));
     zframe_send(&tag_frame, worker, ZFRAME_REUSE);
     zframe_destroy(&tag_frame);
}

void algorithm_ABD_GET_TAG_VALUE( zmsg_t *msg, void *worker, char *object_name) {
     char buf[100];
     TAG tag;
     get_object_tag(hash, object_name, &tag); 

     tag_to_string(tag, buf);
     zframe_t *tag_frame= zframe_new(buf, strlen(buf));
     zframe_send(&tag_frame, worker, ZFRAME_REUSE + ZFRAME_MORE);
     zframe_destroy(&tag_frame);

     zhash_t *temp_hash_hash = zhash_lookup(hash, object_name);

     zlist_t *keys = zhash_keys (temp_hash_hash);

     assert((int)zlist_size(keys)==1);
        
     void *key = zlist_first(keys);
     
     assert(key!=NULL);
     void *item = zhash_lookup(temp_hash_hash,key);

     zframe_t *data_frame= zframe_new((char *)item, strlen(item));
     zframe_send(&data_frame, worker, ZFRAME_REUSE);
     
     zframe_destroy(&data_frame);

}

void algorithm_ABD( zmsg_t *msg, void *worker, char *object_name) {
     char buf[100];
     char tag[10]; 
     int  round;

     printf("algorithm ABD\n");
     zframe_t *phase_frame= zmsg_pop (msg);

     _zframe_str(phase_frame, buf) ;
     zframe_send(&phase_frame, worker, ZFRAME_REUSE + ZFRAME_MORE);

     zframe_t *op_num_frame= zmsg_pop (msg);

     zframe_send(&op_num_frame, worker, ZFRAME_REUSE + ZFRAME_MORE);

     printf("algorithm ABD tag %s\n", buf);
      if( strcmp(buf, GET_TAG)==0)  {
           printf("\t-----------------\n");
           printf("\tGET_TAG\n");
           algorithm_ABD_GET_TAG(msg, worker, object_name);
      }
   
      if( strcmp(buf, WRITE_VALUE)==0)  {
            printf("\tWRITE_VALUE\n");
            algorithm_ABD_WRITE_VALUE(msg, worker, object_name);
      }

      if( strcmp(buf, GET_TAG_VALUE)==0)  {
           printf("\t-----------------\n");
           printf("\tGET_TAG\n");
           algorithm_ABD_GET_TAG_VALUE(msg, worker, object_name);
      }

      zframe_destroy(&phase_frame);
      zframe_destroy(&op_num_frame);

   /* zframe_t *payloadf= zmsg_pop (msg);
           printf("%d\n",  (int)zframe_size(payloadf));
   */
   
   
 }

#endif

//  The main thread simply starts several clients and a server, and then
//  waits for the server to finish.

#ifdef ASMAIN
int main (void)
{
   int i ; 
   zthread_new(server_task, NULL);
   zclock_sleep(60*60*1000); 
   return 0;
}
#endif
