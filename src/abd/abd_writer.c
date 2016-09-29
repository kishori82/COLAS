//  Asynchronous client-to-server (DEALER to ROUTER)
//
//  While this example runs in a single process, that is to make
//  it easier to start and stop the example. Each task has its own
//  context and conceptually acts as a separate process.

#include "abd_client.h"
#include "abd_writer.h"


extern int s_interrupted;

#ifdef ASLIBRARY
#define DEBUG_MODE 1

// this fethers the max tag
Tag *get_max_tag_phase(char *obj_name, unsigned int op_num,
                       zsock_t *sock_to_servers,  char **servers,
                       unsigned int num_servers, char *port) {

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
            exit(EXIT_FAILURE);
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
            } else {
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


// this is the write tag value phase of ABD
/*
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
) ;
*/


// ABD write
bool ABD_write(
    char *obj_name,
    unsigned int op_num ,
    char *payload,
    unsigned int payload_size,
    RawData *raw_data,
    ClientArgs *client_args
) {
    s_catch_signals();
    int j;

    int num_servers = count_num_servers(client_args->servers_str);
#ifndef DEBUG_MODE
    printf("\t\tObj name       : %s\n",obj_name);
    printf("\t\tWriter name    : %s\n",client_args->client_id);
    printf("\t\tOperation num  : %d\n",op_num);
    printf("\t\tSize of data   : %d\n", payload_size);

    printf("\t\tServer string  : %s\n", client_args->servers_str);
    printf("\t\tPort           : %s\n", client_args->port);

    printf("\t\tNum of Servers  : %d\n",num_servers);

    for(j=0; j < num_servers; j++) {
        printf("\t\tServer : %s\n", servers[j]);
    }   
    printf("\n");
#endif

    void *sock_to_servers= get_socket_servers(client_args);

    printf("WRITE %d\n", op_num);
    printf("\tGET_TAG (WRITER)\n");

    Tag *max_tag=  ABD_get_max_tag_phase(
                                       obj_name,  
                                       op_num, 
                                       sock_to_servers, 
                                       num_servers
                                       );

    Tag new_tag;
    new_tag.z = max_tag->z + 1;
    strcpy(new_tag.id, client_args->clietn_id);
    free(max_tag);
    printf("\tWRITE_VALUE (WRITER)\n");
    raw_data->data = payload;
    raw_data->data_size = payload_size;

    ABD_write_value_phase(
                      obj_name, 
                      op_num, 
                      sock_to_servers, 
                      num_servers, 
                      raw_data, 
                      new_tag
                    );


    return true;
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
