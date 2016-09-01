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
#include "sodaw_client.h"
#include "client.h"
#include <base64.h>


extern int s_interrupted;

#ifdef ASLIBRARY

#define WRITE_VALUE "WRITE_VALUE"
#define GET_TAG "GET_TAG"
#define GET_TAG_VALUE "GET_TAG_VALUE"


// this fethers the max tag
TAG SODAW_write_get_or_read_get_phase(char *obj_name, unsigned int op_num, 
                      zsock_t *sock_to_servers,  char **servers, 
                          unsigned int num_servers, char *port)
{

    // send out the messages to all servers

    char buf[400];
    char algorithm[64];
    char phase[64];
    char tag_str[64];
    char buf1[400];
    int round;

    zmq_pollitem_t items [] = { { sock_to_servers, 0, ZMQ_POLLIN, 0 } };

    int i; 
    zframe_t *obj_name_frame = zframe_new(obj_name, strlen(obj_name));
    zframe_t *algo = zframe_new("SODAW", 3);
    zframe_t *phase_frame = zframe_new(GET_TAG, 7);
    zframe_t *op_num_frame = zframe_new((const void *)&op_num, sizeof(int));

    for(i=0; i < num_servers; i++) {
       zframe_send(&obj_name_frame, sock_to_servers, ZFRAME_REUSE + ZFRAME_MORE);
       zframe_send(&algo, sock_to_servers, ZFRAME_REUSE + ZFRAME_MORE);
       zframe_send(&phase_frame, sock_to_servers, ZFRAME_REUSE + ZFRAME_MORE);
       zframe_send(&op_num_frame, sock_to_servers, ZFRAME_REUSE);
       printf("     \tsending to server %d\n",i);
    }

    zframe_destroy(&obj_name_frame);
    zframe_destroy(&algo);
    zframe_destroy(&phase_frame);
    zframe_destroy(&op_num_frame);

//    zframe_destroy (&payloadf);
    unsigned int majority =  ceil(((float)num_servers+1)/2);
//     zmq_pollitem_t items [] = { { sock_to_servers, 0, ZMQ_POLLIN, 0 } };
     unsigned int responses =0;
     zlist_t *tag_list = zlist_new();
     
     TAG *tag;
     while (true) {
        //  Tick once per second, pulling in arriving messages
            
           // zmq_pollitem_t items [] = { { sock_to_servers, 0, ZMQ_POLLIN, 0 } };
            printf("      \treceiving data\n");
            int rc = zmq_poll(items, 1, -1);
            if(rc < 0 ||  s_interrupted ) {
                printf("Interrupted!\n");
                exit(0);
            }
           // zclock_sleep(300); 
            if (items [0].revents & ZMQ_POLLIN) {
                zmsg_t *msg = zmsg_recv (sock_to_servers);

                //object
                zframe_t *object_frame = zmsg_pop(msg);
                _zframe_str(object_frame, buf);
                printf("\t\tobject    : %s\n",buf);

                // algorithm
                zframe_t *algorithm_frame = zmsg_pop(msg);
                _zframe_str(algorithm_frame, buf);
                printf("\t\talgorithm : %s\n",buf);

                // phase
                zframe_t *phase_frame = zmsg_pop(msg);
                _zframe_str(phase_frame, phase);
                printf("\t\tphase     : %s\n",phase);

                // operation number
                zframe_t *op_num_frame = zmsg_pop(msg);
                _zframe_int(op_num_frame, &round);
                printf("\t\tOP_NUM    : %d\n", round);

               //tag 
                zframe_t *tag_str_frame = zmsg_pop(msg);
                _zframe_str(tag_str_frame, tag_str);
                printf("\t\tTAG    : %s\n", tag_str);
                  
                zframe_destroy(&object_frame);
                zframe_destroy(&algorithm_frame);
                zframe_destroy(&phase_frame);
                zframe_destroy(&op_num_frame);
                zframe_destroy(&tag_str_frame);

                zmsg_destroy (&msg);
                if(round==op_num && strcmp(phase, GET_TAG)==0) {
                   responses++;

                   // add tag to list                
                   tag = (TAG *)malloc(sizeof(TAG));
                   string_to_tag(tag_str, tag);
                   zlist_append(tag_list, (void *)tag);

                   if(responses >= majority) break;
                   //if(responses >= num_servers) break;
                }
                else{
                     printf("   OLD MESSAGES : %s  %d\n", phase, op_num);

                }
            }
     }
   //comute the max tag now and return 
     TAG max_tag = get_max_tag(tag_list);

     free_items_in_list(tag_list);
     zlist_destroy(&tag_list);
     return  max_tag;
}

