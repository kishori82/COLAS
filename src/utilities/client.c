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

extern int s_interrupted;

void send_multicast_servers(void *sock_to_servers, int num_servers, char *names[],  int n, ...) {
    va_list valist;
    int i =0, j;

    va_start(valist, n);
     
    void **values = (void *)malloc(n*sizeof(void *));
    zframe_t **frames = (zframe_t **)malloc(n*sizeof(zframe_t *));
    assert(values!=NULL);
    assert(frames!=NULL);
    for(i=0; i < n; i++ ) {

        if( strcmp(names[i], OPNUM)==0)   {
           values[i] = (void *)va_arg(valist, unsigned  int *); 
        }
        else
           values[i] = va_arg(valist, void *); 

        if( strcmp(names[i], OPNUM)==0) {
             
            frames[i]= zframe_new((const void *)values[i], sizeof(unsigned int));
        }
        else {
            frames[i]= zframe_new(values[i], strlen((char *)values[i]));
        }
    }
    va_end(valist);

    // it to all servers in a round robin fashion
    if(DEBUG_MODE)  printf("\n");
    if( DEBUG_MODE) printf("\t\tsending ..\n");
    for(i=0; i < num_servers; i++) {

       for(j=0; j < n-1; j++) {
          if(DEBUG_MODE) {
            if( strcmp(names[j], OPNUM)==0)  
               printf("\t\t\tFRAME%d :%s  %lu\n",j, names[j], *((unsigned int *)values[j]) );
            else if( strcmp(names[j], PAYLOAD)==0)  
               printf("\t\t\tFRAME%d :%s  %lu\n", j, names[j],  strlen((char *)values[j]) );
            else
               printf("\t\t\tFRAME%d :%s  %s\n", j, names[j],   (char *)values[j]);
             
            zframe_send( &frames[j], sock_to_servers, ZFRAME_REUSE + ZFRAME_MORE);
         }
       }

       if(DEBUG_MODE) {
         if( strcmp(names[j], OPNUM)==0)  
            printf("\t\t\tFRAME%d :%s  %lu\n", j, names[j],   *((unsigned int *)values[j]) );
         else if( strcmp(names[j], PAYLOAD)==0)  
            printf("\t\t\tFRAME%d :%s  %lu\n", j, names[j],  strlen((char *)values[j]) );
         else
            printf("\t\t\tFRAME%d :%s  %s\n", j, names[j],   (char *)values[j]);
       }
      zframe_send( &frames[j], sock_to_servers, ZFRAME_REUSE);
      if(DEBUG_MODE)  printf("\n");
   }

    printf("\n");
    if(DEBUG_MODE)  printf("\n");

    if( values!=NULL) free(values); 

    for(i=0; i < n; i++ ) {
       zframe_destroy(&frames[i]);
    }
    if( frames!=NULL) free(frames);
}

