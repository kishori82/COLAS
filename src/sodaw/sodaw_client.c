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
#include <rlnc_rs.h>

#define DEBUG_MODE 1

extern int s_interrupted;

#ifdef ASLIBRARY

#define WRITE_GET "WRITE_GET"
#define WRITE_PUT "WRITE_PUT"
#define READ_COMPLETE "READ_COMPLETE"
#define READ_GET "READ_GET"
#define READ_VALUE "READ_VALUE"


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
    unsigned int round;

    zmq_pollitem_t items [] = { { sock_to_servers, 0, ZMQ_POLLIN, 0 } };

    char *types[] = {"object", "algorithm", "phase", "opnum"};
    send_multicast_servers(sock_to_servers, num_servers, types,  4, obj_name, "SODAW", WRITE_GET, &op_num) ;


    unsigned int majority =  ceil(((float)num_servers+1)/2);
     unsigned int responses =0;
     zlist_t *tag_list = zlist_new();
     
     TAG *tag;
     while (true) {
            
           // zmq_pollitem_t items [] = { { sock_to_servers, 0, ZMQ_POLLIN, 0 } };
            printf("\t\twaiting for data..\n");
            int rc = zmq_poll(items, 1, -1);
            if(rc < 0 ||  s_interrupted ) {
                printf("Interrupted!\n");
                exit(0);
            }
            printf("\t\treceived data\n");
            if (items [0].revents & ZMQ_POLLIN) {
                zmsg_t *msg = zmsg_recv (sock_to_servers);

                zlist_t *names = zlist_new();
                zhash_t* frames = receive_message_frames_at_client(msg, names);
  
                get_string_frame(phase, frames, "phase");
                round = get_int_frame(frames, "opnum");
                get_string_frame(tag_str, frames, "tag");

                if(round==op_num && strcmp(phase, WRITE_GET)==0) {
                   if(DEBUG_MODE) print_out_hash_in_order(frames, names);

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
                zmsg_destroy (&msg);
                zlist_purge(names);
                destroy_frames(frames);
            }
     }
   //comute the max tag now and return 
     TAG max_tag = get_max_tag(tag_list);

     free_items_in_list(tag_list);
     zlist_destroy(&tag_list);
     return  max_tag;
}

// this fetches the max tag and value
char *SODAW_read_value(
            char *obj_name, 
            unsigned int op_num, 
            zsock_t *sock_to_servers,  
            char **servers, 
            unsigned int num_servers, 
            char *port, 
            TAG read_tag
        )
{

    // send out the messages to all servers

    char buf[400];
    char algorithm[100];
    char phase[100];
    char tag_str[100];
    char buf1[400];
    char *value=NULL;
    unsigned int round;
    int size;

    zmq_pollitem_t items [] = { { sock_to_servers, 0, ZMQ_POLLIN, 0 } };

    char *types[] = {"object", "algorithm", "phase", "tag"};
    tag_to_string(read_tag, tag_str); 
    send_multicast_servers(sock_to_servers, num_servers, types,  4, obj_name, "SODAW", READ_VALUE,  tag_str) ;

    unsigned int majority =  ceil(((float)num_servers+1)/2);
     unsigned int responses =0;
     zlist_t *tag_list = zlist_new();
     
     zhash_t *received_data= zhash_new();
     while (true) {
        //  Tick once per second, pulling in arriving messages
            
           // zmq_pollitem_t items [] = { { sock_to_servers, 0, ZMQ_POLLIN, 0 } };
            printf("\t\twaiting for data..\n");
            int rc = zmq_poll(items, 1, -1);
            if(rc < 0 ||  s_interrupted ) {
                printf("Interrupted!\n");
                exit(0);
            }
            printf("\t\treceived data\n");
            if (items [0].revents & ZMQ_POLLIN) {
                zmsg_t *msg = zmsg_recv (sock_to_servers);

                zlist_t *names = zlist_new();
                zhash_t* frames = receive_message_frames_at_client(msg, names);
  
                get_string_frame(phase, frames, "phase");
                get_string_frame(tag_str, frames, "tag");
                TAG tag;
                get_tag_frame(frames, &tag);

                printf("done\n"); return NULL; 
                printf("phase and tag %s %s\n", phase, tag_str);
                if( zhash_lookup(received_data, tag_str)==NULL) {
                    zlist_t *a = zlist_new();
                    zhash_insert(received_data, tag_str,  (void *)a);
                }


                if(compare_tags(tag, read_tag) >=0 && strcmp(phase, READ_VALUE)==0) {
                   if(DEBUG_MODE) print_out_hash_in_order(frames, names);

                   void *payload_frame = zhash_lookup(frames, "payload");
                   char str[3000];
                   get_string_frame(str, frames, "payload");

                   printf("      \treceived data 1 %s %s\n", tag_str, str);

                   zlist_t *coded_elements = (zlist_t *)zhash_lookup(received_data, tag_str);
/*
                   printf("      \treceived data 1.1 %s   %d\n", tag_str, zhash_size(received_data));

                   printf("      \treceived data majority  1.1 %d %d\n", majority, zlist_size(coded_elements));
                   printf("      \treceived data majority  1.1 %d %d\n", majority, zlist_size(coded_elements));
                   printf("      \treceived data majority  1.1 %d %d\n", majority, zlist_size(coded_elements));
*/

                  // zlist_append( (zlist_t *)coded_elements, payload_frame); 
                   //if(coded_elements != NULL) zlist_append( (zlist_t *)coded_elements, payload_frame); 

                   printf("      \treceived data 2 %d\n", majority);

//                   char *decodeableKey = number_responses_at_least(received_data, majority);

                   printf("      \treceived data 3\n");
//                   if(decodeableKey!= NULL) break;
                }
                else{
                     printf("   OLD MESSAGES : %s  %d\n", phase, op_num);

                }
                zmsg_destroy (&msg);
                zlist_purge(names);
                destroy_frames(frames);
            }
     }
  // now we want to decode it
    // value to 
   //comute the max tag now and return 

 
    return value;
}

