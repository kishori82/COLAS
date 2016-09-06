//  Asynchronous client-to-server (DEALER to ROUTER)
//
//  While this example runs in a single process, that is to make
//  it easier to start and stop the example. Each task has its own
//  context and conceptually acts as a separate process.

#include <czmq.h> 
#include <zmq.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WRITE_GET "WRITE_GET"
#define WRITE_PUT "WRITE_PUT"
#define READ_GET "READ_GET"
#define READ_VALUE "READ_VALUE"
#define READ_COMPLETE "READ_COMPLETE"
#define READ_DISPERSE "READ_DISPERSE"
#include "sodaw_server.h"

#define DEBUG_MODE 1
extern int s_interrupted;

extern SERVER_STATUS *status;
extern SERVER_ARGS *server_args;

#ifdef ASLIBRARY
static zhash_t *hash_object_SODAW;

static zhash_t *metadata =NULL;
static zhash_t *readerc = NULL;
static int initialized = 0;

void initialize_SODAW() {
    initialized = 1;
    metadata = zhash_new();
    readerc =  zhash_new();
    hash_object_SODAW= zhash_new();
}

METADATA *MetaData_create(TAG tag, char  *serverid, char *readerid) {
     METADATA *h = (METADATA *)malloc(sizeof(METADATA));
     h->t_r = tag;
     h->readerid = (char *)malloc(strlen(readerid)*sizeof(char));
     h->serverid = (char *)malloc(strlen(serverid)*sizeof(char));
     return h;
}

REGREADER *RegReader_create(TAG tag, char *readerid) {
     REGREADER *h = (REGREADER *)malloc(sizeof(REGREADER));
     h->t_r = tag;
     h->readerid = (char *)malloc(strlen(readerid)*sizeof(char));
     return h;
}
 char *MetaData_keystring(METADATA *m) {
    char buf[100];
    int size = 0; 
    tag_to_string(m->t_r, buf);
    size += strlen(buf);
    buf[size++]='_';

    strncpy(buf+size, m->readerid, strlen(m->readerid));
    size += strlen(m->readerid);
    buf[size++]='_';
     
    strncpy(buf+size, m->serverid, strlen(m->serverid));
    size += strlen(m->serverid);
    buf[size++]='\0';
     
    char *newkey = (char *)malloc(size*sizeof(char));
    strcpy(newkey, buf);
    return newkey;
}

char * RegReader_keystring(REGREADER *m) {
    char buf[100];
    int size = 0; 
    tag_to_string(m->t_r, buf);
    size += strlen(buf);
    buf[size++]='_';

    strncpy(buf+size, m->readerid, strlen(m->readerid));
    size += strlen(m->readerid);
    buf[size++]='\0';
     
    char *newkey = (char *)malloc(size*sizeof(char));
    strcpy(newkey, buf);
    return newkey;
}

static void send_reader_coded_element(void *worker, char *reader, TAG tagw, char *cs) {
    char tag_w_buff[100];

    zframe_t *reader_frame = zframe_new(reader, strlen(reader));
    zframe_send(&reader_frame, worker, ZFRAME_REUSE + ZFRAME_MORE);

    tag_to_string(tagw, tag_w_buff) ;
    zframe_t *tag_frame = zframe_new(tag_w_buff, strlen(tag_w_buff));
    zframe_send(&tag_frame, worker, ZFRAME_REUSE + ZFRAME_MORE);

    zframe_t *cs_frame = zframe_new(reader, strlen(cs));
    zframe_send(&cs_frame, worker, ZFRAME_REUSE);

    zframe_destroy(&reader_frame);
    zframe_destroy(&tag_frame);
    zframe_destroy(&cs_frame);
}

