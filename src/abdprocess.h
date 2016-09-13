#ifndef __ABDPROCESSC__
#define __ABDPROCESSC__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "utilities/algo_utils.h"
#include "codes/rlnc_rs.h"
#include "sodaw/sodaw_client.h"
#include "sodaw/sodaw_reader.h"
#include "sodaw/sodaw_writer.h"
#include "sodaw/sodaw_server.h"


typedef struct _Parameters {
    char **ipaddresses;
    unsigned int num_servers;
    char *server_id;
    char port[10];

    enum Algorithm algorithm;
    enum CodingAlgorithm codingalgorithm;
    int wait;
    float filesize;
    enum ProcessType processtype;
} Parameters;

void setDefaults(Parameters *parameters);

unsigned int readParameters(int argc, char *argv[], Parameters *parameters);

void printParameters(Parameters parameters);

Server_Args *get_server_args(Parameters parameters) ;

Server_Status * get_server_status( Parameters parameters) ;
char *get_servers_str(Parameters parameters) ;

void reader_process(Parameters parameters) ;
void writer_process(Parameters parameters) ;
char * get_random_data(float filesize);


#endif
