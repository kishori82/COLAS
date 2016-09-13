//  Asynchronous client-to-server (DEALER to ROUTER)
//
//  While this example runs in a single process, that is to make
//  it easier to start and stop the example. Each task has its own
//  context and conceptually acts as a separate process.

#include "server.h"

#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>

extern int s_interrupted;

#define S_NOTIFY_MSG " "
#define S_ERROR_MSG "Error while writing to self-pipe.\n"
static int s_fd;
static void s_signal_handler1 (int signal_value)
{
    int rc = write (s_fd, S_NOTIFY_MSG, sizeof(S_NOTIFY_MSG));
    if (rc != sizeof(S_NOTIFY_MSG)) {
        write (STDOUT_FILENO, S_ERROR_MSG, sizeof(S_ERROR_MSG)-1);
        exit(1);
    }
}




static void s_catch_signals1 (int fd)
{
    s_fd = fd;

    struct sigaction action;
    action.sa_handler = s_signal_handler1;
    //  Doesn't matter if SA_RESTART set because self-pipe will wake up zmq_poll
    //  But setting to 0 will allow zmq_read to be interrupted.
    action.sa_flags = 0;
    sigemptyset (&action.sa_mask);
    sigaction (SIGINT, &action, NULL);
    sigaction (SIGTERM, &action, NULL);
}


Server_Status *status;
Server_Args *server_args;

#ifdef ASLIBRARY

//  .split server task
//  This is our server task.
//  It uses the multithreaded server model to deal requests out to a pool
//  of workers and route replies back to clients. One worker can handle
//  one request at a time but one client can talk to multiple workers at
//  once.

static void server_worker (void *args, zctx_t *ctx, void *pipe);


void *server_task (void *server_args)
{

    //  Frontend socket talks to clients over TCP
    zctx_t *ctx = zctx_new ();
    void *frontend = zsocket_new(ctx, ZMQ_ROUTER);
    zsocket_bind(frontend, "tcp://*:8081");
    
    //  Backend socket talks to workers over inproc
    void *backend = zsocket_new (ctx, ZMQ_DEALER);
    zsocket_bind (backend, "inproc://backend");

    //  Launch pool of worker threads, precise number is not critical
 //   for (thread_nbr = 0; thread_nbr < 5; thread_nbr++)
    zthread_fork (ctx, server_worker, server_args);

    //  Connect backend to frontend via a proxy
    zmq_proxy (frontend, backend, NULL);

    zctx_destroy (&ctx);

    return NULL;
}



static void
server_worker (void *_server_args, zctx_t *ctx, void *pipe1)
{
    void *worker = zsocket_new (ctx, ZMQ_DEALER);
    zsocket_connect(worker, "inproc://backend");
    char algorithm_name[100];

    Server_Args *server_args = (Server_Args *)_server_args;
    
    printf("Initial value size %ld\n", strlen(server_args->init_data));

/*
    int pipefds[2];
    int rc= pipe(pipefds);

    if (rc != 0) {
        perror("Creating self-pipe");
        exit(1);
    }
    for (int i = 0; i < 2; i++) {
        int flags = fcntl(pipefds[i], F_GETFL, 0);
        if (flags < 0) {
            perror ("fcntl(F_GETFL)");
            exit(1);
        }
        rc = fcntl (pipefds[i], F_SETFL, flags | O_NONBLOCK);
        if (rc != 0) {
            perror ("fcntl(F_SETFL)");
            exit(1);
        }
    }

    s_catch_signals1(pipefds[1]);
*/

    zmq_pollitem_t items[] = { { worker, 0, ZMQ_POLLIN, 0}};
    while (true) {
        int rc = zmq_poll(items, 1, -1);
        if( rc < 0 || s_interrupted==1) {
             exit(0);
        }
        
        zclock_sleep(5);
        if (items[0].revents & ZMQ_POLLIN) {
           zmsg_t *msg = zmsg_recv (worker);

           // receive the frames
           status->network_data += (float)zmsg_content_size(msg) ;

           zlist_t *frames_list = zlist_new(); 
           zhash_t *frames = receive_message_frames_at_server(msg, frames_list);

           get_string_frame(algorithm_name, frames, ALGORITHM);

           if( strcmp(algorithm_name, "ABD")==0)  {
                printf(" [[[ %s\n", server_args->server_id);
                if(DEBUG_MODE) { 
                   printf("\t\treceiving... %s\n", server_args->server_id);  
                   print_out_hash_in_order(frames, frames_list); 
                }
                 
                algorithm_ABD(frames, worker, server_args);
                printf("   ]]]\n\n");
           }
  
   
           if( strcmp(algorithm_name, SODAW)==0)  {

                printf(" [[[ %s\n",server_args->server_id);
                if(DEBUG_MODE) {
                   printf("\t\treceiving... %s\n", server_args->server_id);  
                   print_out_hash_in_order(frames, frames_list);
                }

                algorithm_SODAW(frames, worker, server_args);
                printf("  ]]]\n\n");
           }

           zlist_purge(frames_list);
           destroy_frames(frames);
        }
    }
   
}