char *number_responses_at_least(zhash_t *received_data, unsigned int atleast){
     zlist_t *keys = zhash_keys(received_data);
     char *key;
     for(  key = (char *)zlist_first(keys);  key != NULL; key = (char *)zlist_next(keys)) {
           int count = zlist_size(zhash_lookup(received_data, key));         
           if( count >= atleast) {
              return key; 
           }
     }
     return NULL;
}


// this is the write tag value phase of SODAW
void SODAW_write_put_phase(  
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
    char buf[4000];
    char algorithm[100];
    char phase[100];
    char tag_str[100];
    char buf1[400];
    char *value;

    int N = num_servers;

    unsigned int majority =  ceil(((float)num_servers+1)/2);
    int K = majority;

    int symbol_size = 1024;

    
    ENCODED_DATA  encoded_data_info = encode(N, K, symbol_size, payload, strlen(payload), full_vector) ;
    //ENCODED_DATA  encoded_data_info = encode(N, K, symbol_size, payload, strlen(payload), reed_solomon) ;
//    destroy_encoded_data(encoded_data_info);

    unsigned int round;

    zmq_pollitem_t items [] = { { sock_to_servers, 0, ZMQ_POLLIN, 0 } };

    tag_to_string(max_tag, tag_str); 


    char *types[] = {"object", "algorithm", "phase", "opnum", "tag"};
    size =  encoded_data_info.num_blocks*encoded_data_info.encoded_symbol_size;

    send_multisend_servers(sock_to_servers, num_servers, encoded_data_info.encoded_raw_data, size,
             types,  5, obj_name, "SODAW", WRITE_PUT, &op_num, tag_str) ;

    int i;
    for(i=0; i < encoded_data_info.N; i++) {
       free(encoded_data_info.encoded_raw_data[i]);
    }
    free(encoded_data_info.encoded_raw_data);


     unsigned int responses =0;
     int j =0;
     zlist_t *tag_list = zlist_new();
     
     TAG *tag;
     while (true) {
           //  Tick once per second, pulling in arriving messages
            
           // zmq_pollitem_t items [] = { { sock_to_servers, 0, ZMQ_POLLIN, 0 } };
            printf("\t\twaiting for data..\n");
            int rc = zmq_poll(items, 1, -1);
            if(rc < 0 ||  s_interrupted ) {
                printf("Interrupted!\n");
                exit(0);
            }
            printf("\t\treceived data\n");
            if (items [0].revents & ZMQ_POLLIN) {
                zmsg_t *msg = zmsg_recv (sock_to_servers);

                zlist_t *names = zlist_new();
                zhash_t* frames = receive_message_frames_at_client(msg, names);
  

                get_string_frame(phase, frames, "phase");
                round = get_int_frame(frames, "opnum");

                if(round==op_num && strcmp(phase, WRITE_PUT)==0) {

                   if(DEBUG_MODE) print_out_hash_in_order(frames, names);

                   responses++;
                   if(responses >= majority) break;
                   //if(responses >= num_servers) break;
                }
                else{
                     printf("   OLD MESSAGES : %s  %d\n", phase, op_num);
                }
                zmsg_destroy (&msg);
                destroy_frames(frames);
                zlist_purge(names);
            }
     }
}

