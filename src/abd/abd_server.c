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
static zhash_t *hash_object_ABD;

static int initialized = 0;

void initialize_ABD() {
   initialized = 1;
   hash_object_ABD = zhash_new();
   assert(hash_object_ABD);
   assert(zhash_size (hash_object_ABD) == 0);
   assert(zhash_first (hash_object_ABD) == NULL);

}


void algorithm_ABD_WRITE_VALUE( zhash_t *frames, void *worker) {
    char tag_str[100];
    char object_name[100];
    printf("\tWRITE_VALUE\n");


    if( DEBUG_MODE ) printf("\t\t INSIDE WRITE VALUE\n");
    TAG tag; 
    get_string_frame(tag_str, frames, "tag");
    string_to_tag(tag_str, &tag);
     
    get_string_frame(object_name, frames, "object");

    TAG local_tag;
    get_object_tag(hash_object_ABD, object_name, &local_tag);

//    if( DEBUG_MODE )printf("\t\t WRITE TAG for COMP (%d, %s)  (%d, %s)\n", local_tag.z, local_tag.id, tag.z, tag.id);

    if( compare_tags(local_tag, tag)==-1 ) {
        if( DEBUG_MODE )printf("\t\tBEHIND\n");

        zframe_t *payload_frame= (zframe_t *)zhash_lookup(frames, "payload");
        int size = zframe_size(payload_frame); 
        void *frame_data = zframe_data(payload_frame); 
        char *data =  (char *)malloc(size + 1);
        memcpy(data, frame_data, size);
        data[size]='\0';

        if( DEBUG_MODE)      printf("data %s\n", data);
        assert(hash_object_ABD!=NULL);
        zhash_t *temp_hash_hash = zhash_lookup(hash_object_ABD, object_name);

        assert(temp_hash_hash!=NULL);
        if( DEBUG_MODE ) printf("\t\tIS NULL %p\n", temp_hash_hash);
 
        zlist_t *keys = zhash_keys (temp_hash_hash);
        assert(keys!=NULL);

       if( DEBUG_MODE ) printf("\t\t# KEYS  %d\n", (int)zlist_size(keys));
        assert(keys!=NULL);

        void *key = zlist_first(keys);
        assert(key!=NULL);

        void *item = zhash_lookup(temp_hash_hash,key);
        status->data_memory -= (float)strlen((char *)item);
        status->metadata_memory -=  (float)strlen( (char *)key);
        zhash_delete(temp_hash_hash, key);
        free(item);

  
        if( DEBUG_MODE ) printf("\t\t# KEYS AFTER DEL %d\n", (int)zhash_size(temp_hash_hash));

        zhash_insert(temp_hash_hash,tag_str, data); 
        status->metadata_memory +=  (float) strlen(tag_str);
        status->data_memory += (float)  size;


       if( DEBUG_MODE ) printf("\t\tINSERTING KEY %s data of size  %d\n", tag_str, size);

        zframe_t *ack_frame = zframe_new("SUCCESS",strlen("SUCCESS"));
        zhash_insert(frames, "acknowledge", ack_frame);
        
       if( DEBUG_MODE ) printf("\t\tSENT SUCCESS\n");
    }
    else {
        zframe_t *ack_frame = zframe_new("SUCCESS",strlen("SUCCESS"));
        zhash_insert(frames, "acknowledge", ack_frame);
       if( DEBUG_MODE ) printf("\t\tSENT BEHIND\n");
    }

    get_string_frame(tag_str, frames, "phase"); 
    printf("sending phase %s\n", tag_str);

    send_frames(frames, worker, SEND_FINAL, 6,  "sender", "object",  "algorithm", "phase", "opnum", "tag");

    return;
}


void algorithm_ABD_GET_TAG(zhash_t *frames, void *worker) {
     char object_name[100];
     char tag_buf[100];
     printf("\tGET_TAG\n");

     get_string_frame(object_name, frames, "object");
     TAG tag;
     get_object_tag(hash_object_ABD, object_name, &tag); 
     tag_to_string(tag, tag_buf);

     zframe_t *tag_frame= zframe_new(tag_buf, strlen(tag_buf));
     zhash_insert(frames, "tag", (void *)tag_frame);
     int opnum= get_int_frame(frames, "opnum");
     assert(opnum>=0);

     send_frames(frames, worker, SEND_FINAL, 6,  "sender", "object",  "algorithm", "phase", "opnum", "tag");
}

void algorithm_ABD_GET_TAG_VALUE(zhash_t  *frames,  void *worker) {
     char tag_buf[100];
     char object_name[100];
     printf("\tGET_TAG\n");

     get_string_frame(object_name, frames, "object");
     TAG tag;
     get_object_tag(hash_object_ABD, object_name, &tag); 
     tag_to_string(tag, tag_buf);


     zhash_t *temp_hash_hash = zhash_lookup(hash_object_ABD, object_name);
     assert(temp_hash_hash!=NULL);

     zlist_t *keys = zhash_keys (temp_hash_hash);

     assert((int)zlist_size(keys)==1);
        
     void *key = zlist_first(keys);
     
     assert(key!=NULL);

     zframe_t *tag_frame= zframe_new(tag_buf, strlen(tag_buf));
     zhash_insert(frames, "tag", (void *)tag_frame);

     void *item = zhash_lookup(temp_hash_hash,key);
     zframe_t *data_frame= zframe_new((char *)item, strlen(item));
     zhash_insert(frames, "payload", (void *)data_frame);

     send_frames(frames, worker, SEND_FINAL, 7,  "sender", "object",  "algorithm", "phase", "opnum", "tag", "payload" );

}

void algorithm_ABD(zhash_t *frames, void *worker, void *server_args) {

     if(initialized==0) initialize_ABD();


     char phase_buf[100];
     char object_name[100];

     printf("algorithm ABD\n");

     get_string_frame(phase_buf, frames, "phase");
     get_string_frame(object_name, frames, "object");

     if( has_object(hash_object_ABD, object_name)==0) {
         create_object(hash_object_ABD, object_name, ((SERVER_ARGS *)server_args)->init_data, status);
         printf("\t\tCreated object %s\n",object_name);
    }

      if( strcmp(phase_buf, GET_TAG)==0)  {
           algorithm_ABD_GET_TAG(frames, worker);
      }
   
      if( strcmp(phase_buf, WRITE_VALUE)==0)  {
            algorithm_ABD_WRITE_VALUE(frames, worker);
      }

      if( strcmp(phase_buf, GET_TAG_VALUE)==0)  {
           algorithm_ABD_GET_TAG_VALUE(frames, worker);
      }

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
