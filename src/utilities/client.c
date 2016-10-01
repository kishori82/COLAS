//  Asynchronous client-to-server (DEALER to ROUTER)
//
//  While this example runs in a single process, that is to make
//  it easier to start and stop the example. Each task has its own
//  context and conceptually acts as a separate process.

#include "client.h"

/*
int
special_zframe_send (zframe_t **self_p, void *dest, int flags) {
    assert (dest);
    assert (self_p);

    void *handle = zsock_resolve (dest);
    if (*self_p) {
        zframe_t *self = *self_p;
        assert (zframe_is (self));

        int send_flags = (flags & ZFRAME_MORE)? ZMQ_SNDMORE: 0;
        send_flags |= (flags & ZFRAME_DONTWAIT)? ZMQ_DONTWAIT: 0;
        if (flags & ZFRAME_REUSE) {
            zmq_msg_t copy;
            zmq_msg_init (&copy);
            if (zmq_msg_copy (&copy, zframe_data(self)))
                return -1;
            if (zmq_sendmsg (handle, &copy, send_flags) == -1) {
                zmq_msg_close (&copy);
                return -2;
            }
        } else {
            if (zmq_sendmsg(handle, zframe_data(&self), send_flags) >= 0)
                zframe_destroy (self_p);
            else
                return -3;
        }
    }
    return 0;
}
*/

#define DEBUG_MODE  1

extern int s_interrupted;

void send_multicast_servers(void *sock_to_servers, int num_servers, char *names[],  int n, ...) {
    RawData *rawdata;
    va_list valist;
    int i =0, j;

    va_start(valist, n);

    void **values = (void **)malloc(n*sizeof(void *));

    /*
        for(i=0; i < n; i++)  {
           values[i] = (void *)malloc(10*sizeof(void));
        }
    */

    zframe_t **frames = (zframe_t **)malloc(n*sizeof(zframe_t *));
    assert(values!=NULL);
    assert(frames!=NULL);
    for(i=0; i < n; i++ ) {
        if( strcmp(names[i], OPNUM)==0)   {
            values[i] = (void *)va_arg(valist, unsigned  int *);
            frames[i]= zframe_new( (const void *)values[i], sizeof(unsigned int));
            //frames[i]= zframe_new((const void *)values[i], sizeof(*values[i]));
        }
        else if( strcmp(names[i], PAYLOAD)==0)   {
            rawdata = va_arg(valist, RawData *);
            values[i] = rawdata;
            frames[i]= zframe_new(rawdata->data, rawdata->data_size);
        } else {
            values[i] = va_arg(valist, char *);
            frames[i]= zframe_new(values[i], strlen((char *)values[i]));
        }
    }
    va_end(valist);

    // it to all servers in a round robin fashion
    int rc;
    if(DEBUG_MODE)  printf("\n");
    if(DEBUG_MODE) printf("\t\tsending ..\n");
    for(i=0; i < num_servers; i++) {
        printf("\t\t\tserver : %d\n", i);
        for(j=0; j < n-1; j++) {
            if(DEBUG_MODE) {
                if( strcmp(names[j], OPNUM)==0)
                    printf("\t\t\tFRAME%d :%s  %u\n",j, names[j], *((unsigned int *)values[j]) );
                else if( strcmp(names[j], PAYLOAD)==0)
                    printf("\t\t\tFRAME%d :%s  %d\n", j, names[j],  ((RawData *)values[j])->data_size);
                else
                    printf("\t\t\tFRAME%d :%s  %s\n", j, names[j],   (char *)values[j]);

                rc = zframe_send(&frames[j], sock_to_servers, ZFRAME_REUSE +  ZFRAME_MORE);

                if( rc < 0) {
                    printf("ERROR: %d\n", rc);
                    exit(EXIT_FAILURE);
                }

                assert(rc!=-1);

            }
        }


        rc = zframe_send(&frames[j], sock_to_servers, ZFRAME_REUSE + ZFRAME_DONTWAIT);

        if(DEBUG_MODE) {
            if( strcmp(names[j], OPNUM)==0) {
                printf("\t\t\tFRAME%d :%s  %u\n", j, names[j],   *((unsigned int *)values[j]) );
            } else if( strcmp(names[j], PAYLOAD)==0) {
                printf("\t\t\tFRAME%d :%s  %d\n", j, names[j],  ((RawData *)values[j])->data_size);
            } else {
                printf("\t\t\tFRAME%d :%s  %s\n", j, names[j],   (char *)values[j]);
            }
        }

        if( rc < 0) {
            printf("ERROR: %d\n", rc);
            exit(EXIT_FAILURE);
        }

        if(DEBUG_MODE)  printf("\n");
    }

    printf("\n");
    if(DEBUG_MODE)  printf("\n");

    //!! Do we not need to free the inner loopo?
//!! potential memory conflict
    /*
    for(i=0; i < n; i++ ) {
    free(values[i]);
    }
    */
    if( values!=NULL) free(values);

    for(i=0; i < n; i++ ) {
        zframe_destroy(&frames[i]);
    }
    if( frames!=NULL) free(frames);
}