// this fetches the max tag and value

TAG SODAW_read_value(
            char *obj_name, 
            unsigned int op_num, 
            zsock_t *sock_to_servers,  
            char **servers, 
            unsigned int num_servers, 
            char *port, 
            TAG *max_tag, 
            char **max_value
        )
{

    // send out the messages to all servers

    char buf[400];
    char algorithm[64];
    char phase[64];
    char tag_str[64];
    char buf1[400];
    char *value=NULL;
    int round;
    int size;

    printf("hello\n");

    TAG *tag;

    zmq_pollitem_t items [] = { { sock_to_servers, 0, ZMQ_POLLIN, 0 } };

    int i; 
    zframe_t *obj_name_frame = zframe_new(obj_name, strlen(obj_name));
    zframe_t *algo = zframe_new("SODAW", 3);
    zframe_t *phase_frame = zframe_new(GET_TAG_VALUE, 13);
    zframe_t *op_num_frame = zframe_new((const void *)&op_num, sizeof(int));

    for(i=0; i < num_servers; i++) {
       zframe_send(&obj_name_frame, sock_to_servers, ZFRAME_REUSE + ZFRAME_MORE);
       zframe_send(&algo, sock_to_servers, ZFRAME_REUSE + ZFRAME_MORE);
       zframe_send(&phase_frame, sock_to_servers, ZFRAME_REUSE + ZFRAME_MORE);
       zframe_send(&op_num_frame, sock_to_servers, ZFRAME_REUSE);
       printf("     \tsending to server %d\n",i);
    }

    zframe_destroy(&obj_name_frame);
    zframe_destroy(&algo);
    zframe_destroy(&phase_frame);
    zframe_destroy(&op_num_frame);

//    zframe_destroy (&payloadf);
    unsigned int majority =  ceil(((float)num_servers+1)/2);
//     zmq_pollitem_t items [] = { { sock_to_servers, 0, ZMQ_POLLIN, 0 } };
     unsigned int responses =0;
     zlist_t *tag_list = zlist_new();
     
     while (true) {
        //  Tick once per second, pulling in arriving messages
            
           // zmq_pollitem_t items [] = { { sock_to_servers, 0, ZMQ_POLLIN, 0 } };
            printf("      \treceiving data\n");
            int rc = zmq_poll(items, 1, -1);
            if(rc < 0 ||  s_interrupted ) {
                printf("Interrupted!\n");
                exit(0);
            }
           // zclock_sleep(300); 
            if (items [0].revents & ZMQ_POLLIN) {
                zmsg_t *msg = zmsg_recv (sock_to_servers);

                //object name
                zframe_t *object_frame = zmsg_pop(msg);
                _zframe_str(object_frame, buf);
                printf("\t\tobject    : %s\n",buf);

                // algorithm
                zframe_t *algorithm_frame = zmsg_pop(msg);
                _zframe_str(algorithm_frame ,buf);
                printf("\t\talgorithm : %s\n",buf);

                // phase
                zframe_t *phase_frame = zmsg_pop(msg);
                _zframe_str(phase_frame, phase);
                printf("\t\tphase     : %s\n",phase);

                // operation number
                zframe_t *op_num_frame = zmsg_pop(msg);
                _zframe_int(op_num_frame, &round);
                printf("\t\tOP_NUM    : %d\n", round);

                //tag
                zframe_t *tag_str_frame = zmsg_pop(msg);
                _zframe_str(tag_str_frame, tag_str);
                printf("\t\tTAG    : %s\n", tag_str);
                

                //value
                zframe_t *value_frame = zmsg_pop(msg);
                size = zframe_size(value_frame);    
                if( value !=NULL) free(value);
                value = (char *)malloc(  (zframe_size(value_frame) + 1)*sizeof(char) );
                _zframe_str(value_frame, value);
                printf("\t\tVALUE    : %s\n", value);

                zframe_destroy(&object_frame);
                zframe_destroy(&algorithm_frame);
                zframe_destroy(&phase_frame);
                zframe_destroy(&op_num_frame);
                zframe_destroy(&tag_str_frame);
                zframe_destroy(&value_frame);

                zmsg_destroy (&msg);

                if(round==op_num && strcmp(phase, GET_TAG_VALUE)==0) {
                   responses++;
                   // add tag to list                
                   tag = (TAG *)malloc(sizeof(TAG));
                   string_to_tag(tag_str, tag);
                   zlist_append(tag_list, (void *)tag);

                   *max_value = value;
                   if(responses >= majority) break;
                   //if(responses >= num_servers) break;
                }
                else{
                     printf("   OLD MESSAGES : %s  %d\n", phase, op_num);

                }
            }
     }
   //comute the max tag now and return 
     *max_tag = get_max_tag(tag_list);

     free_items_in_list(tag_list);
     zlist_destroy(&tag_list);
}


