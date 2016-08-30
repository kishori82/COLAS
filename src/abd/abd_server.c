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

#define DEBUG_MODE 0
static int se_interrupted=0;

SERVER_STATUS *status;


void se_signal_handler(int signal_value)
{
    se_interrupted=1;
}

void se_catch_signals ()
{
    struct sigaction action;
    action.sa_handler = se_signal_handler;
    //  Doesn't matter if SA_RESTART set because self-pipe will wake up zmq_poll
    //  But setting to 0 will allow zmq_read to be interrupted.
    action.sa_flags = 0;
    sigemptyset (&action.sa_mask);
    sigaction (SIGINT, &action, NULL);
    sigaction (SIGTERM, &action, NULL);
}


#ifdef ASLIBRARY
#include "algo_utils.h"
zhash_t *hash;

//  .split server task
//  This is our server task.
//  It uses the multithreaded server model to deal requests out to a pool
//  of workers and route replies back to clients. One worker can handle
//  one request at a time but one client can talk to multiple workers at
//  once.

static void server_worker (void *args, zctx_t *ctx, void *pipe);

int has_object( char *obj_name) {
    void *item;
    item = zhash_lookup(hash, obj_name);
    if( item==NULL) {
       return 0;
    }
    return 1;
}



int create_object(char *obj_name, char *init_data) {
    void *item =NULL;
    char tag_str[100];
    TAG tag;

    item = zhash_lookup(hash, obj_name);

    if( item==NULL) {
       zhash_t *hash_hash = zhash_new();
       tag.z = 0;
       sprintf(tag.id, "%s", "client_0");
       tag_to_string(tag, tag_str);

       char *value =(void *)malloc(strlen(init_data)+1);
       strcpy(value, init_data);
       value[strlen(init_data)]= '\0';
       zhash_insert(hash_hash, tag_str, (void *)value); 

       status->metadata_memory += (float) strlen(tag_str);
       status->data_memory += (float) strlen(init_data);
        
       //add it to the main list 
       zhash_insert(hash, obj_name, (void *)hash_hash); 
       return 1;
    }
    return 0;
}



void *server_task (void *server_args)
{

    hash = zhash_new();
    assert (hash);
    assert (zhash_size (hash) == 0);
    assert (zhash_first (hash) == NULL);


    //  Frontend socket talks to clients over TCP
    zctx_t *ctx = zctx_new ();
    void *frontend = zsocket_new(ctx, ZMQ_ROUTER);
    zsocket_bind(frontend, "tcp://*:8081");
    
    //  Backend socket talks to workers over inproc
    void *backend = zsocket_new (ctx, ZMQ_DEALER);
    zsocket_bind (backend, "inproc://backend");

    //  Launch pool of worker threads, precise number is not critical
    int thread_nbr;
 //   for (thread_nbr = 0; thread_nbr < 5; thread_nbr++)
    zthread_fork (ctx, server_worker, server_args);

    //  Connect backend to frontend via a proxy
    zmq_proxy (frontend, backend, NULL);

    zctx_destroy (&ctx);

    return NULL;
}



static void algorithm_ABD_WRITE_VALUE( zmsg_t *msg, void *worker, char *object_name) {
    char tag_str[100];
    zframe_t *tag_frame= zmsg_pop(msg);
    _zframe_str(tag_frame, tag_str) ;

    //zframe_send(&tag_frame, worker, ZFRAME_REUSE + ZFRAME_MORE);
    //zframe_send(&tag_frame, worker, ZFRAME_REUSE);
    //zframe_destroy(&tag_frame);

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


static void algorithm_ABD_GET_TAG( zmsg_t *msg, void *worker, char *object_name) {
     char buf[100];
     TAG tag;
     get_object_tag(hash, object_name, &tag); 

     tag_to_string(tag, buf);
     zframe_t *tag_frame= zframe_new(buf, strlen(buf));
     zframe_send(&tag_frame, worker, ZFRAME_REUSE);
     zframe_destroy(&tag_frame);
}

static void algorithm_ABD_GET_TAG_VALUE( zmsg_t *msg, void *worker, char *object_name) {
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

static void algorithm_ABD( zmsg_t *msg, void *worker, char *object_name) {
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

//  .split worker task
//  Each worker task works on one request at a time and sends a random number
//  of replies back, with random delays between replies:


static void
server_worker (void *server_args, zctx_t *ctx, void *pipe)
{
    int i, j;
    void *worker = zsocket_new (ctx, ZMQ_DEALER);
    zsocket_connect(worker, "inproc://backend");
    char buf[100];
    char object_name[100];
    char tag[10]; 
    int  round;

    
    printf("Initial value size %ld\n", strlen( ((SERVER_ARGS *)server_args)->init_data));
    printf("Initial value  %s\n", ((SERVER_ARGS *)server_args)->init_data);

    zmq_pollitem_t items[] = { { worker, 0, ZMQ_POLLIN, 0}};
    while (true) {
        //  The DEALER socket gives us the reply envelope and message
   //     zmq_pollitem_t items[] = { { worker, 0, ZMQ_POLLIN, 0}};
        int rc = zmq_poll(items, 1, -1);
        if( rc < 0 || se_interrupted ) {
             exit(0);
        }
        zclock_sleep(5);
        if (items[0].revents & ZMQ_POLLIN) {
           zmsg_t *msg = zmsg_recv (worker);
           status->network_data += zmsg_content_size(msg) ;
            
           zframe_t *identity = zmsg_pop (msg);
           _zframe_str(identity, buf) ;
    //       printf("\n\n%s\n",  buf);
           zframe_send (&identity, worker, ZFRAME_REUSE +ZFRAME_MORE );
   
           zframe_t *object_name_frame= zmsg_pop (msg);
           _zframe_str(object_name_frame, object_name) ;
           zframe_send (&object_name_frame, worker, ZFRAME_REUSE +ZFRAME_MORE );


            //crate object if it is not found
           if( has_object(object_name)==0) {
               create_object(object_name, ((SERVER_ARGS *)server_args)->init_data);
               printf("\t\tCreated object %s\n",object_name);
           }

           zframe_t *algo_frame= zmsg_pop (msg);
           _zframe_str(algo_frame, buf) ;
           zframe_send (&algo_frame, worker, ZFRAME_REUSE +ZFRAME_MORE );
           //printf("%s\n",  buf);

           if( strcmp(buf, "ABD")==0)  {
                printf("\tABD\n");
                algorithm_ABD(msg, worker, object_name);
           }
   
           printf("\tABD DONE\n");
           zframe_destroy (&identity);
           zframe_destroy (&object_name_frame);
           zframe_destroy (&algo_frame);
   
       }
   }
}

int ABD_server_process(char *server_id, char *port, char *init_data, SERVER_STATUS *_status)
{
   int i ; 
   se_catch_signals();
   SERVER_ARGS *server_args = (SERVER_ARGS *)malloc(sizeof(SERVER_ARGS));
   server_args->init_data = init_data;

   status = _status;

   zthread_new(server_task, (void *)server_args);
   printf("Starting the thread with id %s\n", server_id);

   while(true) {
      zclock_sleep(60*600*1000); 
   }
   free(server_args);
   return 0;
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
