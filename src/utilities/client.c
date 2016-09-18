//  Asynchronous client-to-server (DEALER to ROUTER)
//
//  While this example runs in a single process, that is to make
//  it easier to start and stop the example. Each task has its own
//  context and conceptually acts as a separate process.

#include "client.h"





int
special_zframe_send (zframe_t **self_p, void *dest, int flags)
{
    assert (dest);
    assert (self_p);

    void *handle = zsock_resolve (dest);
    if (*self_p) {
        zframe_t *self = *self_p;
        assert (zframe_is (self));

        int send_flags = (flags & ZFRAME_MORE)? ZMQ_SNDMORE: 0;
        send_flags |= (flags & ZFRAME_DONTWAIT)? ZMQ_DONTWAIT: 0;
        if (flags & ZFRAME_REUSE) {
            zmq_msg_t copy;
            zmq_msg_init (&copy);
            if (zmq_msg_copy (&copy, zframe_data(self)))
                return -1;
            if (zmq_sendmsg (handle, &copy, send_flags) == -1) {
                zmq_msg_close (&copy);
                return -2;
            }
        }
        else {
            if (zmq_sendmsg (handle, zframe_data(&self), send_flags) >= 0)
                zframe_destroy (self_p);
            else
                return -3;
        }
    }   
    return 0;
}




#define DEBUG_MODE  1

extern int s_interrupted;

void send_multicast_servers(void *sock_to_servers, int num_servers, char *names[],  int n, ...) {
    va_list valist;
    int i =0, j;

    va_start(valist, n);
     
    void **values = (void **)malloc(n*sizeof(void *));
    for(i=0; i < n; i++) 
       values[i] = (void *)malloc(10*sizeof(void));

    zframe_t **frames = (zframe_t **)malloc(n*sizeof(zframe_t *));
    assert(values!=NULL);
    assert(frames!=NULL);
    for(i=0; i < n; i++ ) {

        if( strcmp(names[i], OPNUM)==0)   {
           values[i] = (void *)va_arg(valist, unsigned  int *); 
           frames[i]= zframe_new( (const void *)values[i], sizeof(unsigned int));
           //frames[i]= zframe_new((const void *)values[i], sizeof(*values[i]));
        }
        else {
           values[i] = va_arg(valist, char *); 
           frames[i]= zframe_new(values[i], strlen((char *)values[i]));
        }
    }
    va_end(valist);

    // it to all servers in a round robin fashion
    int rc;
    if(DEBUG_MODE)  printf("\n");
    if(DEBUG_MODE) printf("\t\tsending ..\n");
    for(i=0; i < num_servers; i++) {
       printf("\t\t\tserver : %d\n", i);
       for(j=0; j < n-1; j++) {
          if(DEBUG_MODE) {
            if( strcmp(names[j], OPNUM)==0)  
               printf("\t\t\tFRAME%d :%s  %u\n",j, names[j], *((unsigned int *)values[j]) );
            else if( strcmp(names[j], PAYLOAD)==0)  
               printf("\t\t\tFRAME%d :%s  %lu\n", j, names[j],  strlen((char *)values[j]) );
            else
               printf("\t\t\tFRAME%d :%s  %s\n", j, names[j],   (char *)values[j]);
             
           rc = zframe_send(&frames[j], sock_to_servers, ZFRAME_REUSE +  ZFRAME_MORE);
          /* rc = zmq_send(sock_to_servers, (char *)zframe_data(frames[j]), zframe_size(frames[j]), ZMQ_SNDMORE);
           char empty[1];
           rc = zmq_send(sock_to_servers, empty, 0,  ZMQ_SNDMORE);
*/
//		 rc = special_zframe_send( &frames[j], sock_to_servers,  ZFRAME_REUSE + ZFRAME_MORE);


           if( rc < 0) {
              printf("ERROR: %d\n", rc);
				      exit(-1);
           }


           printf("\t\t\tSIZE%d :%d  %d\n", j, rc,  zframe_size(frames[j]));

           assert(rc!=-1);
             
         }
      }


      rc = zframe_send(&frames[j], sock_to_servers, ZFRAME_REUSE + ZFRAME_DONTWAIT);

      if(DEBUG_MODE) {
         if( strcmp(names[j], OPNUM)==0)  
            printf("\t\t\tFRAME%d :%s  %u\n", j, names[j],   *((unsigned int *)values[j]) );
         else if( strcmp(names[j], PAYLOAD)==0)  
            printf("\t\t\tFRAME%d :%s  %lu\n", j, names[j],  strlen((char *)values[j]) );
         else
            printf("\t\t\tFRAME%d :%s  %s\n", j, names[j],   (char *)values[j]);
      }

/*
     if( (rc=zframe_send( &frames[j], sock_to_servers,  ZFRAME_REUSE ))==-1) {
        printf("ERROR: %s\n", zmq_strerror(errno));
				exit(-1);
     }
*/

      rc = zframe_send(&frames[j], sock_to_servers, ZFRAME_REUSE + ZFRAME_DONTWAIT);
    //  rc = zmq_send(sock_to_servers, zframe_data(frames[j]), zframe_size(frames[j]), 0);
      if( rc < 0) {
          printf("ERROR: %d\n", rc);
		 	  	exit(-1);
      }



/*
		 int src = special_zframe_send( &frames[j], sock_to_servers,  ZFRAME_REUSE);
     if( src < 0) {
        //printf("ERROR: %s\n", zmq_strerror(errno));
        printf("ERROR: %d\n", src);
				exit(-1);
     }
*/

/*
     rc = zframe_send( &frames[j], sock_to_servers, ZFRAME_REUSE);
     printf("\t\t\tSIZE%d :%d  %d\n", j, rc,  zframe_size(frames[j]));
     assert(rc!=-1);
*/

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

         if( strcmp(phase_name, READ_GET) ==0 ) {
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


