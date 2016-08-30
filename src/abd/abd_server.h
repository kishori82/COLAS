#ifndef _ABD_SERVER
#define _ABD_SERVER


typedef struct _SERVER_STATUS {
    float network_data, metadata_memory, data_memory, cpu_load;
    int time_point;
} SERVER_STATUS;

typedef struct _SERVER_ARGS {
    char *init_data;
    SERVER_STATUS *status;
} SERVER_ARGS;


int ABD_server_process(
               char *server_id, 
               char *port,
               char *init_data,
               SERVER_STATUS *status
             );


#endif
