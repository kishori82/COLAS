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

#include "sodaw_server.h"

#define DEBUG_MODE 1
extern int s_interrupted;

extern SERVER_STATUS *status;

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

METADATA *MetaData_create(TAG tag, char *readerid, char *serverid) {
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

void algorithm_SODAW_WRITE_PUT(char *ID,  zmsg_t *msg, zhash_t *frames,  void *worker, char *client, char *object_name, char *algorithm) {
    char tag_w_str[100];

    printf("====> 1\n");
    zframe_t *opnum_frame = zmsg_pop(msg);
    zhash_insert(frames, "opnum", (void *) opnum_frame);

    printf("=========> 2\n");
    zframe_t *tag_frame= zmsg_pop(msg);
    _zframe_str(tag_frame, tag_w_str) ;
    //create the tag t_w as a string 
    TAG tag_w; 
    string_to_tag(tag_w_str, &tag_w);
    printf("=============> 3 %s\n", tag_w_str);

    if( DEBUG_MODE ) printf("\t\t INSIDE WRITE PUT\n");

     // read the new coded element  from the message
    // this is the coded element cs
    zframe_t *payload_frame= zmsg_pop(msg);
    int size = zframe_size(payload_frame); 
    printf("====================> 4  %d\n", size);
    void *payload = zframe_data(payload_frame); 
    
 
    // loop through all the existing (r, tr) pairs 
    if( readerc==NULL) {
      printf("hull\n");
      printf("hull %d\n", initialized);
    }
    zlist_t *r_tr_keys = zhash_keys(readerc);
    int empty = zhash_size(readerc);

    void *key, *newkey;    
    void *value;
    for(key= zlist_first(r_tr_keys);  key!= NULL; key=zlist_next(r_tr_keys) ) {
         printf("==============================> 5 %d==\n", empty);
         value  = zhash_lookup(readerc, (const char *)key);
         printf("===============================> 6\n");
         if(  compare_tags(tag_w, ((REGREADER *)value)->t_r) >= 0 ) {
            send_reader_coded_element(worker, ((REGREADER*)value)->readerid, ((REGREADER *)value)->t_r, payload);

            printf("=====================================> 7\n");
            METADATA *h = MetaData_create(tag_w, client, ID) ;
            newkey = MetaData_keystring(h);

            zhash_insert((void *)metadata, (const char *)newkey, (void *)h);
         } 
    }

    printf("============================> 5\n");
    //read the local tag
    TAG tag;
    get_object_tag(hash_object_SODAW, object_name, &tag);

     // if tw > tag
    printf("===================================> 6\n");
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
        _zframe_str(payload_frame, data);  // put in the storable format
        zhash_insert(temp_hash_hash,tag_w_str, data); 

        //count the data size now
        free(data);
        status->metadata_memory +=  strlen(tag_w_str);
        status->data_memory += (float) size;
    }

    printf("===========================================> 7\n");

    zframe_t *sender_frame = (zframe_t *)zhash_lookup(frames, "sender");
    zframe_send(&sender_frame, worker, ZFRAME_REUSE + ZFRAME_MORE);

    zframe_t *object_frame = (zframe_t *) zhash_lookup(frames, "object");
    zframe_send(&object_frame, worker, ZFRAME_REUSE + ZFRAME_MORE);

    zframe_t *algorithm_frame = (zframe_t *) zhash_lookup(frames, "algorithm");
    zframe_send(&algorithm_frame, worker, ZFRAME_REUSE + ZFRAME_MORE);

    printf("=========================================================> 8\n");
    zframe_t *phase_frame = (zframe_t *) zhash_lookup(frames, "phase");
    zframe_send(&phase_frame, worker, ZFRAME_REUSE + ZFRAME_MORE);

    opnum_frame = (zframe_t *)zhash_lookup(frames, "opnum");
    zframe_send(&opnum_frame, worker, ZFRAME_REUSE + ZFRAME_MORE);

    printf("========================      =================================> 9\n");
    zframe_t *ack_frame = zframe_new("SUCCESS", strlen("SUCCESS"));
    zframe_send(&ack_frame, worker, ZFRAME_REUSE);

    printf("========================      =================================> 10\n");
    zframe_destroy(&tag_frame);
    printf("========================      =================================> 11\n");
    zframe_destroy(&ack_frame);
    zframe_destroy(&payload_frame);
    printf("========================      =================================> 14   %p\n", payload);
    free(payload);
    printf("========================      =    ===================================> 14\n");
    return;
}

