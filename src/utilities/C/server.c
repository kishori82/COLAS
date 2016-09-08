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

#include "../../sodaw/sodaw_server.h"

#include "../../abd/abd_server.h"

#define DEBUG_MODE 1
extern int s_interrupted;

SERVER_STATUS *status;
SERVER_ARGS *server_args;

#ifdef ASLIBRARY

//  .split server task
//  This is our server task.
//  It uses the multithreaded server model to deal requests out to a pool
//  of workers and route replies back to clients. One worker can handle
//  one request at a time but one client can talk to multiple workers at
//  once.

static void server_worker (void *args, zctx_t *ctx, void *pipe);

char *ID;

void *server_task (void *server_args)
{

    //  Frontend socket talks to clients over TCP
    zctx_t *ctx = zctx_new ();
    void *frontend = zsocket_new(ctx, ZMQ_ROUTER);
    zsocket_bind(frontend, "tcp://*:8081");
    
    //  Backend socket talks to workers over inproc
    void *backend = zsocket_new (ctx, ZMQ_DEALER);
    zsocket_bind (backend, "inproc://backend");

    //  Launch pool of worker threads, precise number is not critical
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
    void *worker = zsocket_new (ctx, ZMQ_DEALER);
    zsocket_connect(worker, "inproc://backend");
    char algorithm_name[100];

    
    printf("Initial value size %ld\n", strlen( ((SERVER_ARGS *)server_args)->init_data));

    zmq_pollitem_t items[] = { { worker, 0, ZMQ_POLLIN, 0}};
    while (true) {

        int rc = zmq_poll(items, 1, -1);
        if( rc < 0 || s_interrupted ) {
             exit(0);
        }
        
        zclock_sleep(5);
        if (items[0].revents & ZMQ_POLLIN) {
           zmsg_t *msg = zmsg_recv (worker);

           // receive the frames
           status->network_data += (float)zmsg_content_size(msg) ;

          
           zhash_t *frames = receive_message_frames_at_server(msg);

           get_string_frame(algorithm_name, frames, "algorithm");

           if( strcmp(algorithm_name, "ABD")==0)  {
                printf("ABD RESPONDING %s\n", ID);
                if(DEBUG_MODE) { printf("\treceiving...\n");  print_out_hash(frames); }
                algorithm_ABD(frames, worker, server_args);
                printf("ABD RESPONSE COMPLETE\n\n");
           }
   
           if( strcmp(algorithm_name, "SODAW")==0)  {
                printf("\tSODAW RESPONDING %s\n",ID);
                if(DEBUG_MODE)  print_out_hash(frames);
                algorithm_SODAW(frames, worker, server_args);
                printf("\tSODAW RESPONSE COMPLETE\n");
           }

           destroy_frames(frames);
        }
    }
   
}

int server_process(char *server_id, char *servers_str, char *port, char *init_data, SERVER_STATUS *_status)
{
   ID=server_id;

   s_catch_signals();

   status = _status;

   status->network_data = 0;
   status->data_memory = 0;
   status->metadata_memory = 0;

   server_args = (SERVER_ARGS *)malloc(sizeof(SERVER_ARGS));
   server_args->init_data = init_data;
   server_args->servers_str = servers_str;
   server_args->port = port;
   server_args->server_id= server_id;

   zthread_new(server_task, (void *)server_args);
   printf("Starting the thread id %s\n", server_id);
   printf("Starting server  name : %s\n", ID);

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