// this is the write tag value phase of SODAW
TAG SODAW_write_put_phase(  
                      char *obj_name,
                      char *writer_id, 
                      unsigned int op_num, 
                      zsock_t *sock_to_servers,  
                      char **servers, 
                      unsigned int num_servers, 
                      char *port, 
                      char *payload, 
                      int size, 
                      TAG max_tag   // for read it is max and for write it is new
                   )
{
    // send out the messages to all servers
    char buf[400];
    char algorithm[64];
    char phase[64];
    char tag_str[64];
    char buf1[400];
    char *value;
    

    int round;

    zmq_pollitem_t items [] = { { sock_to_servers, 0, ZMQ_POLLIN, 0 } };

    int i; 
    zframe_t *obj_name_frame = zframe_new(obj_name, strlen(obj_name));
    zframe_t *algo = zframe_new("SODAW", 3);
    zframe_t *phase_frame = zframe_new(WRITE_VALUE, 11);
    zframe_t *op_num_frame = zframe_new((const void *)&op_num, sizeof(int));


    tag_to_string(max_tag, tag_str); 
    zframe_t *tag_frame = zframe_new(tag_str, strlen(tag_str));

    zframe_t *payload_frame = zframe_new(payload, size);

    for(i=0; i < num_servers; i++) {
       zframe_send(&obj_name_frame, sock_to_servers, ZFRAME_REUSE + ZFRAME_MORE);
       zframe_send(&algo, sock_to_servers, ZFRAME_REUSE + ZFRAME_MORE);
       zframe_send(&phase_frame, sock_to_servers, ZFRAME_REUSE + ZFRAME_MORE);
       zframe_send(&op_num_frame, sock_to_servers, ZFRAME_REUSE+ ZFRAME_MORE);
       zframe_send(&tag_frame, sock_to_servers, ZFRAME_REUSE+ ZFRAME_MORE);
       zframe_send(&payload_frame, sock_to_servers, ZFRAME_REUSE);
       printf("     \tsending to server %d\n",i);
    }

    zframe_destroy(&obj_name_frame);
    zframe_destroy(&algo);
    zframe_destroy(&phase_frame);
    zframe_destroy(&op_num_frame);
    zframe_destroy(&tag_frame);
    zframe_destroy(&payload_frame);

    unsigned int majority =  ceil((num_servers+1)/2);
     unsigned int responses =0;
     int j =0;
     zlist_t *tag_list = zlist_new();
     
     TAG *tag;
     while (true) {
        //  Tick once per second, pulling in arriving messages
            
           // zmq_pollitem_t items [] = { { sock_to_servers, 0, ZMQ_POLLIN, 0 } };
            printf("      \twaiting ack\n");
            int rc = zmq_poll(items, 1, -1);
            if(rc < 0 ||  s_interrupted ) {
                printf("Interrupted!\n");
                exit(0);
            }
           // zclock_sleep(300); 
            if (items [0].revents & ZMQ_POLLIN) {
                zmsg_t *msg = zmsg_recv (sock_to_servers);

                //object name
                zframe_t *object_frame = zmsg_pop(msg);
                _zframe_str(object_frame, buf);
                printf("\t\tobject    : %s\n",buf);

                // algorithm
                zframe_t *algorithm_frame = zmsg_pop(msg);
                _zframe_str(algorithm_frame, buf);
                printf("\t\talgorithm : %s\n",buf);

                // phase
                zframe_t *phase_frame = zmsg_pop(msg);
                _zframe_str(phase_frame, phase);
                printf("\t\tphase     : %s\n",phase);

                // operation number
                zframe_t *op_num_frame = zmsg_pop(msg);
                _zframe_int(op_num_frame, &round);
                printf("\t\tOP_NUM    : %d\n", round);

                // tag string
                zframe_t *tag_str_frame = zmsg_pop(msg);
                _zframe_str(tag_str_frame, tag_str);
                printf("\t\tTAG STRING    : %s\n", tag_str);


                zframe_destroy(&object_frame);
                zframe_destroy(&algorithm_frame);
                zframe_destroy(&phase_frame);
                zframe_destroy(&op_num_frame);
                zframe_destroy(&tag_str_frame);

                zmsg_destroy (&msg);
                if(round==op_num && strcmp(phase, WRITE_VALUE)==0) {
                   responses++;

                   // add tag to list                
                   tag = (TAG *)malloc(sizeof(TAG));
                   string_to_tag(tag_str, tag);
                   zlist_append(tag_list, (void *)tag);

                   if(responses >= majority) break;
                   //if(responses >= num_servers) break;
                }
                else{
                     printf("   OLD MESSAGES : %s  %d\n", phase, op_num);

                }
            }
     }
     return max_tag;
}