//void algorithm_SODAW_WRITE_PUT(char *ID,  zmsg_t *msg, zhash_t *frames,  void *worker, char *client, char *object_name, char *algorithm) {
void algorithm_SODAW_WRITE_PUT(zhash_t *frames,  void *worker) {
    char tag_w_str[100];
    char sender[100];
    char object_name[100];
    char ID[100];

    //create the tag t_w as a string 
    TAG tag_w; 
    get_string_frame(tag_w_str, frames, "tag");
    string_to_tag(tag_w_str, &tag_w);

    if( DEBUG_MODE ) printf("\t\t INSIDE WRITE PUT\n");

     // read the new coded element  from the message
    // this is the coded element cs
    void *payload = zhash_lookup(frames, "payload"); 
    int size = zframe_size(payload);
    
    get_string_frame(sender, frames, "sender");
    get_string_frame(ID, frames, "ID");
    get_string_frame(object_name, frames, "object");
 
    // loop through all the existing (r, tr) pairs 
    zlist_t *r_tr_keys = zhash_keys(readerc);
    int empty = zhash_size(readerc);

    void *key, *newkey;    
    void *value;
    for(key= zlist_first(r_tr_keys);  key!= NULL; key=zlist_next(r_tr_keys) ) {
         value  = zhash_lookup(readerc, (const char *)key);
         if(  compare_tags(tag_w, ((REGREADER *)value)->t_r) >= 0 ) {
          send_reader_coded_element(worker, ((REGREADER*)value)->readerid, ((REGREADER *)value)->t_r, payload);

            METADATA *h = MetaData_create(tag_w, ID,  sender) ;
            newkey = MetaData_keystring(h);

            zhash_insert((void *)metadata, (const char *)newkey, (void *)h);
         } 
    }

    //read the local tag
    TAG tag;
    get_object_tag(hash_object_SODAW, object_name, &tag);

     // if tw > tag
    if( compare_tags(tag_w, tag)==1 ) {
        if( DEBUG_MODE )printf("\t\tBEHIND\n");
         // get the hash for the object
        zhash_t *temp_hash_hash = zhash_lookup(hash_object_SODAW, object_name);

        // get the stored keys (tags essentially)
        zlist_t *keys = zhash_keys (temp_hash_hash);

        // actually there should be only one key
        void *key = zlist_first(keys);  
        assert(key!=NULL);  // should not be empyt
          
        // get the  objects stored, i.e., the stored local value
        void *item = zhash_lookup(temp_hash_hash,key);
        assert(item!=NULL);

         // discount the metadata and data  
        status->data_memory -= (float)strlen((char *)item);
        status->metadata_memory -=  (float)strlen( (char *)key);
      
        //deleting it it    
        zhash_delete(temp_hash_hash, key);
        free(item);

        //insert the new tag and coded value
        char *data =  (char *)malloc(size + 1);
        _zframe_str(payload, data);  // put in the storable format

        zhash_insert(temp_hash_hash,tag_w_str, data); 

        //count the data size now
        status->metadata_memory +=  strlen(tag_w_str);
        status->data_memory += (float) size;
    }

    send_frames(frames, worker, SEND_FINAL, 6,  "sender", "object",  "algorithm", "phase", "opnum", "tag");
    printf("        <--- sending  --- %s\n",tag_w_str);
    return;
}

void algorithm_SODAW_READ_COMPLETE(zhash_t *frames, void *worker) {
    char tag_w_str[100];

    zframe_t *tag_frame= zmsg_pop(msg);
    _zframe_str(tag_frame, tag_w_str) ;

    TAG tag_w; 
    string_to_tag(tag_w_str, &tag_w);

    TAG tag_w; 
    get_string_frame(tag_w_str, frames, "tag");
    string_to_tag(tag_w_str, &tag_w);

    get_string_frame(sender, frames, "sender");
    get_string_frame(ID, frames, "ID");
    get_string_frame(object_name, frames, "object");
 

     
    REGREADER *r_tr = RegReader_create(tag_w, client);
    char *r_tr_key = RegReader_keystring(r_tr);
     

    void *item = zhash_lookup((void *)metadata, (const char *)r_tr_key);
    if( item != NULL) {
         zhash_delete((void *)metadata, (const char *)r_tr_key);
         zlist_t *metadata_keys = zhash_keys((void *)metadata);
         void *key;
         void *value;

         //line 26 in the paper Fig 5
         for( zlist_first(metadata_keys); zlist_next(metadata_keys)!=NULL; key = zlist_item(metadata_keys) ) {
            value  = zhash_lookup((void *)metadata_keys, (const char *)key);
            if( strcmp(  ((METADATA *)item)->readerid, client) == 0) {
                 zhash_delete( (void *)metadata, (const char *)key);
            };
         }
    }
    else {
          TAG tag;
          init_tag(&tag);

          METADATA *h = MetaData_create(tag, ID, client) ;
          char *h_str = MetaData_keystring(h);
          zhash_insert((void *)metadata, (const char *)h_str, (void *)h);
    }
    
    return;
}