void send_multisend_servers(void *sock_to_servers, int num_servers,  uint8_t **messages, int msg_size, 
        char *names[],  int n, ...) {
    va_list valist;
    int i =0, j;
    void **values = (void *)malloc(n*sizeof(void *));
    assert(values!=NULL);

    zframe_t **frames = (zframe_t **)malloc( (n+1)*sizeof(zframe_t *));
    assert(frames!=NULL);

   
    va_start(valist, n);
    for(i=0; i < n; i++ ) {

        if( strcmp(names[i], OPNUM)==0)   {
           values[i] = (void *)va_arg(valist, unsigned  int *); 
        }
        else
           values[i] = va_arg(valist, void *); 

        if( strcmp(names[i], OPNUM)==0) {
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
            if( strcmp(names[j], OPNUM)==0)  {
              if(DEBUG_MODE) { printf("\t\t\tFRAME%d :%s  %d\n", j, names[j], *((unsigned int *)values[j]) ); }
            } else {
              if(DEBUG_MODE) {printf("\t\t\tFRAME%d :%s  %s\n", j, names[j], (char *)values[j]); }
            }
          }
         zframe_send( &frames[j], sock_to_servers, ZFRAME_REUSE + ZFRAME_MORE);
       }
        // a different coded element for each different server
       frames[n]= zframe_new(messages[i], msg_size);
       //char pay[] = "dkjfkajdfjdakfjdkassjfjdkdfjkasjfkdjasfjkasjdfkljasdkjkajsdfjaksdfjajkadfdjkfjdkkakdjfkjakdfjkdjasfkjdasfjdajfldjasfkljdsklfjajdfkljdkajfkldjasfjasdklfjkldasjfkldjasfkljdsfjkajfdajfkdjkljafkdjfkddfdfdasfjdkasjfdjafjadlfjdasljfdklafdajfklasdfjldasfjkladjfdjaskfdkasjfkdajkdaskjffjdaskfkdasjflkdajfdkjjfasddkjfkajdfjdakfjdkassjfjdkdfjkasjfkdjasfjkasjdfkljasdkjkajsdfjaksdfjajkadfdjkfjdkkakdjfkjakdfjkdjasfkjdasfjdajfldjasfkljdsklfjajdfkljdkajfkldjasfjasdklfjkldasjfkldjasfkljdsfjkajfdajfkdjkljafkdjfkddfdfdasfjdkasjfdjafjadlfjdasljfdklafdajfklasdfjldasfjkladjfdjaskfdkasjfkdajkdaskjffjdaskfkdasjflkdajfdkjjfasddkjfkajdfjdakfjdkassjfjdkdfjkasjfkdjasfjkasjdfkljasdkjkajsdfjaksdfjajkadfdjkfjdkkakdjfkjakdfjkdjasfkjdasfjdajfldjasfkljdsklfjajdfkljdkajfkldjasfjasdklfjkldasjfkldjasfkljdsfjkajfdajfkdjkljafkdjfkddfdfdasfjdkasjfdjafjadlfjdasljfdklafdajfklasdfjldasfjkladjfdjaskfdkasjfkdajkdaskjffjdaskfkdasjflkdajfdkjjfasddkjfkajdfjdakfjdkassjfjdkdfjkasjfkdjasfjkasjdfkljasdkjkajsdfjaksdfjajkadfdjkfjdkkakdjfkjakdfjkdjasfkjdasfjdajfldjasfkljdsklfjajdfkljdkajfkldjasfjasdklfjkldasjfkldjasfkljdsfjkajfdajfkdjkljafkdjfkddfdfdasfjdkasjfdjafjadlfjdasljfdklafdajfklasdfjldasfjkladjfdjaskfdkasjfkdajkdaskjffjdaskfkdasjflkdajfdkjjfasddkjfkajdfjdakfjdkassjfjdkdfjkasjfkdjasfjkasjdfkljasdkjkajsdfjaksdfjajkadfdjkfjdkkakdjfkjakdfjkdjasfkjdasfjdajfldjasfkljdsklfjajdfkljdkajfkldjasfjasdklfjkldasjfkldjasfkljdsfjkajfdajfkdjkljafkdjfkddfdfdasfjdkasjfdjafjadlfjdasljfdklafdajfklasdfjldasfjkladjfdjaskfdkasjfkdajkdaskjffjdaskfkdasjflkdajfdkjjfasddkjfkajdfjdakfjdkassjfjdkdfjkasjfkdjasfjkasjdfkljasdkjkajsdfjaksdfjajkadfdjkfjdkkakdjfkjakdfjkdjasfkjdasfjdajfldjasfkljdsklfjajdfkljdkajfkldjasfjasdklfjkldasjfkldjasfkljdsfjkajfdajfkdjkljafkdjfkddfdfdasfjdkasjfdjafjadlfjdasljfdklafdajfklasdfjldasfjkladjfdjaskfdkasjfkdajkdaskjffjdaskfkdasjflkdajfdkjjfasd";

   //    frames[n]= zframe_new( payload, strlen(pay));
       if(DEBUG_MODE) printf("\t\t\tFRAME%d :%s  %d\n", n, PAYLOAD,  msg_size );
       zframe_send( &frames[n], sock_to_servers, ZFRAME_REUSE);
       if(DEBUG_MODE)  printf("\n");
    }

    if( values!=NULL) free(values); 

