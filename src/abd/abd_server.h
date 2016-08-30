#ifndef _ABD_SERVER
#define _ABD_SERVER

int ABD_server_process(
               char *server_id, 
               char *port,
               char *init_data
             );

typedef struct _SERVER_ARGS {
    char *init_data;
} SERVER_ARGS;



#endif
