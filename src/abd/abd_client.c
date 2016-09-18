//  Asynchronous client-to-server (DEALER to ROUTER)
//
//  While this example runs in a single process, that is to make
//  it easier to start and stop the example. Each task has its own
//  context and conceptually acts as a separate process.

#include "abd_client.h"


extern int s_interrupted;

#ifdef ASLIBRARY
#define DEBUG_MODE 1

// this fethers the max tag
Tag *get_max_tag_phase(char *obj_name, unsigned int op_num, 
                      zsock_t *sock_to_servers,  char **servers, 
                          unsigned int num_servers, char *port)
{

    // send out the messages to all servers

    char phase[100];
    char tag_str[100];
    unsigned int round;

    zmq_pollitem_t items [] = { { sock_to_servers, 0, ZMQ_POLLIN, 0 } };

    char *types[] = {OBJECT, ALGORITHM, PHASE, OPNUM};
    send_multicast_servers(sock_to_servers, num_servers, types,  4, obj_name, "ABD", GET_TAG, &op_num) ;

    unsigned int majority =  ceil(((float)num_servers+1)/2);
//     zmq_pollitem_t items [] = { { sock_to_servers, 0, ZMQ_POLLIN, 0 } };
     unsigned int responses =0;
     zlist_t *tag_list = zlist_new();
     
     Tag *tag;
     while (true) {
        //  Tick once per second, pulling in arriving messages
            
           // zmq_pollitem_t items [] = { { sock_to_servers, 0, ZMQ_POLLIN, 0 } };
            printf("\t\twaiting for data....\n");
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
                get_string_frame(tag_str, frames, TAG);
                  
                if(round==op_num && strcmp(phase, GET_TAG)==0) {
                     if(DEBUG_MODE) print_out_hash_in_order(frames, names);

                     responses++;
                     // add tag to list                
                     tag = (Tag *)malloc(sizeof(TAG));
                     string_to_tag(tag_str, tag);
                     zlist_append(tag_list, (void *)tag);

                     if(responses >= majority) break;
                   //if(responses >= num_servers) break;
                }
                else{
                    printf("\tOLD MESSAGES : %s  %d\n", phase, op_num);
                }

                zmsg_destroy (&msg);
                zlist_purge(names);
           }
     }
   //comute the max tag now and return 
     Tag *max_tag = get_max_tag(tag_list);

     free_items_in_list(tag_list);
     zlist_destroy(&tag_list);
     return  max_tag;
}

// this fetches the max tag and value

void  get_max_tag_value_phase(
            char *obj_name, 
            unsigned int op_num, 
            zsock_t *sock_to_servers,  
            char **servers, 
            unsigned int num_servers, 
            char *port, 
            Tag **max_tag, 
            char **max_value
        )
{

    // send out the messages to all servers

    char phase[64];
    char tag_str[64];
    char *value=NULL;
    unsigned int round;

    Tag *tag;

    zmq_pollitem_t items [] = { { sock_to_servers, 0, ZMQ_POLLIN, 0 } };

    char *types[] = {OBJECT, ALGORITHM, PHASE, OPNUM};
    send_multicast_servers(sock_to_servers, num_servers, types,  4, obj_name, "ABD", GET_TAG_VALUE, &op_num) ;


    unsigned int majority =  ceil(((float)num_servers+1)/2);
     unsigned int responses =0;
     zlist_t *tag_list = zlist_new();
     
     while (true) {
            //  Tick once per second, pulling in arriving messages
            
           // zmq_pollitem_t items [] = { { sock_to_servers, 0, ZMQ_POLLIN, 0 } };
            printf("\t\twaiting for  data...\n");
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

                //value
                zframe_t *value_frame = (zframe_t *)zhash_lookup(frames, PAYLOAD);
                assert(value_frame !=NULL);
                value = (char *)malloc(  (zframe_size(value_frame) + 1)*sizeof(char) );
                _zframe_str(value_frame, value);

                get_string_frame(phase, frames, PHASE);
                round = get_int_frame(frames, OPNUM);
                get_string_frame(tag_str, frames, TAG);
 

                if(round==op_num && strcmp(phase, GET_TAG_VALUE)==0) {
                   responses++;
                   // add tag to list                
                   tag = (Tag *)malloc(sizeof(Tag));
                   string_to_tag(tag_str, tag);
                   zlist_append(tag_list, (void *)tag);

                   *max_value = value;
                   if(responses >= majority) break;
                   //if(responses >= num_servers) break;
                }
                else{
                     printf("\tOLD MESSAGES : (%s, %d)\n", phase, op_num);

                }
                zmsg_destroy (&msg);
            }
     }
   //comute the max tag now and return 
      max_tag = get_max_tag(tag_list);

     free_items_in_list(tag_list);
     zlist_destroy(&tag_list);
}


