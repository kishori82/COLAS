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
#include "sodaw_client.h"
#include <base64.h>
#include <rlnc_rs.h>

#define DEBUG_MODE 1

extern int s_interrupted;

#ifdef ASLIBRARY


Tag *SODAW_write_get_phase(char *obj_name, unsigned int op_num, 
                      zsock_t *sock_to_servers,  char **servers, 
                          unsigned int num_servers, char *port)
{

return SODAW_write_get_or_read_get_phase(
                     obj_name, "WRITE_GET",   op_num, 
                      sock_to_servers,  servers, 
                      num_servers, port);
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
                      Tag max_tag   // for read it is max and for write it is new
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

    int symbol_size = SYMBOL_SIZE;

    
    ENCODED_DATA  encoded_data_info = encode(N, K, symbol_size, payload, size, reed_solomon) ;

    //ENCODED_DATA  encoded_data_info = encode(N, K, symbol_size, payload, strlen(payload), reed_solomon) ;
//    destroy_encoded_data(encoded_data_info);

    unsigned int round;

    zmq_pollitem_t items [] = { { sock_to_servers, 0, ZMQ_POLLIN, 0 } };

    tag_to_string(max_tag, tag_str); 


    char *types[] = {OBJECT, ALGORITHM, PHASE, OPNUM, TAG};
    size =  encoded_data_info.num_blocks*encoded_data_info.encoded_symbol_size;

    send_multisend_servers(
                    sock_to_servers, num_servers, 
                    encoded_data_info.encoded_raw_data, size,
                    types,  5, obj_name, "SODAW", 
                    WRITE_PUT, 
                    &op_num, tag_str) ;


    int i;
    for(i=0; i < encoded_data_info.N; i++) {
       free(encoded_data_info.encoded_raw_data[i]);
    }
    free(encoded_data_info.encoded_raw_data);


     unsigned int responses =0;
     int j =0;
     zlist_t *tag_list = zlist_new();
     
     Tag *tag;
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
  

                get_string_frame(phase, frames, PHASE);
                round = get_int_frame(frames, OPNUM);

                if(round==op_num && strcmp(phase, WRITE_PUT)==0) {

                   if(DEBUG_MODE) print_out_hash_in_order(frames, names);

                   responses++;
                   if(responses >= majority) break;
                   //if(responses >= num_servers) break;
                }
                else{
                     printf("\t\tOLD MESSAGES : (%s, %d)\n", phase, op_num);
                }
                zmsg_destroy (&msg);
                destroy_frames(frames);
                zlist_purge(names);
            }
     }
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

    int64_t affinity = 50000;
    int rc = zmq_setsockopt(socket, ZMQ_SNDBUF, &affinity, sizeof affinity);


    for(j=0; j < num_servers; j++) {    
       char *destination = create_destination(servers[j], port);
       int rc = zsocket_connect(sock_to_servers, (const char *)destination);
       assert(rc==0);
       free(destination);
    }

   printf("WRITE %d\n", op_num);
   printf("\tWRITE_GET (WRITER)\n");

   Tag *max_tag=  SODAW_write_get_phase(
                      obj_name,  
                      op_num, 
                      sock_to_servers, 
                      servers, 
                      num_servers, 
                      port
                  );


   Tag new_tag;
   new_tag.z = max_tag->z + 1;
   strcpy(new_tag.id, writer_id);
   free(max_tag);
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

    destroy_server_names(servers, num_servers);
    return true;
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
   char obj_name[] = {OBJECT};

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