// SODAW write
bool SODAW_write(
                char *obj_name,
                char *writer_id, 
                unsigned int op_num ,
                char *payload, 
                unsigned int size, 
                char *servers_str, 
                char *port

             )
{
    s_catch_signals();
    int j;
    char *myb64 = (char *)malloc(strlen(payload));
    b64_decode(payload, myb64);
    int num_servers = count_num_servers(servers_str);
    char **servers = create_server_names(servers_str);

#ifdef DEBUG_MODE
    printf("Obj name       : %s\n",obj_name);
    printf("Writer name    : %s\n",writer_id);
    printf("Operation num  : %d\n",op_num);
    printf("Size           : %d\n", size);
    printf("Size of        : %u\n", (unsigned int)strlen(payload));

    printf("Encoded string  : %s\n", payload);
    printf("Server string   : %s\n", servers_str);
    printf("Port to Use     : %s\n", port);

    printf("Num of Servers  : %d\n",num_servers);

//    printf("Decoded string  : %s\n", myb64);
    for(j=0; j < num_servers; j++) {
        printf("\tServer : %s\n", servers[j]);
    }
    printf("\n");
#endif

    free(myb64);
    zctx_t *ctx  = zctx_new();
    void *sock_to_servers = zsocket_new(ctx, ZMQ_DEALER);
    zctx_set_linger(ctx, 0);
    assert (sock_to_servers);

    zsocket_set_identity(sock_to_servers,  writer_id);
    for(j=0; j < num_servers; j++) {    
       char *destination = create_destination(servers[j], port);
       int rc = zsocket_connect(sock_to_servers, (const char *)destination);
       assert(rc==0);
       free(destination);
    }

   printf("WRITE %d\n", op_num);
   printf("     MAX_TAG (WRITER)\n");

   TAG max_tag=  SODAW_write_get_or_read_get_phase(
                      obj_name,  
                      op_num, 
                      sock_to_servers, 
                      servers, 
                      num_servers, 
                      port
                  );

    TAG new_tag;
    new_tag.z = max_tag.z + 1;
    strcpy(new_tag.id, writer_id);

   printf("     WRITE_VALUE (WRITER)\n");
   SODAW_write_put_phase(
                          obj_name, 
                          writer_id,  
                          op_num, 
                          sock_to_servers, servers,
                          num_servers, 
                          port, 
                          payload, 
                          size, 
                          max_tag
                        );

    zsocket_destroy(ctx, sock_to_servers);
    zctx_destroy(&ctx);


    return true;
}


