//  Asynchronous client-to-server (DEALER to ROUTER)
//
//  While this example runs in a single process, that is to make
//  it easier to start and stop the example. Each task has its own
//  context and conceptually acts as a separate process.

#include "abd_client.h"
#include "abd_reader.h"


extern int s_interrupted;

#ifdef ASLIBRARY
#define DEBUG_MODE 1


// this fetches the max tag and value

void  ABD_get_max_tag_value_phase(
    char *obj_name,
    unsigned int op_num,
    zsock_t *sock_to_servers,
    unsigned int num_servers,
    RawData *max_tag_value
) {

    // send out the messages to all servers

    char phase[64];
    char tag_str[64];
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
            exit(EXIT_FAILURE);
        }
        printf("\t\treceived data\n");

        if (items [0].revents & ZMQ_POLLIN) {
            zmsg_t *msg = zmsg_recv(sock_to_servers);

            zlist_t *names = zlist_new();
            zhash_t* frames = receive_message_frames_at_client(msg, names);

            //value
            zframe_t *value_frame = (zframe_t *)zhash_lookup(frames, PAYLOAD);
            assert(value_frame !=NULL);

            get_string_frame(phase, frames, PHASE);
            round = get_int_frame(frames, OPNUM);
            get_string_frame(tag_str, frames, TAG);


            if(round==op_num && strcmp(phase, GET_TAG_VALUE)==0) {
                if(DEBUG_MODE) print_out_hash_in_order(frames, names);
                responses++;
                // add tag to list
                tag = (Tag *)malloc(sizeof(Tag));
                string_to_tag(tag_str, tag);
                zlist_append(tag_list, (void *)tag);

                max_tag_value->data = (void *)value_frame;
                max_tag_value->data_size = zframe_size(value_frame);

                if(responses >= majority) { 
                   zlist_purge(names);
                   zlist_destroy(&names);
                   zmsg_destroy (&msg);
                   destroy_frames(frames);
                   break;
                }
                //if(responses >= num_servers) break;
            } else {
                printf("\tOLD MESSAGES : (%s, %d)\n", phase, op_num);

            }
            zmsg_destroy (&msg);
        }
    }
    //comute the max tag now and return
    max_tag_value->tag = get_max_tag(tag_list);

    free_items_in_list(tag_list);
    zlist_destroy(&tag_list);
}


RawData  *ABD_read(
    char *obj_name,
    unsigned int op_num ,
    ClientArgs *client_args
) {
    s_catch_signals();

    int num_servers = count_num_servers(client_args->servers_str);
    void *sock_to_servers= get_socket_servers(client_args);
#ifdef DEBUG_MODE
    printf("\t\tObj name       : %s\n",obj_name);
    printf("\t\tWriter name    : %s\n",client_args->client_id);
    printf("\t\tOperation num  : %d\n",op_num);

    printf("\t\tServer string   : %s\n", client_args->servers_str);
    printf("\t\tPort to Use     : %s\n", client_args->port);
    printf("\t\tNum of Servers  : %d\n",num_servers);
#endif


    printf("READ %d\n", op_num);
    printf("\tMAX_TAG_VALUE (READER)\n");

    Tag max_tag;

    RawData *max_tag_value = malloc(sizeof(RawData));

    ABD_get_max_tag_value_phase(obj_name,  
                            op_num, 
                            sock_to_servers,       
                            num_servers, 
                            max_tag_value
                           );

    printf("\tWRITE_VALUE (READER)\n");
    ABD_write_value_phase(
                      obj_name, 
                      op_num, 
                      sock_to_servers, 
                      num_servers, 
                      max_tag_value,
                      max_tag
                    );


   return max_tag_value;
}


#endif


//  The main thread simply starts several clients and a server, and then
//  waits for the server to finish.
//#define ASMAIN

#ifdef ASMAIN

int main (void) {
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
    char port[]= {PORT};

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
