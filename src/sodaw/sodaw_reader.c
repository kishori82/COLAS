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
#include "sodaw_reader.h"
#include <base64.h>
#include <rlnc_rs.h>

#define DEBUG_MODE 1

extern int s_interrupted;

#ifdef ASLIBRARY

Tag *SODAW_read_get_phase(char *obj_name,
                          unsigned int op_num,
                          zsock_t *sock_to_servers,
                          unsigned int num_servers
                         ) {

    return SODAW_write_get_or_read_get_phase(
               obj_name, READ_GET, op_num,
               sock_to_servers, num_servers);
}


// this fetches the max tag and value
char *SODAW_read_value (char *obj_name,
                        unsigned int op_num,
                        zsock_t *sock_to_servers,
                        unsigned int num_servers,
                        Tag read_tag,
                        EncodeData *encoding_info) {

    // send out the messages to all servers

    char algorithm[BUFSIZE];
    char phase[BUFSIZE];
    char hashbuf[BUFSIZE];
    char tag_str[BUFSIZE];
    char *value=NULL;
    unsigned int round;
    int size;

    zmq_pollitem_t items [] = { { sock_to_servers, 0, ZMQ_POLLIN, 0 } };

    char *types[] = {OBJECT, ALGORITHM, PHASE, OPNUM, TAG};
    tag_to_string(read_tag, tag_str);
    send_multicast_servers(sock_to_servers, num_servers, types,  5, obj_name, SODAW, READ_VALUE, &op_num,   tag_str) ;

    unsigned int majority =  ceil(((float)num_servers+1)/2);
    unsigned int responses =0;
    zlist_t *tag_list = zlist_new();

    zhash_t *seen_values = zhash_new();

    zhash_t *received_data= zhash_new();
    char *decodeableKey ;
    while (true) {
        //  Tick once per second, pulling in arriving messages

        // zmq_pollitem_t items [] = { { sock_to_servers, 0, ZMQ_POLLIN, 0 } };
        printf("\t\twaiting for data..\n");
        int rc = zmq_poll(items, 1, -1);
        if(rc < 0 ||  s_interrupted ) {
            printf("Interrupted!\n");
            exit(EXIT_FAILURE);
        }
        if (items [0].revents & ZMQ_POLLIN) {
            zmsg_t *msg = zmsg_recv (sock_to_servers);
            assert(msg != NULL);

            zlist_t *names = zlist_new();
            assert(names);
            assert(zlist_size(names) == 0);

            zhash_t* frames = receive_message_frames_at_client(msg, names);

            get_string_frame(phase, frames, PHASE);
            get_string_frame(tag_str, frames, TAG);
            Tag tag;
            get_tag_frame(frames, &tag);

            if( zhash_lookup(received_data, tag_str)==NULL) {
                zlist_t *data_list = zlist_new();
                zhash_insert(received_data, tag_str,  (void *)data_list);
            }


            if( compare_tags(tag, read_tag) >=0 && strcmp(phase, READ_VALUE)==0) {
                if(DEBUG_MODE) print_out_hash_in_order(frames, names);

                zframe_t *payload_frame = (zframe_t *)zhash_lookup(frames, PAYLOAD);
                sprintf(hashbuf,"%x", simple_hash(zframe_data(payload_frame), zframe_size(payload_frame)));

                if( zhash_lookup(seen_values, hashbuf) != NULL ) continue;

                zhash_insert(seen_values, hashbuf, (void *)100);

                zlist_t *coded_elements = (zlist_t *)zhash_lookup(received_data, tag_str);
                assert(coded_elements!=NULL);

                zframe_t *dup_payload_frame =zframe_dup(payload_frame);
                zlist_append(coded_elements, dup_payload_frame);

                decodeableKey = number_responses_at_least(received_data, majority);

                if(decodeableKey!= NULL) {
                    zmsg_destroy (&msg);
                    zlist_purge(names);
                    destroy_frames(frames);
                    clear_hash(seen_values);
                    break;
                }
            } else {
                //printf("\t\tOLD MESSAGES : (%s, %d)\n", phase, op_num);
            }
            zmsg_destroy (&msg);
            zlist_purge(names);
            destroy_frames(frames);
        }
    }

    zlist_t *coded_elements = (zlist_t *)zhash_lookup(received_data, decodeableKey);
    printf("Number of keys to decode %s   %d\n", decodeableKey, zlist_size(coded_elements));

    get_encoded_info(received_data, decodeableKey, encoding_info);
    printf("Key to decode %s\n", decodeableKey);

    if(decode(encoding_info)==0) {
        printf("Failed to decode for key %s\n", decodeableKey);
        exit(EXIT_FAILURE);
    }

    return encoding_info->decoded_data;
}


