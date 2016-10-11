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
//#include "sodaw_client.h"
#include "client.h"
#include "soda_reader.h"
#include <base64.h>
#include <rlnc_rs.h>

#define DEBUG_MODE 1

extern int s_interrupted;

#ifdef ASLIBRARY



// this fetches the max tag and value
char *SODA_generic_phase(
                        zsock_t *sock_to_servers,
                        unsigned int num_servers
                        ) {

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
    //tag_to_string(read_tag, tag_str);
//    send_multicast_servers(sock_to_servers, num_servers, types,  5, obj_name, SODAW, READ_VALUE, &op_num,   tag_str) ;

    unsigned int majority =  ceil(((float)num_servers+1)/2);
    unsigned int responses =0;



    return NULL;
}


char *SODA_read (char *obj_name,
                  unsigned int op_num ,
                  EncodeData *encoded_data,
                  ClientArgs *client_args) {
    s_catch_signals();
    int j;

    int num_servers = count_num_servers(client_args->servers_str);
    void *sock_to_servers= get_socket_servers(client_args);
    void *md_socket_dealer= get_md_socket_dealer(client_args);
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

    char *payload = SODA_generic_phase(
                                   sock_to_servers,
                                   num_servers
                                );

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
