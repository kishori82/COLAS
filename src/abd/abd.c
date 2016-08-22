//  Asynchronous client-to-server (DEALER to ROUTER)
//
//  While this example runs in a single process, that is to make
//  it easier to start and stop the example. Each task has its own
//  context and conceptually acts as a separate process.

#include "czmq.h"
#include <stdio.h>
#include <stdlib.h>
#include "algo_utils.h"
#define NUM_CLIENTS 2

//  This is our client task
//  It connects to the server, and then sends a request once per second
//  It collects responses as they arrive, and it prints them out. We will
//  run several client tasks in parallel, each with a different random ID.

static void * client_task (void *args)
{
    
    zctx_t *ctx = zctx_new ();
    zframe_t  *f;
    void *client = zsocket_new (ctx, ZMQ_DEALER);

    //  Set random identity to make tracing easier
    char identity [10];
    sprintf (identity, "%04X-%04X", randof (0x10000), randof (0x10000));

    zsocket_set_identity (client, identity);
    zsocket_connect (client, "tcp://localhost:5570");

    zmq_pollitem_t items [] = { { client, 0, ZMQ_POLLIN, 0 } };
    int request_nbr = 0;

   zframe_t *algo = zframe_new("ABD", 3);
   zframe_t *step = zframe_new("GET_TAG", 7);

   byte *payload = (byte *)malloc(100000000*sizeof(byte));
   int size = 100000000*sizeof(byte);
   zframe_t *payloadf = zframe_new((const void *)payload, size);



    int round =0;
    while (true) {
        //  Tick once per second, pulling in arriving messages
        int centitick;
        for (centitick = 0; centitick < 100; centitick++) {
            zmq_poll (items, 1, 10 * ZMQ_POLL_MSEC);
            if (items [0].revents & ZMQ_POLLIN) {
                zmsg_t *msg = zmsg_recv (client);
               // zframe_print (zmsg_last (msg), identity);

/*
                while ( (f =zmsg_pop(msg) )!=NULL ) {
                   zframe_print (f, identity);
                   printf("Data %d\n", (int)zframe_size(f));
                }
*/

                zmsg_destroy (&msg);
            }
        }

        zframe_t *roundf = zframe_new((const void *)&round, sizeof(int));
        round++;

        zframe_send (&algo, client, ZFRAME_REUSE + ZFRAME_MORE);
        zframe_send (&step, client, ZFRAME_REUSE + ZFRAME_MORE);
        zframe_send (&roundf, client, ZFRAME_REUSE + ZFRAME_MORE);
        zframe_send (&payloadf, client, ZFRAME_REUSE);
        zframe_destroy (&roundf);

//        zstr_sendf (client, "request #%d", ++request_nbr);
    }
    zframe_destroy (&algo);
    zframe_destroy (&step);
   // zframe_destroy (&roundf);
    zframe_destroy (&payloadf);

    zctx_destroy (&ctx);
    return NULL;
}

//  .split server task
//  This is our server task.
//  It uses the multithreaded server model to deal requests out to a pool
//  of workers and route replies back to clients. One worker can handle
//  one request at a time but one client can talk to multiple workers at
//  once.

static void server_worker (void *args, zctx_t *ctx, void *pipe);

void *server_task (void *args)
{
    //  Frontend socket talks to clients over TCP
    zctx_t *ctx = zctx_new ();
    void *frontend = zsocket_new (ctx, ZMQ_ROUTER);
    zsocket_bind (frontend, "tcp://*:5570");

    //  Backend socket talks to workers over inproc
    void *backend = zsocket_new (ctx, ZMQ_DEALER);
    zsocket_bind (backend, "inproc://backend");

    //  Launch pool of worker threads, precise number is not critical
    int thread_nbr;
    for (thread_nbr = 0; thread_nbr < 100; thread_nbr++)
        zthread_fork (ctx, server_worker, NULL);

    //  Connect backend to frontend via a proxy
    zmq_proxy (frontend, backend, NULL);
    zctx_destroy (&ctx);
    return NULL;
}

//  .split worker task
//  Each worker task works on one request at a time and sends a random number
//  of replies back, with random delays between replies:


static void
server_worker (void *args, zctx_t *ctx, void *pipe)
{
    int i, j;
    void *worker = zsocket_new (ctx, ZMQ_DEALER);
    zsocket_connect (worker, "inproc://backend");
    char buf[100];
    char tag[10]; 
    int  round;
    sprintf (tag, "%04X-%04X", randof (0x10000), randof (0x10000));



    while (true) {
        //  The DEALER socket gives us the reply envelope and message
        zmsg_t *msg = zmsg_recv (worker);

        zframe_t *identity = zmsg_pop (msg);

        zframe_t *algo= zmsg_pop (msg);
        _zframe_str(algo, buf) ;
        printf("%s\n",  buf);

        zframe_t *step= zmsg_pop (msg);
        _zframe_str(step, buf) ;
        printf("%s\n",  buf);
        
        zframe_t *roundf= zmsg_pop (msg);
        _zframe_int(roundf, &round) ;
        printf("%d\n",  round);

        zframe_t *payloadf= zmsg_pop (msg);
        printf("%d\n",  (int)zframe_size(payloadf));

        assert (step);
        zmsg_destroy (&msg);

        //  Send 0..4 replies back
        int reply, replies = randof (5);
        for (reply = 0; reply < replies; reply++) {
            //  Sleep for some fraction of a second
            zclock_sleep (randof (1000) + 1);
            //zframe_send (&identity, worker, ZFRAME_REUSE + ZFRAME_MORE);
            zframe_send (&identity, worker, ZFRAME_REUSE );
            //zframe_send (&content, worker, ZFRAME_REUSE);
        }

        zframe_destroy (&identity);
        zframe_destroy (&algo);
        zframe_destroy (&step);
        zframe_destroy (&payloadf);
    }
}

//  The main thread simply starts several clients and a server, and then
//  waits for the server to finish.

int main (void)
{
   int i ; 
   for(i =0; i < NUM_CLIENTS; i++) {
      zthread_new (client_task, NULL);
   }
   zthread_new (server_task, NULL);
   zclock_sleep(50*1000); 
   return 0;
}