/*
    for(i=0; i < n+1; i++ ) {
       zframe_destroy(frames+i);
    }
*/
    if( frames!=NULL) free(frames);
}

zhash_t *receive_message_frames_at_client(zmsg_t *msg, zlist_t *names)  {
     char algorithm_name[100];
     char object_name[100];
     char phase_name[100];
     zhash_t *frames = zhash_new();

     zframe_t *object_name_frame= zmsg_pop (msg);
     zhash_insert(frames, OBJECT, (void *)object_name_frame);
     get_string_frame(object_name, frames, OBJECT);
     if( names!= NULL) zlist_append(names, OBJECT);
 
     zframe_t *algorithm_frame= zmsg_pop (msg);
     zhash_insert(frames, ALGORITHM, (void *)algorithm_frame);
     get_string_frame(algorithm_name, frames, ALGORITHM);
     if( names!= NULL) zlist_append(names, ALGORITHM);

     zframe_t *phase_frame= zmsg_pop (msg);
     zhash_insert(frames, PHASE, (void *)phase_frame);
     get_string_frame(phase_name, frames, PHASE);
     if( names!= NULL) zlist_append(names, PHASE);

     if( strcmp(algorithm_name, "ABD") ==0 ) {
         zframe_t *opnum_frame= zmsg_pop (msg);
         zhash_insert(frames, OPNUM, (void *)opnum_frame);
         if( names!= NULL) zlist_append(names, OPNUM);

         if( strcmp(phase_name, GET_TAG) ==0 ) {
           zframe_t *tag_frame= zmsg_pop (msg);
           zhash_insert(frames, TAG, (void *)tag_frame);
           if( names!= NULL) zlist_append(names, TAG);
         }

         if( strcmp(phase_name, WRITE_VALUE) ==0 ) {

           zframe_t *tag_frame= zmsg_pop (msg);
           zhash_insert(frames, TAG, (void *)tag_frame);
           if( names!= NULL) zlist_append(names, TAG);
         }

         if( strcmp(phase_name, GET_TAG_VALUE) ==0 ) {
           zframe_t *tag_frame= zmsg_pop (msg);
           zhash_insert(frames, TAG, (void *)tag_frame);
           if( names!= NULL) zlist_append(names, TAG);

           zframe_t *payload_frame= zmsg_pop (msg);
           zhash_insert(frames, PAYLOAD, (void *)payload_frame);
           if( names!= NULL) zlist_append(names, PAYLOAD);
         }
     }

     if( strcmp(algorithm_name, SODAW) ==0 ) {
         if( strcmp(phase_name, WRITE_GET) ==0 ) {
           zframe_t *opnum_frame= zmsg_pop (msg);
           zhash_insert(frames, OPNUM, (void *)opnum_frame);
           if( names!= NULL) zlist_append(names, OPNUM);

           zframe_t *tag_frame= zmsg_pop (msg);
           zhash_insert(frames, TAG, (void *)tag_frame);
           if( names!= NULL) zlist_append(names, TAG);
         }

         if( strcmp(phase_name, WRITE_PUT) ==0 ) {
           zframe_t *opnum_frame= zmsg_pop (msg);
           zhash_insert(frames, OPNUM, (void *)opnum_frame);
           if( names!= NULL) zlist_append(names, OPNUM);

           zframe_t *tag_frame= zmsg_pop (msg);
           zhash_insert(frames, TAG, (void *)tag_frame);
           if( names!= NULL) zlist_append(names, TAG);
         }

         if( strcmp(phase_name, READ_VALUE) ==0 ) {

           zframe_t *tag_frame= zmsg_pop (msg);
           zhash_insert(frames, TAG, (void *)tag_frame);
           if( names!= NULL) zlist_append(names, TAG);

           zframe_t *payload_frame= zmsg_pop (msg);
           zhash_insert(frames, PAYLOAD, (void *)payload_frame);
           if( names!= NULL) zlist_append(names, PAYLOAD);
         }
     }
     return frames;
}