void algorithm_SODAW_READ_DISPERSE(char *ID, zmsg_t *msg, void *worker, char *client, char *object_name, char *algorithm) {
    char tag_w_str[100];
    char serverid_str[100];
    char readerid_str[100];

    zframe_t *tag_frame= zmsg_pop(msg);
    _zframe_str(tag_frame, tag_w_str) ;
    TAG tag_w; 
    string_to_tag(tag_w_str, &tag_w);

    zframe_t *serverid_frame= zmsg_pop(msg);
    _zframe_str(tag_frame, serverid_str) ;

    zframe_t *readerid_frame= zmsg_pop(msg);
    _zframe_str(tag_frame, readerid_str) ;

    METADATA *h = MetaData_create(tag_w, serverid_str, readerid_str);

    REGREADER *r_tr = RegReader_create(tag_w, client);
    char *r_tr_key = RegReader_keystring(r_tr);
     

    void *item = zhash_lookup((void *)metadata, (const char *)r_tr_key);
    if( item != NULL) {
         zhash_delete((void *)metadata, (const char *)r_tr_key);
         zlist_t *metadata_keys = zhash_keys((void *)metadata);
         void *key;
         void *value;

         //line 26 in the paper Fig 5
         for( zlist_first(metadata_keys); zlist_next(metadata_keys)!=NULL; key = zlist_item(metadata_keys) ) {
            value  = zhash_lookup((void *)metadata_keys, (const char *)key);
            if( strcmp(  ((METADATA *)item)->readerid, client) == 0) {
                 zhash_delete( (void *)metadata, (const char *)key);
            };
         }
    }
    else {
          TAG tag;
          init_tag(&tag);

          METADATA *h = MetaData_create(tag,ID, client);
          char *h_str = MetaData_keystring(h);

          zhash_insert((void *)metadata, (const char *)h_str, (void *)h);
    }
    
    return;
}



void algorithm_SODAW_WRITE_GET_OR_READ_GET_TAG(zhash_t *frames,  void *worker) {
     char object_name[100];
     char tag_buf[100];
 

     get_string_frame(object_name, frames, "object");
     TAG tag;
     get_object_tag(hash_object_SODAW, object_name, &tag);
     tag_to_string(tag, tag_buf);

     zframe_t *tag_frame= zframe_new(tag_buf, strlen(tag_buf));
     zhash_insert(frames, "tag", (void *)tag_frame);

     int opnum = get_int_frame(frames, "opnum");
     printf("        READ_GET %d\n", opnum);
     send_frames(frames, worker, SEND_FINAL, 6,  "sender", "object",  "algorithm", "phase", "opnum", "tag");

     printf("        <--- sending  --- %s\n",tag_buf);
}




void algorithm_SODAW_READ_VALUE( zhash_t *frames, void *worker) {
     char buf[100];
     char sender[100];
     char ID[100];
     char algorithm_name[100];
     char object_name[100];

     TAG tag0;
     init_tag(&tag0);

     get_string_frame(sender, frames, "sender");
     get_string_frame(object_name, frames, "ID");

     METADATA *h = MetaData_create(tag0, ID, sender);
     char *h_str = MetaData_keystring(h);

     if( zhash_lookup(metadata, h_str)!=NULL )  {
          zlist_t *Hr = metadata_with_reader(metadata, sender);
          metadata_remove_keys(metadata, Hr);
     } else {
          TAG tag_r;
          get_tag_frame(frames, &tag_r);
          REGREADER *r_tr = RegReader_create(tag_r, sender);
          char *r_tr_key = RegReader_keystring(r_tr);
          zhash_insert(readerc, r_tr_key, (void *)r_tr);

          TAG tag_loc;
          get_string_frame(object_name, frames, "object");
          get_object_tag(hash_object_SODAW, object_name, &tag_loc);
          char *payload = get_object_value(hash_object_SODAW, object_name, tag_loc);

          if( compare_tags(tag_loc, tag_r)>=0 ) {
               send_reader_coded_element(worker, sender, tag_loc,  payload);

               METADATA *h = MetaData_create(tag_loc, ID, server_args->server_id) ;
               char *newkey = MetaData_keystring(h);
               zhash_insert((void *)metadata, (const char *)newkey, (void *)h);

          }
          // medatat READE_DISPWERSE
          get_string_frame(algorithm_name, frames, "algorithm");
          metadata_disperse(object_name, algorithm_name,  h);
     }

     destroy_frames(frames);
}


void  metadata_remove_keys(zhash_t *metadata, zlist_t *Hr) {
    char *key;

    for(key= zlist_first(Hr);  key!= NULL; key=zlist_next(Hr) ) {
       zhash_delete((void *)metadata, (const char *)key);
    }
}


zlist_t *metadata_with_reader(zhash_t *metadata, char *sender) {

    zlist_t *metadata_keys = zhash_keys(metadata);
    zlist_t *relevant_keys = zlist_new();

    void *key;    
    for(key= zlist_first(metadata_keys);  key!= NULL; key=zlist_next(metadata_keys) ) {
         void *value  = zhash_lookup(readerc, (const char *)key);
         if(  strcmp(sender, ((METADATA *)value)->readerid) == 0 ) {
            zlist_append((void *)relevant_keys, key);
         } 
    }
    return relevant_keys;
}

