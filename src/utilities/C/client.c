//  Asynchronous client-to-server (DEALER to ROUTER)
//
//  While this example runs in a single process, that is to make
//  it easier to start and stop the example. Each task has its own
//  context and conceptually acts as a separate process.

#include "czmq.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <algo_utils.h>
#include <base64.h>
#include "client.h"


#define DEBUG_MODE  1

int s_interrupted;

void s_signal_handler(int signal_value)
{
    s_interrupted=1;
}

void s_catch_signals ()
{
    struct sigaction action;
    action.sa_handler = s_signal_handler;
    //  Doesn't matter if SA_RESTART set because self-pipe will wake up zmq_poll
    //  But setting to 0 will allow zmq_read to be interrupted.
    s_interrupted=0;
    action.sa_flags = 0;
    sigemptyset (&action.sa_mask);
    sigaction (SIGINT, &action, NULL);
    sigaction (SIGTERM, &action, NULL);
}

void send_multicast_servers(void *sock_to_servers, int num_servers, char *names[],  int n, ...) {
    va_list valist;
    int i =0, j;

    va_start(valist, n);
     
    void **values = (void *)malloc(n*sizeof(void *));
    zframe_t **frames = (zframe_t *)malloc(n*sizeof(zframe_t *));
    assert(values!=NULL);
    assert(frames!=NULL);

    for(i=0; i < n; i++ ) {

        if( strcmp(names[i], "opnum")==0)   {
           values[i] = (void *)va_arg(valist, unsigned  int *); 
        }
        else
           values[i] = va_arg(valist, void *); 

        if( strcmp(names[i], "opnum")==0) {
            frames[i]= zframe_new((const void *)values[i], sizeof(unsigned int));
        }
        else {
            frames[i]= zframe_new(values[i], strlen((char *)values[i]));
        }
    }
    va_end(valist);

    // it to all servers in a round robin fashion
    if(DEBUG_MODE)  printf("\n");

    if( DEBUG_MODE) printf("\tsending ..\n");
    for(i=0; i < num_servers; i++) {

       for(j=0; j < n-1; j++) {
          if(DEBUG_MODE) {
            if( strcmp(names[j], "opnum")==0)  
               printf("\t\tFRAME%d :%s  %d\n", j, names[j], *((unsigned int *)values[j]) );
            else if( strcmp(names[j], "payload")==0)  
               printf("\t\tFRAME%d :%s  %d\n", j, names[j],  strlen((char *)values[j]) );
            else
               printf("\t\tFRAME%d :%s  %s\n", j, names[j],   (char *)values[j]);
             
            zframe_send( &frames[j], sock_to_servers, ZFRAME_REUSE + ZFRAME_MORE);
         }
       }

       if(DEBUG_MODE) {
         if( strcmp(names[j], "opnum")==0)  
            printf("\t\tFRAME%d :%s  %d\n", j, names[j],   *((unsigned int *)values[j]) );
         else if( strcmp(names[j], "payload")==0)  
            printf("\t\tFRAME%d :%s  %d\n", j, names[j],  strlen((char *)values[j]) );
         else
            printf("\t\tFRAME%d :%s  %s\n", j, names[j],   (char *)values[j]);
       }
      zframe_send( &frames[j], sock_to_servers, ZFRAME_REUSE);
      if(DEBUG_MODE)  printf("\n");
   }

     printf("\n");
    if(DEBUG_MODE)  printf("\n");

    if( values!=NULL) free(values); 

    for(i=0; i < n; i++ ) {
       zframe_destroy(frames+i);
    }
    if( frames!=NULL) free(frames);
}

void send_multisend_servers(void *sock_to_servers, int num_servers,  char **messages, int msg_size, 
        char *names[],  int n, ...) {
    va_list valist;
    int i =0, j;

    va_start(valist, n);
     
    void **values = (void *)malloc(n*sizeof(void *));
    zframe_t **frames = (zframe_t *)malloc( (n+1)*sizeof(zframe_t *));
    assert(values!=NULL);
    assert(frames!=NULL);

    for(i=0; i < n; i++ ) {
        if( strcmp(names[i], "opnum")==0)   {
           values[i] = (void *)va_arg(valist, unsigned  int *); 
        }
        else
           values[i] = va_arg(valist, void *); 

        if( strcmp(names[i], "opnum")==0) {
            frames[i]= zframe_new((const void *)values[i], sizeof(unsigned int));
        }
        else {
            frames[i]= zframe_new(values[i], strlen((char *)values[i]));
        }
    }

    va_end(valist);

    // it to all servers in a round robin fashion
    if( DEBUG_MODE) printf("\tsending ..\n");
    for(i=0; i < num_servers; i++) {
       for(j=0; j < n; j++) {
          if(DEBUG_MODE) {
            if( strcmp(names[j], "opnum")==0)  {
              if(DEBUG_MODE) { printf("\t\tFRAME%d :%s  %d\n", j, names[j], *((unsigned int *)values[j]) ); }
            } else {
              if(DEBUG_MODE) {printf("\t\tFRAME%d :%s  %s\n", j, names[j], (char *)values[j]); }
            }
          }
         zframe_send( &frames[j], sock_to_servers, ZFRAME_REUSE + ZFRAME_MORE);
       }
        // a different coded element for each different server
       frames[n]= zframe_new(messages[i], msg_size);
       if(DEBUG_MODE) printf("\t\tFRAME%d :%s  %d\n", n, "payload",  msg_size );
       zframe_send( &frames[n], sock_to_servers, ZFRAME_REUSE);
       if(DEBUG_MODE)  printf("\n");
    }

    if( values!=NULL) free(values); 

    for(i=0; i < n+1; i++ ) {
       zframe_destroy(frames+i);
    }
    if( frames!=NULL) free(frames);
}


