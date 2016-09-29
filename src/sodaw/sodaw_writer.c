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
                           zsock_t *sock_to_servers,
                           unsigned int num_servers) {

    return SODAW_write_get_or_read_get_phase(
               obj_name, "WRITE_GET",   op_num,
               sock_to_servers,
               num_servers);
}



// this is the write tag value phase of SODAW
void SODAW_write_put_phase (char *obj_name,
                            char *writer_id,
                            unsigned int op_num,
                            zsock_t *sock_to_servers,
                            unsigned int num_servers,
                            Tag max_tag,    // for read it is max and for write it is new
                            EncodeData *encoded_info) {
    // send out the messages to all servers
    char buf[4000];
    char algorithm[BUFSIZE];
    char phase[BUFSIZE];
    char tag_str[BUFSIZE];
    char buf1[400];
    char *value;

    int N = num_servers;

    unsigned int majority =  ceil(((float)num_servers+1)/2);
    int K = majority;

    int symbol_size = SYMBOL_SIZE;

    if(encode(encoded_info)==FALSE) {
        printf("Failed to encode data \n");
        exit(EXIT_FAILURE);
    } 

/*
else {
        if(checking_decoding(encoded_info)==FALSE) {
            printf("Failed to decode encoded data \n");
            exit(EXIT_FAILURE);
        } else {
            printf("Successfully decoded encoded data \n");
        }
    }
*/

    printf("waiting\n");
//    destroy_encoded_data(encoded_data_info);

    unsigned int round;

    zmq_pollitem_t items [] = { { sock_to_servers, 0, ZMQ_POLLIN, 0 } };

    tag_to_string(max_tag, tag_str);

    char *types[] = {OBJECT, ALGORITHM, PHASE, OPNUM, TAG};
    int per_server_payload_size =  encoded_info->num_blocks*encoded_info->encoded_symbol_size;


/*
    if(N==3) {
        sprintf(tag_str, "%s-%x-%x-%x", tag_str,
                simple_hash(encoded_info->encoded_data[0], per_server_payload_size),
                simple_hash(encoded_info->encoded_data[1], per_server_payload_size),
                simple_hash(encoded_info->encoded_data[2], per_server_payload_size)
               );
    }
*/

    send_multisend_servers (sock_to_servers, num_servers,
                            encoded_info->encoded_data, per_server_payload_size,
                            types,  5, obj_name, "SODAW",
                            WRITE_PUT,
                            &op_num, tag_str);


    int i;
    for(i=0; i < encoded_info->N; i++) {
        free(encoded_info->encoded_data[i]);
    }
    free(encoded_info->encoded_data);


    unsigned int responses =0;
    int j =0;

    Tag *tag;
    while (true) {
        //  Tick once per second, pulling in arriving messages

        // zmq_pollitem_t items [] = { { sock_to_servers, 0, ZMQ_POLLIN, 0 } };
        printf("\t\twaiting for data..\n");
        int rc = zmq_poll(items, 1, -1);
        if(rc < 0 ||  s_interrupted ) {
            printf("Interrupted!\n");
            exit(EXIT_FAILURE);
        }
        printf("\t\treceived data\n");
        if (items [0].revents & ZMQ_POLLIN) {
            zmsg_t *msg = zmsg_recv (sock_to_servers);
            //!!
            assert(msg != NULL);

            zlist_t *names = zlist_new();
            //!!
            assert(names);
            assert(zlist_size(names) == 0);

            zhash_t* frames = receive_message_frames_at_client(msg, names);

            get_string_frame(phase, frames, PHASE);
            round = get_int_frame(frames, OPNUM);

            if(round==op_num && strcmp(phase, WRITE_PUT)==0) {

                if(DEBUG_MODE) print_out_hash_in_order(frames, names);

                responses++;
                if(responses >= majority) {
                    zmsg_destroy (&msg);
                    destroy_frames(frames);
                    zlist_purge(names);
                    zlist_destroy(&names);
                    break;
                }
                //if(responses >= num_servers) break;
            } else {
                printf("\t\tOLD MESSAGES : (%s, %d)\n", phase, op_num);
            }
            zmsg_destroy (&msg);
            destroy_frames(frames);
            zlist_purge(names);
            zlist_destroy(&names);
        }
    }
}



// SODAW write
bool SODAW_write(
    char *obj_name,
    unsigned int op_num ,
    char *payload,
    unsigned int payload_size,
    EncodeData *encoded_data,
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
    printf("\tWRITE_GET (WRITER)\n");

    Tag *max_tag=  SODAW_write_get_phase(
                       obj_name,
                       op_num,
                       sock_to_servers,
                       num_servers
                   );

    Tag new_tag;
    new_tag.z = max_tag->z + 1;
    strcpy(new_tag.id, client_args->client_id);
    free(max_tag);
    printf("\tWRITE_PUT (WRITER)\n");
    encoded_data->raw_data = payload;
    encoded_data->raw_data_size = payload_size;
    SODAW_write_put_phase(
        obj_name,
        client_args->client_id,
        op_num,
        sock_to_servers,
        num_servers,
        new_tag,
        encoded_data
    );


//!! Why was this turned off? We need to destroy the socket or else it becomes a memory leak
// as we will constantly generate a new one
    /*
        zsocket_destroy(ctx, sock_to_servers);
        zctx_destroy(&ctx);
        destroy_server_names(servers, num_servers);

    */
    printf("done\n");
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
        //SODAW_write(obj_name, writer_id, i,  payload, size, servers, port);
    }

//   zclock_sleep(50*1000);
    return 0;
}

#endif