void metadata_disperse(char *object_name, char *algorithm,  METADATA *h) {

    int i; 
    zframe_t *obj_name_frame = zframe_new(object_name, strlen(object_name));
    zframe_t *algorithm_frame = zframe_new(algorithm, strlen(algorithm));
    zframe_t *phase_frame = zframe_new(READ_DISPERSE, strlen(READ_DISPERSE));

    char meta_tag_string[100];
    tag_to_string(h->t_r, meta_tag_string);
    zframe_t *meta_tag_frame = zframe_new( meta_tag_string, strlen(meta_tag_string));

    zframe_t *serverid_frame = zframe_new(h->serverid, strlen(h->serverid));
    zframe_t *readerid_frame = zframe_new(h->readerid, strlen(h->readerid));


    for(i=0; i < server_args->num_servers; i++) {
       zframe_send(&obj_name_frame, server_args->sock_to_servers, ZFRAME_REUSE + ZFRAME_MORE);
       zframe_send(&algorithm_frame, server_args->sock_to_servers, ZFRAME_REUSE + ZFRAME_MORE);
       zframe_send(&phase_frame, server_args->sock_to_servers, ZFRAME_REUSE + ZFRAME_MORE);
       zframe_send(&meta_tag_frame, server_args->sock_to_servers, ZFRAME_REUSE + ZFRAME_MORE);
       zframe_send(&serverid_frame, server_args->sock_to_servers, ZFRAME_REUSE + ZFRAME_MORE);
       zframe_send(&readerid_frame, server_args->sock_to_servers, ZFRAME_REUSE);
       printf("     \tsending to server %d\n",i);
    }

    zframe_destroy(&obj_name_frame);
    zframe_destroy(&algorithm_frame);
    zframe_destroy(&phase_frame);
    zframe_destroy(&meta_tag_frame);
    zframe_destroy(&serverid_frame);
    zframe_destroy(&readerid_frame);

}

void create_metadata_sending_sockets() {
    int num_servers = count_num_servers(server_args->servers_str);
    server_args->num_servers = num_servers;

    char **servers = create_server_names(server_args->servers_str);

    zctx_t *ctx  = zctx_new();
    void *sock_to_servers = zsocket_new(ctx, ZMQ_DEALER);
    zctx_set_linger(ctx, 0);
    assert (sock_to_servers);

    zsocket_set_identity(sock_to_servers,  server_args->server_id);
    int j; 
    for(j=0; j < num_servers; j++) {
        char *destination = create_destination(servers[j], server_args->port);
        int rc = zsocket_connect(sock_to_servers, destination);
        assert(rc==0);
        free(destination);
    }
   
    server_args->sock_to_servers = sock_to_servers;
}



void algorithm_SODAW(zhash_t *frames, void *worker, void *server_args) {
     char phasebuf[100];
     char tag[100]; 
     char buf[100]; 
     char object_name[100];
     int  round;

     printf("algorithm SODAW\n");

     if(initialized==0) initialize_SODAW();

     get_string_frame(phasebuf, frames, "phase");
     get_string_frame(object_name, frames, "object");


     if( has_object(hash_object_SODAW, object_name)==0) {
         create_object(hash_object_SODAW, object_name, ((SERVER_ARGS *)server_args)->init_data, status);
         printf("\t\tCreated object %s\n",object_name);
     }

     
      if( strcmp(phasebuf, WRITE_GET)==0)  {
           printf("\t-----------------\n");
           printf("\tSODAW WRITE_GET from client %s\n", buf);
           algorithm_SODAW_WRITE_GET_OR_READ_GET_TAG(frames,  worker);
      }

      if( strcmp(phasebuf, WRITE_PUT)==0)  {
           printf("\t-----------------\n");
           printf("\t SODAW WRITE PUT\n");
           algorithm_SODAW_WRITE_PUT(frames, worker);
      }

      if( strcmp(phasebuf, READ_GET)==0)  {
           printf("\tSODAW READ_GET\n");
           algorithm_SODAW_WRITE_GET_OR_READ_GET_TAG(frames, worker);
      }

      if( strcmp(phasebuf, READ_VALUE)==0)  {
           printf("\t-----------------\n");
           printf("\tSODAW READ VALUE\n");
           algorithm_SODAW_READ_VALUE(frames, worker);
      }


      if( strcmp(phasebuf, READ_COMPLETE)==0)  {
           printf("\t-----------------\n");
           printf("\tSODAW READ VALUE\n");
           algorithm_SODAW_READ_COMPLETE(frame, worker);
         //  algorithm_SODAW_READ_VALUE(msg, worker, object_name);
      }


   /* zframe_t *payloadf= zmsg_pop (msg);
           printf("%d\n",  (int)zframe_size(payloadf));
   */
   
   
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
