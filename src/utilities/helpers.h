#ifndef __HELPERS__
#define __HELPERS__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "server.h"
#include "algo_utils.h"
#include "client.h"
#include "../codes/rlnc_rs.h"


typedef struct _Parameters {
    char **ipaddresses;
    unsigned int num_servers;
    char server_id[BUFSIZE];
    char port[10];
    char port1[10];
    enum Algorithm algorithm;
    enum CodingAlgorithm coding_algorithm;
    int wait;
    float filesize_kb;
    enum ProcessType processtype;
} Parameters;


/*
char **get_memory_for_ipaddresses(int num_ips) {
    char **ipaddresses =  (char **)malloc(num_ips *sizeof(char *));
    int i;
    for( i =0; i < num_ips; i++)  {
        ipaddresses[i] = (char *)malloc(16*sizeof(char));
    }
    return ipaddresses;
}
*/


EncodeData *create_EncodeData(Parameters parameters) ;

RawData *create_RawData(Parameters parameters) ;

ClientArgs *create_ClientArgs(Parameters parameters) ;

void setDefaults(Parameters *parameters);

Server_Args *get_server_args(Parameters parameters) ;

Server_Status * get_server_status( Parameters parameters) ;

char *get_servers_str(Parameters parameters) ;

char * get_random_data(unsigned int filesize);

bool is_equal(char *payload1, char*payload2, unsigned int size) ;

void destroy_server_args(Server_Args *server_args) ;

void printParameters(Parameters parameters) ;

#endif