// this is the write tag value phase of SODAW
void SODAW_read_complete_phase(  
                      char *obj_name,
                      char *reader_id, 
                      zsock_t *sock_to_servers,  
                      char **servers, 
                      unsigned int num_servers, 
                      char *port, 
                      TAG max_tag   // for read it is max and for write it is new
                   )
{
    // send out the messages to all servers
    char tag_str[100];

    char *types[] = {"object", "algorithm", "phase", "tag"};
    tag_to_string(max_tag, tag_str); 
    send_multicast_servers(sock_to_servers, num_servers, types,  4, obj_name, "SODAW", READ_COMPLETE, tag_str) ;


/*
    zframe_t *object_name_frame = zframe_new(obj_name, strlen(obj_name));
    zframe_t *algorithm_frame = zframe_new("SODAW", strlen("SODAW"));
    zframe_t *phase_frame = zframe_new(READ_COMPLETE, strlen(READ_COMPLETE));

    tag_to_string(max_tag, tag_str); 
    zframe_t *tag_frame = zframe_new(tag_str, strlen(tag_str));

    int i; 
    for(i=0; i < num_servers; i++) {
       zframe_send(&object_name_frame, sock_to_servers, ZFRAME_REUSE + ZFRAME_MORE);
       zframe_send(&algorithm_frame, sock_to_servers, ZFRAME_REUSE + ZFRAME_MORE);
       zframe_send(&phase_frame, sock_to_servers, ZFRAME_REUSE + ZFRAME_MORE);
       zframe_send(&tag_frame, sock_to_servers, ZFRAME_REUSE);
    }

    zframe_destroy(&object_name_frame);
    zframe_destroy(&algorithm_frame);
    zframe_destroy(&phase_frame);
    zframe_destroy(&tag_frame);
*/
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

#ifndef DEBUG_MODE
    printf("\t\tObj name       : %s\n",obj_name);
    printf("\t\tWriter name    : %s\n",writer_id);
    printf("\t\tOperation num  : %d\n",op_num);
    printf("\t\tSize           : %d\n", size);
    printf("\t\tSize of        : %u\n", (unsigned int)strlen(payload));

    printf("\t\tEncoded string : %s\n", strlen(payload));
    printf("\t\tServer string  : %s\n", servers_str);
    printf("\t\tPort           : %s\n", port);

    printf("\t\tNum of Servers  : %d\n",num_servers);

//    printf("Decoded string  : %s\n", myb64);
    for(j=0; j < num_servers; j++) {
        printf("\t\tServer : %s\n", servers[j]);
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
   printf("\tWRITE_GET (WRITER)\n");

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

   printf("\tWRITE_PUT (WRITER)\n");
   SODAW_write_put_phase(
                          obj_name, 
                          writer_id,  
                          op_num, 
                          sock_to_servers, servers,
                          num_servers, 
                          port, 
                          payload, 
                          size, 
                          new_tag
                        );

    zsocket_destroy(ctx, sock_to_servers);
    zctx_destroy(&ctx);


    return true;
}


char *SODAW_read(
                char *obj_name,
                char *reader_id, 
                unsigned int op_num ,
                char *servers_str, 
                char *port
             )
{
    s_catch_signals();
    int j;
    int num_servers = count_num_servers(servers_str);

    char **servers = create_server_names(servers_str);
#ifndef DEBUG_MODE
    printf("\t\tObj name       : %s\n",obj_name);
    printf("\t\tWriter name    : %s\n",reader_id);
    printf("\t\tOperation num  : %d\n",op_num);

   /* char *myb64 = (char *)malloc(strlen(payload));
    b64_decode(payload, myb64);
    printf("Encoded string  : %s\n", payload);
    free(myb64);
*/
    printf("\t\tServer string   : %s\n", servers_str);
    printf("\t\tPort to Use     : %s\n", port);
    printf("\t\tNum of Servers  : %d\n",num_servers);

//    printf("Decoded string  : %s\n", myb64);
    for(j=0; j < num_servers; j++) {
        printf("\t\t\tServer : %s\n", servers[j]);
    }
    printf("\n");
#endif

    
    zctx_t *ctx  = zctx_new();
    void *sock_to_servers = zsocket_new(ctx, ZMQ_DEALER);
    zctx_set_linger(ctx, 0);
    assert (sock_to_servers);

    zsocket_set_identity(sock_to_servers,  reader_id);

    for(j=0; j < num_servers; j++) {    
       char *destination = create_destination(servers[j], port);
       int rc = zsocket_connect(sock_to_servers, destination);
       assert(rc==0);
       free(destination);
    }

   printf("READ %d\n", op_num);
   printf("\tREAD_GET (READER)\n");

   TAG read_tag=  SODAW_write_get_or_read_get_phase(
                      obj_name,  
                      op_num, 
                      sock_to_servers, 
                      servers, 
                      num_servers, 
                      port
                  );

    printf("\t\tmax tag (%d,%s)\n\n", read_tag.z, read_tag.id);


    printf("\tREAD_VALUE (READER)\n");
 

    char *payload = SODAW_read_value(
            obj_name, 
            op_num, 
            sock_to_servers,  
            servers, 
            num_servers, 
            port, 
            read_tag 
        );


    printf("\tREAD_COMPLETE (READER)\n");
   SODAW_read_complete_phase(
            obj_name, 
            reader_id,
            sock_to_servers,  
            servers, 
            num_servers, 
            port, 
            read_tag 
        );



    zsocket_destroy(ctx, sock_to_servers);
    zctx_destroy(&ctx);


    return payload;
}

zhash_t *receive_message_frames_from_server_SODAW(zmsg_t *msg)  {
     char algorithm_name[100];
     char phase_name[100];
     char buf[100];
     zhash_t *frames = zhash_new();

     zframe_t *object_name_frame= zmsg_pop (msg);
     zhash_insert(frames, "object", (void *)object_name_frame);
     get_string_frame(buf, frames, "object");
     printf("\tobject  %s\n", buf); 

     zframe_t *algorithm_frame= zmsg_pop (msg);
     zhash_insert(frames, "algorithm", (void *)algorithm_frame);
     get_string_frame(algorithm_name, frames, "algorithm");
     printf("\talgorithm  %s\n", algorithm_name); 

     zframe_t *phase_frame= zmsg_pop (msg);
     zhash_insert(frames, "phase", (void *)phase_frame);
     get_string_frame(phase_name, frames, "phase");
     printf("\tphase naum %s\n", phase_name);
     
     if( strcmp(algorithm_name, "SODAW") ==0 ) {
         if( strcmp(phase_name, WRITE_GET) ==0 ) {
           zframe_t *opnum_frame= zmsg_pop (msg);
           zhash_insert(frames, "opnum", (void *)opnum_frame);

           zframe_t *tag_frame= zmsg_pop (msg);
           zhash_insert(frames, "tag", (void *)tag_frame);
           get_string_frame(buf, frames, "tag");
         }

         if( strcmp(phase_name, WRITE_PUT) ==0 ) {
           zframe_t *opnum_frame= zmsg_pop (msg);
           zhash_insert(frames, "opnum", (void *)opnum_frame);

           zframe_t *tag_frame= zmsg_pop (msg);
           zhash_insert(frames, "tag", (void *)tag_frame);
           get_string_frame(buf, frames, "tag");
         }

         if( strcmp(phase_name, READ_VALUE) ==0 ) {

           zframe_t *tag_frame= zmsg_pop (msg);
           zhash_insert(frames, "tag", (void *)tag_frame);
           get_string_frame(buf, frames, "tag");
           printf("tag  %s\n", buf); 

           zframe_t *payload_frame= zmsg_pop (msg);
           zhash_insert(frames, "payload", (void *)payload_frame);
         }
     }
     return frames;
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