int get_encoded_info(zhash_t *received_data, char *decodeableKey, EncodeData *encoding_info) {
    printf("N : %d\n", encoding_info->N);
    printf("K : %d\n", encoding_info->K);
    printf("Symbol size : %d\n", encoding_info->symbol_size);
    printf("actual datasize %d\n", encoding_info->raw_data_size);
    printf("num_blocks %d\n", encoding_info->num_blocks);

    zlist_t *coded_elements = (zlist_t *)zhash_lookup(received_data, decodeableKey);

    zframe_t *data_frame;
    int frame_size=0, cum_size=0;;
    for(data_frame= (zframe_t *) zlist_first(coded_elements); data_frame!=NULL; data_frame=zlist_next(coded_elements)) {
        printf("Length of data %lu\n", zframe_size(data_frame));
        frame_size = zframe_size(data_frame);
        cum_size += frame_size;
    }

    int i;
    encoding_info->encoded_data = (uint8_t **)malloc( encoding_info->K *sizeof(uint8_t*));
    for(i =0; i < encoding_info->K; i++) {
        encoding_info->encoded_data[i] = (uint8_t *)malloc(frame_size*sizeof(uint8_t));
    }

    i=0;
    for(data_frame= (zframe_t *) zlist_first(coded_elements); data_frame!=NULL; data_frame=zlist_next(coded_elements)) {
        frame_size = zframe_size(data_frame);
        printf("-%x\n", simple_hash(zframe_data(data_frame), frame_size));
        memcpy(encoding_info->encoded_data[i], zframe_data(data_frame), frame_size);
        printf("+%x\n", simple_hash(encoding_info->encoded_data[i], frame_size));
        i++;
    }

    printf("number of symbols used in decoding %d\n", i);
    printf("encoded symbol size %d\n", cum_size/(encoding_info->num_blocks*encoding_info->K));
    encoding_info->encoded_symbol_size = ceil(cum_size/(encoding_info->num_blocks*encoding_info->K));

}

// this is the write tag value phase of SODAW
void SODAW_read_complete_phase (char *obj_name,
                                char *reader_id,
                                zsock_t *sock_to_servers,
                                unsigned int num_servers,
                                int opnum,
                                Tag max_tag   /* for read it is max and for write it is new*/ ) {
    // send out the messages to all servers
    char tag_str[BUFSIZE];
    char *types[] = {OBJECT, ALGORITHM, PHASE, OPNUM, TAG};

    tag_to_string(max_tag, tag_str);
    printf("\tREAD_COMPLETE (READER) 1 %s %d\n", tag_str, opnum);
    send_multicast_servers(sock_to_servers, num_servers, types,  5, obj_name, SODAW, READ_COMPLETE, &opnum, tag_str) ;
    printf("\tREAD_COMPLETE (READER) 2\n");
}

char *SODAW_read (char *obj_name,
                  unsigned int op_num ,
                  EncodeData *encoded_data,
                  ClientArgs *client_args) {
    s_catch_signals();
    int j;

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
    printf("\tREAD_GET (READER)\n");

    Tag *read_tag=  SODAW_read_get_phase(obj_name,
                                         op_num,
                                         sock_to_servers,
                                         num_servers);

    printf("\t\tmax tag (%d,%s)\n\n", read_tag->z, read_tag->id);

    printf("\tREAD_VALUE (READER)\n");

    char *payload = SODAW_read_value(obj_name,
                                     op_num,
                                     sock_to_servers,
                                     num_servers,
                                     *read_tag,
                                     encoded_data);

    printf("\tREAD_COMPLETE (READER)\n");
    SODAW_read_complete_phase(obj_name,
                              client_args->client_id,
                              sock_to_servers,
                              num_servers, 
                              op_num, 
                              *read_tag);

    printf("\tREAD_COMPLETE (READER)\n");
    free(read_tag);

    //!! Why was this turned off? Socket generates a memory leak
    /*
    	 zsocket_destroy(ctx, sock_to_servers);
    	 zctx_destroy(&ctx);
    	 destroy_server_names(servers, num_servers);
    	 */

    return payload;
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
    }

    //   zclock_sleep(50*1000);
    return 0;
}

#endif