void algorithm_SODAW_READ_COMPLETE(char *ID, zmsg_t *msg, void *worker, char *client, char *object_name, char *algorithm) {
    char tag_w_str[100];

    zframe_t *tag_frame= zmsg_pop(msg);
    _zframe_str(tag_frame, tag_w_str) ;

    TAG tag_w; 
    string_to_tag(tag_w_str, &tag_w);

     
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

          METADATA *h = MetaData_create(tag, client, ID) ;
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

          METADATA *h = MetaData_create(tag, client, ID);
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

     send_frames(frames, worker, SEND_FINAL, 6,  "sender", "object",  "algorithm", "phase", "opnum", "tag");

}

void algorithm_SODAW_READ_VALUE( zmsg_t *msg, void *worker, char *object_name) {
     char buf[100];
     TAG tag;
     get_object_tag(hash_object_SODAW, object_name, &tag); 

     tag_to_string(tag, buf);
     zframe_t *tag_frame= zframe_new(buf, strlen(buf));
     zframe_send(&tag_frame, worker, ZFRAME_REUSE + ZFRAME_MORE);
     zframe_destroy(&tag_frame);

     zhash_t *temp_hash_hash = zhash_lookup(hash_object_SODAW, object_name);

     zlist_t *keys = zhash_keys(temp_hash_hash);

     assert((int)zlist_size(keys)==1);
        
     void *key = zlist_first(keys);
     
     assert(key!=NULL);
     void *item = zhash_lookup(temp_hash_hash,key);

     zframe_t *data_frame= zframe_new((char *)item, strlen(item));
     zframe_send(&data_frame, worker, ZFRAME_REUSE);
     
     zframe_destroy(&data_frame);

}

void algorithm_SODAW(zhash_t *frames, void *worker, void *server_args) {
     char phasebuf[100];
     char tag[100]; 
     char buf[100]; 
     int  round;

     printf("algorithm SODAW\n");

     if(initialized==0) initialize_SODAW();

     get_string_frame(phasebuf, frames, "phase");
     get_string_frame(buf, frames, "sender");
     
      if( strcmp(phasebuf, WRITE_GET)==0)  {
           printf("\t-----------------\n");
           printf("\tSODAW WRITE_GET from client %s\n", buf);
           algorithm_SODAW_WRITE_GET_OR_READ_GET_TAG(frames,  worker);
      }

      if( strcmp(phasebuf, WRITE_PUT)==0)  {
           printf("\t-----------------\n");
           printf("\t SODAW WRITE PUT\n");
       //    algorithm_SODAW_WRITE_PUT(ID, msg, frames, worker, client, object_name, algorithm);
      }

      if( strcmp(phasebuf, READ_GET)==0)  {
           printf("\tSODAW READ_GET\n");
           printf("\tSODAW READ_GET from client %s\n",buf);

        //   algorithm_SODAW_WRITE_GET_OR_READ_GET_TAG(msg, frames, worker, object_name);
      }

      if( strcmp(phasebuf, READ_VALUE)==0)  {
           printf("\t-----------------\n");
           printf("\tSODAW READ VALUE\n");
        //   algorithm_SODAW_READ_COMPLETE(msg, worker, object_name);
      }


      if( strcmp(phasebuf, READ_COMPLETE)==0)  {
           printf("\t-----------------\n");
           printf("\tSODAW READ VALUE\n");
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