//int server_process(char *server_id, char *servers_str, char *port, char *init_data, Server_Status *_status)
int server_process(Server_Args *server_args, Server_Status *_status)
{
   status = _status;

   zthread_new(server_task, (void *)server_args);
   printf("Starting the thread id %s\n", server_args->server_id);

   while(true) {
      zclock_sleep(60*600*1000); 
   }
   free(server_args);
   return 0;
}


int store_payload(zhash_t *object_hash, char *obj_name, Tag tag, zframe_t *payload, enum INSERT_DATA_POLICY policy) {
    char tag_str[BUFSIZE];

    zhash_t *single_object_hash = (zhash_t *)zhash_lookup(object_hash, obj_name);
    tag_to_string(tag, tag_str);

    if( single_object_hash==NULL) {
       single_object_hash = zhash_new();
       zhash_insert(object_hash, obj_name, (void *)single_object_hash); 
    }
    zframe_t *payload_frame = (zframe_t *)zhash_lookup(single_object_hash, tag_str);
    if( policy == yield) {
        if(payload_frame!=NULL) return -1;
    }
    else if(policy==force){
        if(payload_frame!=NULL) {
           zframe_destroy(&payload_frame);
           zhash_delete(single_object_hash, tag_str);
        }
    }

    return(zhash_insert(single_object_hash, tag_str, (void *)payload)); 
}


int create_object(zhash_t *object_hash, char *obj_name, char *algorithm, 
                 char *init_data, Server_Status *status) {
    void *item =NULL;
    char tag_str[BUFSIZE];
    Tag tag;

    item = zhash_lookup(object_hash, obj_name);
    if( item!= NULL) return 0;

    
    init_tag(&tag);
    tag_to_string(tag, tag_str);

    if( strcmp(algorithm, "ABD")==0) {
       zhash_t *hash_hash = zhash_new();

       char *value =(void *)malloc(strlen(init_data)+1);
       strcpy(value, init_data);
       value[strlen(init_data)]= '\0';

      
       zhash_insert(hash_hash, tag_str, (void *)value); 

       status->metadata_memory += (float) strlen(tag_str);
       status->data_memory += (float) strlen(init_data);
        
       printf("\tCreated \"%s\" (size %d) \n", obj_name, (int)status->data_memory);
       //add it to the main list 
       zhash_insert(object_hash, obj_name, (void *)hash_hash); 

       return 1;
    }

    if( strcmp(algorithm, SODAW)==0) {

       printf("object lookup :  %s\n",obj_name);
       char *value =(void *)malloc(strlen(init_data)+1);
       strcpy(value, init_data);
       value[strlen(init_data)]= '\0';

       zframe_t *payload_frame = zframe_new((void *)value, strlen(init_data));
       store_payload(object_hash, obj_name, tag, payload_frame, yield) ;
       free(value);

       status->metadata_memory += (float) strlen(tag_str);
       status->data_memory += (float) strlen(init_data);
        
       printf("\tCreated \"%s\" (size %d) \n", obj_name, status->data_memory, strlen(init_data));
       //add it to the main list 
      return 1;
   }
   
   return 0;
}