void send_multisend_servers(void *sock_to_servers,
                            int num_servers,
                            uint8_t **messages,
                            int msg_size,
                            char *names[],
                            int n, ...) {
    va_list valist;
    int i, j;

    void **values = malloc(n*sizeof(void *));
    assert(values!=NULL);

    zframe_t **frames = (zframe_t **)malloc( (n+1)*sizeof(zframe_t *));
    assert(frames!=NULL);

    va_start(valist, n);
    // for n arguments
    for(i=0; i < n; i++ ) {

        if( strcmp(names[i], OPNUM)==0)   {
            values[i] = (void *)va_arg(valist, unsigned  int *);
        } else {
            values[i] = va_arg(valist, void *);
        }

        if( strcmp(names[i], OPNUM)==0) {
            frames[i]= zframe_new((const void *)values[i], sizeof(unsigned int));
        } else {
            frames[i]= zframe_new(values[i], strlen((char *)values[i]));
        }
    }
    va_end(valist);

    // it to all servers in a round robin fashion
    if( DEBUG_MODE) printf("\tsending ..\n");
    for(i=0; i < num_servers; i++) {  // one server at a time
        for(j=0; j < n; j++) { //send the first n arguments
            if(DEBUG_MODE) {
                if( strcmp(names[j], OPNUM)==0)  {
                    printf("\t\t\tFRAME%d :%s  %d\n", j, names[j], *((unsigned int *)values[j]) );
                } else {
                    printf("\t\t\tFRAME%d :%s  %s\n", j, names[j], (char *)values[j]);
                }
            }
            zframe_send( &frames[j], sock_to_servers, ZFRAME_REUSE + ZFRAME_MORE);
        }
        // a different coded element for each different server

        frames[n]= zframe_new(messages[i], msg_size);
        assert( zframe_size(frames[n])==msg_size);

        //    frames[n]= zframe_new( payload, strlen(pay));
        if(DEBUG_MODE) printf("\t\t\tFRAME%d :%s  %d\n", n, PAYLOAD,  msg_size );
        //zframe_send( &frames[n], sock_to_servers, ZFRAME_REUSE);
        zframe_send( &frames[n], sock_to_servers, 0);
        if(DEBUG_MODE)  printf("\n");
        zframe_destroy(&frames[n]);
    } //for a server end


    //!! Inner loop of buffers not freed
//!! potential memory conflict
    /*
    		for(i=0; i<n; i++){
        	free(values[i]);
    		}
    */
    if( values!=NULL) free(values);

    for(i=0; i < n; i++ ) {
        zframe_destroy(&frames[i]);
    }

    if( frames!=NULL) free(frames);
}

zhash_t *receive_message_frames_at_client(zmsg_t *msg, zlist_t *names)  {
    char algorithm_name[BUFSIZE];
    char object_name[BUFSIZE];
    char phase_name[BUFSIZE];
    zhash_t *frames = zhash_new();

    insertIntoHashAndList(OBJECT, msg, frames, names);
    get_string_frame(object_name, frames, OBJECT);
    insertIntoHashAndList(ALGORITHM, msg, frames, names);
    get_string_frame(algorithm_name, frames, ALGORITHM);
    insertIntoHashAndList(PHASE, msg, frames, names);
    get_string_frame(phase_name, frames, PHASE);

    if( strcmp(algorithm_name, "ABD") ==0 ) {

        if( strcmp(phase_name, GET_TAG) ==0 ) {
            insertIntoHashAndList(OPNUM, msg, frames, names);
            insertIntoHashAndList(TAG, msg, frames, names);
        }

        if( strcmp(phase_name, WRITE_VALUE) ==0 ) {
            insertIntoHashAndList(OPNUM, msg, frames, names);
            insertIntoHashAndList(TAG, msg, frames, names);
        }

        if( strcmp(phase_name, GET_TAG_VALUE) ==0 ) {
            insertIntoHashAndList(OPNUM, msg, frames, names);
            insertIntoHashAndList(TAG, msg, frames, names);
            insertIntoHashAndList(PAYLOAD, msg, frames, names);
        }
    }

    if( strcmp(algorithm_name, SODAW) ==0 ) {

        if( strcmp(phase_name, WRITE_GET) ==0 ) {
            insertIntoHashAndList(OPNUM, msg, frames, names);
            insertIntoHashAndList(TAG, msg, frames, names);
        }

        if( strcmp(phase_name, READ_GET) ==0 ) {
            insertIntoHashAndList(OPNUM, msg, frames, names);
            insertIntoHashAndList(TAG, msg, frames, names);
        }

        if( strcmp(phase_name, WRITE_PUT) ==0 ) {
            insertIntoHashAndList(OPNUM, msg, frames, names);
            insertIntoHashAndList(TAG, msg, frames, names);
        }

        if( strcmp(phase_name, READ_VALUE) ==0 ) {
            insertIntoHashAndList(TAG, msg, frames, names);
            insertIntoHashAndList(PAYLOAD, msg, frames, names);
        }
    }
    return frames;
}

void *get_socket_servers(ClientArgs *client_args) {
    int j;
    static int socket_create=0;
    static void *sock_to_servers =0;

    if( socket_create==1) return sock_to_servers;

    int num_servers = count_num_servers(client_args->servers_str);
    char **servers = create_server_names(client_args->servers_str);

    socket_create=1;
    zctx_t *ctx  = zctx_new();
    sock_to_servers = zsocket_new(ctx, ZMQ_DEALER);
    assert (sock_to_servers);
    zsocket_set_identity(sock_to_servers,  client_args->client_id);

    for(j=0; j < num_servers; j++) {
        char *destination = create_destination(servers[j], client_args->port);
        int rc = zsocket_connect(sock_to_servers, (const char *)destination);
        assert(rc==0);
        free(destination);
    }

    return sock_to_servers;
}
