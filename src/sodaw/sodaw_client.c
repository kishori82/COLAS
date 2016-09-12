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

// this fethers the max tag
Tag *SODAW_write_get_or_read_get_phase(
        char *obj_name, 
        const char *_phase, 
        unsigned int op_num, 
        zsock_t *sock_to_servers,  
        char **servers, 
        unsigned int num_servers, 
        char *port)
{
    // send out the messages to all servers
    char buf[400];
    char algorithm[100];
    char phase[100];
    char tag_str[100];
    char buf1[400];
    unsigned int round;

    zmq_pollitem_t items [] = { { sock_to_servers, 0, ZMQ_POLLIN, 0 } };

    char *types[] = {OBJECT, ALGORITHM, PHASE, OPNUM};
    send_multicast_servers(sock_to_servers, num_servers, types, 4, obj_name, SODAW, READ_GET, &op_num) ;

    unsigned int majority =  ceil(((float)num_servers+1)/2);
    unsigned int responses =0;
    zlist_t *tag_list = zlist_new();

    Tag *tag;
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

            get_string_frame(phase, frames, PHASE);
            round = get_int_frame(frames, OPNUM);
            get_string_frame(tag_str, frames, TAG);

            if(round==op_num && strcmp(phase, _phase)==0) {
                if(DEBUG_MODE) print_out_hash_in_order(frames, names);

                responses++;
                // add tag to list                
                tag = (Tag *)malloc(sizeof(Tag));
                string_to_tag(tag_str, tag);
                zlist_append(tag_list, (void *)tag);

                if(responses >= majority) { 
                    break;
                }
                //if(responses >= num_servers) break;
            }
            else{
                printf("\t\tOLD MESSAGES : (%s, %d)\n", phase, op_num);

            }
            zmsg_destroy (&msg);
            zlist_purge(names);
            destroy_frames(frames);
        }
    }
    //comute the max tag now and return 
    Tag *max_tag = get_max_tag(tag_list);
    free_items_in_list(tag_list);
    zlist_destroy(&tag_list);

    return  max_tag;
}

// this fetches the max tag and value


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


zhash_t *receive_message_frames_from_server_SODAW(zmsg_t *msg)  {
    char algorithm_name[100];
    char phase_name[100];
    char buf[100];
    zhash_t *frames = zhash_new();

    zframe_t *object_name_frame= zmsg_pop (msg);
    zhash_insert(frames, OBJECT, (void *)object_name_frame);
    get_string_frame(buf, frames, OBJECT);
    printf("\tobject  %s\n", buf); 

    zframe_t *algorithm_frame= zmsg_pop (msg);
    zhash_insert(frames, ALGORITHM, (void *)algorithm_frame);
    get_string_frame(algorithm_name, frames, ALGORITHM);
    printf("\talgorithm  %s\n", algorithm_name); 

    zframe_t *phase_frame= zmsg_pop (msg);
    zhash_insert(frames, PHASE, (void *)phase_frame);
    get_string_frame(phase_name, frames, PHASE);
    printf("\tphase naum %s\n", phase_name);

    if( strcmp(algorithm_name, SODAW) ==0 ) {
        if( strcmp(phase_name, WRITE_GET) ==0 ) {
            zframe_t *opnum_frame= zmsg_pop (msg);
            zhash_insert(frames, OPNUM, (void *)opnum_frame);

            zframe_t *tag_frame= zmsg_pop (msg);
            zhash_insert(frames, TAG, (void *)tag_frame);
            get_string_frame(buf, frames, TAG);
        }

        if( strcmp(phase_name, WRITE_PUT) ==0 ) {
            zframe_t *opnum_frame= zmsg_pop (msg);
            zhash_insert(frames, OPNUM, (void *)opnum_frame);

            zframe_t *tag_frame= zmsg_pop (msg);
            zhash_insert(frames, TAG, (void *)tag_frame);
            get_string_frame(buf, frames, TAG);
        }

        if( strcmp(phase_name, READ_VALUE) ==0 ) {

            zframe_t *tag_frame= zmsg_pop (msg);
            zhash_insert(frames, TAG, (void *)tag_frame);
            get_string_frame(buf, frames, TAG);
            printf("tag  %s\n", buf); 

            zframe_t *payload_frame= zmsg_pop (msg);
            zhash_insert(frames, PAYLOAD, (void *)payload_frame);
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
