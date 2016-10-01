//  Asynchronous client-to-server (DEALER to ROUTER)
//
//  While this example runs in a single process, that is to make
//  it easier to start and stop the example. Each task has its own
//  context and conceptually acts as a separate process.

#include "abd_client.h"


int s_interrupted;

#ifdef ASLIBRARY

#define DEBUG_MODE 1

void  ABD_write_value_phase(
    char *obj_name,
    unsigned int op_num,
    zsock_t *sock_to_servers,
    unsigned int num_servers,
    RawData *abd_data,
    Tag max_tag   // for read it is max and for write it is new
) {
    // send out the messages to all servers
    char phase[100];
    char tag_str[100];


    unsigned int majority =  ceil(((float)num_servers+1)/2);

    unsigned int round;
    zmq_pollitem_t items [] = { { sock_to_servers, 0, ZMQ_POLLIN, 0 } };
    tag_to_string(max_tag, tag_str);


    zframe_t *tag_frame = zframe_new(tag_str, strlen(tag_str));

    char *types[] = {OBJECT, ALGORITHM, PHASE, OPNUM, TAG, PAYLOAD};

    send_multicast_servers(sock_to_servers, num_servers, types,  6, obj_name, ABD, WRITE_VALUE, &op_num, tag_str, abd_data->data) ;

    unsigned int responses =0; 

    while (true) {
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
            assert(msg!=NULL);

            zlist_t *names = zlist_new();
            assert(names!=NULL);
            assert(zlist_size(names)==0);

            zhash_t* frames = receive_message_frames_at_client(msg, names);

            get_string_frame(phase, frames, PHASE);
            round = get_int_frame(frames, OPNUM);

            if(round==op_num && strcmp(phase, WRITE_VALUE)==0) {
                responses++;
                if(responses >= majority) {
                    zmsg_destroy(&msg);
                    destroy_frames(frames);
                    zlist_purge(names);
                    zlist_destroy(&names);
                    break;
                }
                //if(responses >= num_servers) break;
            } else {
                printf("\tOLD MESSAGES : (%s, %d)\n", phase, op_num);

            }
        }
    }   
    return;
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