// this is the write tag value phase of ABD
Tag write_value_phase(  
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
    char phase[100];
    char tag_str[100];
    

    unsigned int round;

    zmq_pollitem_t items [] = { { sock_to_servers, 0, ZMQ_POLLIN, 0 } };


    tag_to_string(max_tag, tag_str); 
    zframe_t *tag_frame = zframe_new(tag_str, strlen(tag_str));

    char *types[] = {OBJECT, ALGORITHM, PHASE, OPNUM, TAG, PAYLOAD};
    send_multicast_servers(sock_to_servers, num_servers, types,  6, obj_name, "ABD",\
            WRITE_VALUE, &op_num, tag_str, payload) ;

    unsigned int majority =  ceil((num_servers+1)/2);
    unsigned int responses =0;
     
     while (true) {
           // zmq_pollitem_t items [] = { { sock_to_servers, 0, ZMQ_POLLIN, 0 } };
            printf("\t\twaiting for data....\n");
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
                  
                zmsg_destroy (&msg);
        
                if(round==op_num && strcmp(phase, WRITE_VALUE)==0) {
                   responses++;

                   if(responses >= majority) break;
                   //if(responses >= num_servers) break;
                }
                else{
                     printf("\tOLD MESSAGES : (%s, %d)\n", phase, op_num);

                }
            }
     }
     return max_tag;
}


// ABD write
bool ABD_write(
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
    printf("WRITE %d\n", op_num);
    printf("\tObj name       : %s\n",obj_name);
    printf("\tWriter name    : %s\n",writer_id);
    printf("\tOperation num  : %d\n",op_num);
    printf("\tSize           : %d\n", size);
    printf("\tSize of        : %u\n", (unsigned int)strlen(payload));

    char *myb64 = (char *)malloc(strlen(payload));

    b64_decode(payload, myb64);

    printf("\tBase64 Encoded string len  : %d\n", strlen(payload));
    printf("\tServer string   : %s\n", servers_str);
    printf("\tPort to Use     : %s\n", port);

    int num_servers = count_num_servers(servers_str);

    printf("\tNum of Servers  : %d\n",num_servers);

//    printf("Decoded string  : %s\n", myb64);
    char **servers = create_server_names(servers_str);
    for(j=0; j < num_servers; j++) {
        printf("\t\tServer : %s\n", servers[j]);
    }
    printf("\n");
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

   printf("\tGET_TAG (WRITER)\n");

   Tag *max_tag=  get_max_tag_phase(obj_name,  op_num, sock_to_servers, servers, num_servers, port);

    Tag new_tag;
    new_tag.z = max_tag->z + 1;
    strcpy(new_tag.id, writer_id);
    free(max_tag);

   printf("\tWRITE_VALUE (WRITER)\n");
   write_value_phase(obj_name, writer_id,  op_num, sock_to_servers, servers,
                     num_servers, port, payload, size, new_tag);

    zsocket_destroy(ctx, sock_to_servers);
    zctx_destroy(&ctx);


    return true;
}


char *ABD_read(
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

    int num_servers = count_num_servers(servers_str);

    printf("Num of Servers  : %d\n",num_servers);

//    printf("Decoded string  : %s\n", myb64);
    char **servers = create_server_names(servers_str);
    for(j=0; j < num_servers; j++) {
        printf("\tServer : %s\n", servers[j]);
    }
    printf("\n");
    
    zctx_t *ctx  = zctx_new();
    void *sock_to_servers = zsocket_new(ctx, ZMQ_DEALER);
    zctx_set_linger(ctx, 0);
    assert (sock_to_servers);

    zsocket_set_identity(sock_to_servers,  writer_id);
    for(j=0; j < num_servers; j++) {    
       char *destination = create_destination(servers[j], port);
       int rc = zsocket_connect(sock_to_servers, destination);
       assert(rc==0);
       free(destination);
    }

   printf("READ %d\n", op_num);
   printf("     MAX_TAG_VALUE (READER)\n");

   char *payload;
   Tag *max_tag;
   get_max_tag_value_phase(obj_name,  op_num, sock_to_servers, servers, num_servers, port, &max_tag, &payload);

   printf("\tmax tag (%d,%s)\n\n", max_tag->z, max_tag->id);
//   printf("\tsize of data read (%s)\n", strlen(payload));

   printf("     WRITE_VALUE (READER)\n");
   int size = strlen(payload);
   write_value_phase(obj_name, writer_id,  op_num, sock_to_servers, servers,
                     num_servers, port, payload, size, *max_tag);

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
   char obj_name[] = {OBJECT};

   unsigned int op_num;
   s_catch_signals();

  for( i=0; i < 5; i++) {
    printf("\nWRITE %d\n", i);
    //ABD_write(obj_name, writer_id, i,  payload, size, servers, port);
  }

//   zclock_sleep(50*1000); 
   return 0;
}




#endif