zhash_t *receive_message_frames_at_server(zmsg_t *msg, zlist_t *names)  {
     char algorithm_name[BUFSIZE];
     char phase_name[BUFSIZE];
     zhash_t *frames = zhash_new();

     zframe_t *sender_frame = zmsg_pop (msg);
     zhash_insert(frames, SENDER, (void *)sender_frame);
     if( names!= NULL) zlist_append(names, SENDER);
   
     zframe_t *object_name_frame= zmsg_pop (msg);
     zhash_insert(frames, OBJECT, (void *)object_name_frame);
     if(names!=NULL) zlist_append(names, OBJECT);
       //    zframe_send (&object_name_frame, worker, ZFRAME_REUSE +ZFRAME_MORE );

     zframe_t *algorithm_frame= zmsg_pop (msg);
     zhash_insert(frames, ALGORITHM, (void *)algorithm_frame);
     get_string_frame(algorithm_name, frames, ALGORITHM);
     if(names!=NULL) zlist_append(names, ALGORITHM);

     zframe_t *phase_frame= zmsg_pop (msg);
     zhash_insert(frames, PHASE, (void *)phase_frame);
     get_string_frame(phase_name, frames, PHASE);
     if(names!=NULL) zlist_append(names, PHASE);

     if( strcmp(algorithm_name, "ABD") ==0 ) {

         if( strcmp(phase_name, WRITE_VALUE) ==0 ) {
           zframe_t *opnum_frame= zmsg_pop (msg);
           zhash_insert(frames, OPNUM, (void *)opnum_frame);
           if(names!=NULL) zlist_append(names, OPNUM);

           zframe_t *tag_frame= zmsg_pop (msg);
           zhash_insert(frames, TAG, (void *)tag_frame);
           if(names!=NULL) zlist_append(names, TAG);

           zframe_t *payload_frame= zmsg_pop (msg);
           zhash_insert(frames, PAYLOAD, (void *)payload_frame);
           if(names!=NULL) zlist_append(names, PAYLOAD);
         }

         if( strcmp(phase_name, GET_TAG) ==0 ) {
           zframe_t *opnum_frame= zmsg_pop (msg);
           zhash_insert(frames, OPNUM, (void *)opnum_frame);
           if(names!=NULL) zlist_append(names, OPNUM);
         }

         if( strcmp(phase_name, GET_TAG_VALUE) ==0 ) {
            zframe_t *opnum_frame= zmsg_pop (msg);
            zhash_insert(frames, OPNUM, (void *)opnum_frame);
           if(names!=NULL) zlist_append(names, OPNUM);
         }

     }

     if( strcmp(algorithm_name, SODAW) ==0 ) {

         if( strcmp(phase_name, WRITE_GET) ==0 ) {
           zframe_t *opnum_frame= zmsg_pop (msg);
           zhash_insert(frames, OPNUM, (void *)opnum_frame);
           if(names!=NULL) zlist_append(names, OPNUM);
         }

         if( strcmp(phase_name, READ_GET) ==0 ) {
           zframe_t *opnum_frame= zmsg_pop (msg);
           zhash_insert(frames, OPNUM, (void *)opnum_frame);
           if(names!=NULL) zlist_append(names, OPNUM);
         }


         if( strcmp(phase_name, WRITE_PUT) ==0 ) {
           zframe_t *opnum_frame= zmsg_pop (msg);
           zhash_insert(frames, OPNUM, (void *)opnum_frame);
           if(names!=NULL) zlist_append(names, OPNUM);

           zframe_t *tag_frame= zmsg_pop (msg);
           zhash_insert(frames, TAG, (void *)tag_frame);
           if(names!=NULL) zlist_append(names, TAG);

           zframe_t *payload_frame= zmsg_pop (msg);
           zhash_insert(frames, PAYLOAD, (void *)payload_frame);
           printf("size of payload %lu\n", zframe_size(payload_frame));
           if(names!=NULL) zlist_append(names, PAYLOAD);
         }

         if( strcmp(phase_name, READ_VALUE) ==0 ) {
           zframe_t *opnum_frame= zmsg_pop (msg);
           zhash_insert(frames, OPNUM, (void *)opnum_frame);
           if(names!=NULL) zlist_append(names, OPNUM);

           zframe_t *tag_frame= zmsg_pop (msg);
           zhash_insert(frames, TAG, (void *)tag_frame);
           if(names!=NULL) zlist_append(names, TAG);
         }

         if( strcmp(phase_name, READ_DISPERSE) ==0 ) {
            zframe_t *meta_tag_frame= zmsg_pop (msg);
            zhash_insert(frames, META_TAG, (void *)meta_tag_frame);
           if(names!=NULL) zlist_append(names, META_TAG);

            zframe_t *serverid_frame= zmsg_pop (msg);
            zhash_insert(frames, META_SERVERID, (void *)serverid_frame);
           if(names!=NULL) zlist_append(names, META_SERVERID);

            zframe_t *meta_readerid_frame= zmsg_pop (msg);
            zhash_insert(frames, META_READERID, (void *)meta_readerid_frame);
           if(names!=NULL) zlist_append(names, META_READERID);
         }

          if( strcmp(phase_name, READ_COMPLETE) ==0 ) {
            zframe_t *tag_frame= zmsg_pop (msg);
            zhash_insert(frames, TAG, (void *)tag_frame);
           if(names!=NULL) zlist_append(names, TAG);
          }
     }
     return frames;
}

void send_frames_at_server(zhash_t *frames, void *worker,  enum SEND_TYPE type, int n, ...) {
    char *key;
    va_list valist;
    int i =0;

    va_start(valist, n);
     
    zlist_t *names = zlist_new();

    for(i=0; i < n; i++ ) {
        key = va_arg(valist, char *); 
        zframe_t *frame = (zframe_t *)zhash_lookup(frames, key);

        printf("\t\t%s\n", key);
        assert(zframe_is(frame));
        zlist_append(names, key);
/*
        if( strcmp(key, OPNUM)==0) {
            temp_int=get_uint_frame(frames, key);
            printf("\t\t\t%s : %d\n", key, temp_int);
            assert(temp_int >=0);
        }
        else if( strcmp(key, "payload")==0) {
           get_string_frame(buf, frames, key);
           printf("\t\t\t%s : %d\n", (char *)key, strlen(buf));
        }
        else {
           get_string_frame(buf, frames, key);
            printf("\t\t\t%s : %s\n", key, buf);
        }
*/

        if( i == n-1 && type==SEND_FINAL)  {
            zframe_send(&frame, worker, ZFRAME_REUSE);
        }
        else
            zframe_send(&frame, worker, ZFRAME_REUSE + ZFRAME_MORE);
    }

    if(DEBUG_MODE) print_out_hash_in_order(frames, names);

    zlist_purge(names);
    va_end(valist);
}

#endif

//  The main thread simply starts several clients and a server, and then
//  waits for the server to finish.

#ifdef ASMAIN
int main (void)
{
   int i ; 
   zthread_new(server_task, NULL);
   zclock_sleep(60*60*1000); 
   return 0;
}
#endif