char *SODAW_read(
                char *obj_name,
                char *writer_id, 
                unsigned int op_num ,
//                char *payload, 
                char *servers_str, 
                char *port
             )
{
    s_catch_signals();
    int j;
    int num_servers = count_num_servers(servers_str);

    printf("hello=======1\n");
    char **servers = create_server_names(servers_str);
#ifdef DEBUG_MODE
    printf("Obj name       : %s\n",obj_name);
    printf("Writer name    : %s\n",writer_id);
    printf("Operation num  : %d\n",op_num);

   /* char *myb64 = (char *)malloc(strlen(payload));
    b64_decode(payload, myb64);
    printf("Encoded string  : %s\n", payload);
    free(myb64);
*/
    printf("Server string   : %s\n", servers_str);
    printf("Port to Use     : %s\n", port);
    printf("Num of Servers  : %d\n",num_servers);

//    printf("Decoded string  : %s\n", myb64);
    for(j=0; j < num_servers; j++) {
        printf("\tServer : %s\n", servers[j]);
    }
    printf("\n");
#endif

    
    printf("hello=========2\n");
    zctx_t *ctx  = zctx_new();
    void *sock_to_servers = zsocket_new(ctx, ZMQ_DEALER);
    zctx_set_linger(ctx, 0);
    assert (sock_to_servers);

    zsocket_set_identity(sock_to_servers,  writer_id);
    printf("hello=========2\n");
    for(j=0; j < num_servers; j++) {    
       char *destination = create_destination(servers[j], port);
       int rc = zsocket_connect(sock_to_servers, destination);
       assert(rc==0);
       free(destination);
    }

   printf("READ %d\n", op_num);
   printf("     MAX_TAG_VALUE (READER)\n");

   char *payload;

   TAG max_tag=  SODAW_write_get_or_read_get_phase(
                      obj_name,  
                      op_num, 
                      sock_to_servers, 
                      servers, 
                      num_servers, 
                      port
                  );

   printf("\tmax tag (%d,%s)\n\n", max_tag.z, max_tag.id);

   printf("     WRITE_VALUE (READER)\n");
   int size = strlen(payload);
   SODAW_write_put_phase(obj_name, writer_id,  op_num, sock_to_servers, servers,
                     num_servers, port, payload, size, max_tag);

    zsocket_destroy(ctx, sock_to_servers);
    zctx_destroy(&ctx);


    return payload;
}




#endif


//  The main thread simply starts several clients and a server, and then
//  waits for the server to finish.
//#define ASMAIN

#ifdef ASMAIN

int main (void)
{
   int i ; 
   
   char *payload = (char *)malloc(100000000*sizeof(char));
   unsigned int size = 100000000*sizeof(char);

/*
   char *servers[]= {
                     "172.17.0.7", "172.17.0.5", 
                     "172.17.0.4", "172.17.0.6",
                     "172.17.0.3"
                   };

*/

/*
   char *servers[] = {
"172.17.0.22", "172.17.0.21", "172.17.0.18", "172.17.0.17", "172.17.0.20", "172.17.0.16", "172.17.0.19", "172.17.0.15", "172.17.0.14", "172.17.0.13", "172.17.0.12", "172.17.0.11", "172.17.0.10", "172.17.0.9", "172.17.0.7", "172.17.0.8", "172.17.0.6", "172.17.0.5", "172.17.0.4", "172.17.0.3"
                     };
*/

   char *servers[]= {
                     "172.17.0.2"
                     };


   unsigned int num_servers = 1;
   char port[]= {"8081"};

   char writer_id[] = { "writer_1"};
   char obj_name[] = {"object"};

   unsigned int op_num;
   s_catch_signals();

  for( i=0; i < 5; i++) {
    printf("\nWRITE %d\n", i);
    //SODAW_write(obj_name, writer_id, i,  payload, size, servers, port);
  }

//   zclock_sleep(50*1000); 
   return 0;
}




#endif
