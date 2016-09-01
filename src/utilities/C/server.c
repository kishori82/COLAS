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

#include "client.h"
#include "server.h"
#include "algo_utils.h"

#include "../../abd/abd_server.h"

#define DEBUG_MODE 0
extern int s_interrupted;

SERVER_STATUS *status;

#ifdef ASLIBRARY
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
        if( rc < 0 || s_interrupted ) {
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
                printf("\tABD DONE\n");
           }
   
           if( strcmp(buf, "SODAW")==0)  {
                printf("\tSODAW\n");
                algorithm_SODAW(msg, worker, object_name);
                printf("\tSODAW DONE\n");
           }

           zframe_destroy (&identity);
           zframe_destroy (&object_name_frame);
           zframe_destroy (&algo_frame);
   
       }
   }
}

int server_process(char *server_id, char *port, char *init_data, SERVER_STATUS *_status)
{
   int i ; 

   s_catch_signals();

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
