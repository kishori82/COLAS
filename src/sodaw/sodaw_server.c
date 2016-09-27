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
#include "sodaw_client.h"
#include "sodaw_server.h"

#define DEBUG_MODE 1
extern int s_interrupted;

extern Server_Status *status;
extern Server_Args *server_args;

#ifdef ASLIBRARY
static zhash_t *hash_object_SODAW;
static zhash_t *metadata =NULL;
static zhash_t *readerc = NULL;
static int initialized = 0;

void  destroy_metadata(MetaData *m) {
    free(m->reader_opnum);
    free(m->serverid);
}


void  destroy_regreader(RegReader *r_tr) {
    free(r_tr->reader_op);
    free(r_tr->reader_id);
}

void initialize_SODAW() {
    initialized = 1;

    metadata = zhash_new();
    readerc =  zhash_new();

    hash_object_SODAW= zhash_new();

    create_metadata_sending_sockets() ;
    server_args->K =  ceil(((float)server_args->num_servers+1)/2);
}

MetaData *MetaData_create(Tag tag, char  *serverid, char *readerid, int opnum) {
    char buf[BUFSIZE];
    MetaData *h = (MetaData *)malloc(sizeof(MetaData));
    h->t_r.z= tag.z;
    strcpy(h->t_r.id, tag.id);

    strcpy(h->serverid, serverid);

    sprintf(buf, "%s_%d", readerid, opnum);
    strcpy(h->reader_opnum, buf);

    return h;
}

RegReader *RegReader_create(Tag tag, char *readerid, int opnum) {
    char buf[BUFSIZE];
    RegReader *h = (RegReader *)malloc(sizeof(RegReader));

    h->t_r.z= tag.z;
    strcpy(h->t_r.id, tag.id);

    sprintf(buf, "%s_%d", readerid, opnum);
    h->reader_op = (char *)malloc( (strlen(buf)+1)*sizeof(char));
    strcpy(h->reader_op, buf);

    h->reader_id = (char *)malloc( (strlen(readerid)+1)*sizeof(char));
    strcpy(h->reader_id, readerid);
    return h;
}

void  *reader_op_create(char *buf, char *readerid, int opnum) {
    sprintf(buf, "%s_%d", readerid, opnum);
}


char *MetaData_keystring(MetaData *m) {
    char buf[BUFSIZE];
    int size = 0;
    tag_to_string(m->t_r, buf);
    size += strlen(buf);
    buf[size++]='_';

    strncpy(buf+size, m->serverid, strlen(m->serverid));
    size += strlen(m->serverid);
    buf[size++]='_';

    strncpy(buf+size, m->reader_opnum, strlen(m->reader_opnum));
    size += strlen(m->reader_opnum);
    buf[size++]='\0';

    char *newkey = (char *)malloc(size*sizeof(char));
    strcpy(newkey, buf);

    return newkey;
}

char * RegReader_keystring(RegReader *m) {
    char buf[BUFSIZE];
    int size = 0;
    tag_to_string(m->t_r, buf);
    size += strlen(buf);
    buf[size++]='_';

    strncpy(buf+size, m->reader_op, strlen(m->reader_op));
    size += strlen(m->reader_op);
    buf[size++]='\0';

    char *newkey = (char *)malloc(size*sizeof(char));
    strcpy(newkey, buf);
    return newkey;
}

static void send_reader_coded_element(void *worker, char *reader,
                                      char *object_name,
                                      char *algorithm, char *phase,
                                      Tag tagw, zframe_t *cs) {
    char tag_w_buff[BUFSIZE];

    if(DEBUG_MODE) printf("\t\treaderid : %s\n", reader);
    zframe_t *reader_frame = zframe_new(reader, strlen(reader));
    zframe_send(&reader_frame, worker, ZFRAME_REUSE + ZFRAME_MORE);

    if(DEBUG_MODE) printf("\t\tobject : %s\n", object_name);
    zframe_t *object_frame = zframe_new(object_name, strlen(object_name));
    zframe_send(&object_frame, worker, ZFRAME_REUSE + ZFRAME_MORE);

    if(DEBUG_MODE) printf("\t\talgorithm : %s\n", algorithm);
    zframe_t *algorithm_frame = zframe_new(algorithm, strlen(algorithm));
    zframe_send(&algorithm_frame, worker, ZFRAME_REUSE + ZFRAME_MORE);

    if(DEBUG_MODE) printf("\t\tphase : %s\n", phase);
    zframe_t *phase_frame = zframe_new(phase, strlen(phase));
    zframe_send(&phase_frame, worker, ZFRAME_REUSE + ZFRAME_MORE);

    tag_to_string(tagw, tag_w_buff) ;
    if(DEBUG_MODE) printf("\t\ttag : %s\n", tag_w_buff);
    zframe_t *tag_frame = zframe_new(tag_w_buff, strlen(tag_w_buff));
    zframe_send(&tag_frame, worker, ZFRAME_REUSE + ZFRAME_MORE);

    if(DEBUG_MODE) printf("\t\tcoded-elem : %lu\n", zframe_size(cs));
    zframe_t *cs_frame = zframe_dup(cs);
    zframe_send(&cs_frame, worker, ZFRAME_REUSE);

    zframe_destroy(&reader_frame);
    zframe_destroy(&object_frame);
    zframe_destroy(&algorithm_frame);
    zframe_destroy(&phase_frame);
    zframe_destroy(&tag_frame);
    zframe_destroy(&cs_frame);
}

//void algorithm_SODAW_WRITE_PUT(char *ID,  zmsg_t *msg, zhash_t *frames,  void *worker, char *client, char *object_name, char *algorithm) {
void algorithm_SODAW_WRITE_PUT(zhash_t *frames,  void *worker) {
    char tag_w_str[BUFSIZE];
    char algorithm[BUFSIZE];
    char sender[BUFSIZE];
    char object_name[BUFSIZE];
    char ID[BUFSIZE];
    int opnum;

    //create the tag t_w as a string
    Tag tag_w;
    get_string_frame(tag_w_str, frames, TAG);
    string_to_tag(tag_w_str, &tag_w);

    if( DEBUG_MODE ) printf("\tWRITE PUT\n");

    // read the new coded element  from the message
    // this is the coded element cs
    zframe_t *payload = (zframe_t *)zhash_lookup(frames, PAYLOAD);
    int size = zframe_size(payload);

    get_string_frame(sender, frames, SENDER);
    get_string_frame(algorithm, frames, ALGORITHM);
    get_string_frame(ID, frames, "ID");
    get_string_frame(object_name, frames, OBJECT);
    opnum = get_int_frame(frames, OPNUM);

    // loop through all the existing (r, tr) pairs
    zlist_t *r_tr_keys = zhash_keys(readerc);
    int empty = zhash_size(readerc);

    void *key, *newkey;
    void *value;
    if(DEBUG_MODE) {
        print_object_hash(hash_object_SODAW);
    }


    for(key= zlist_first(r_tr_keys);  key!= NULL; key=zlist_next(r_tr_keys) ) {
        RegReader *value  = (RegReader *)zhash_lookup(readerc, (const char *)key);
        // char dest_reader  = get_reader_from_reader_op(dest_reader, value->readerid);
        if(  compare_tags(tag_w, value->t_r) >= 0 ) {
            printf("\t\tsending coded element\n");

            printf("Sending CODED_ELEMENT\n");
            /*
                        send_reader_coded_element(worker, value->reader_id,
                                                  object_name, algorithm, READ_VALUE,
                                                  tag_w, payload
                                                 );
            */
            /*
                        send_frames_at_server(frames, worker, SEND_FINAL, 6,
                                   SENDER, OBJECT,  ALGORITHM, PHASE, TAG, PAYLOAD);
            */

            MetaData *h = MetaData_create(tag_w, ID,  sender, opnum ) ;
            newkey = MetaData_keystring(h);

            zhash_insert((void *)metadata, (const char *)newkey, (void *)h);

            printf("\t\tsending...\n");
            char *types[] = {OBJECT, ALGORITHM, PHASE, META_TAG,
                             META_SERVERID, META_READERID
                            };

            /*
                         send_multicast_servers(
                               server_args->sock_to_servers, server_args->num_servers,
                               types,  6, object_name, "SODAW", READ_DISPERSE,
                               tag_w_str, h->serverid, h->readerid
                         );
            */

        }
    }

    zlist_purge(r_tr_keys);
    zlist_destroy(&r_tr_keys);

    //read the local tag
    Tag tag;
    get_object_tag(hash_object_SODAW, object_name, &tag);

    // if tw > tag
    if( compare_tags(tag_w, tag)==1 ) {
        // get the hash for the object
        zhash_t *temp_hash_hash = zhash_lookup(hash_object_SODAW, object_name);

        // get the stored keys (tags essentially)
        zlist_t *keys = zhash_keys(temp_hash_hash);

        // actually there should be only one key
        char *key = (char *)zlist_first(keys);

        printf("will replace %s\n", key);
        assert(key!=NULL);  // should not be empyt

        // get the  objects stored, i.e., the stored local value
        zframe_t *item = (zframe_t *)zhash_lookup(temp_hash_hash,key);
        assert(item!=NULL);


        // discount the metadata and data
        status->data_memory -= (float)zframe_size((zframe_t *)item);
        status->metadata_memory -=  (float)strlen(key);

        // discount the metadata and data
        //deleting it it

        zframe_destroy(&item);
        printf("deleting %s\n", key);
        zhash_delete(temp_hash_hash, key);

        if(DEBUG_MODE) {
            print_object_hash(hash_object_SODAW);
        }

        printf("deleted\n");

        item = (zframe_t *)zhash_lookup(temp_hash_hash,key);
        assert(item==NULL);
        //insert the new tag and coded value

        zframe_t *new_payload_frame = zframe_dup(payload);
        zhash_insert(temp_hash_hash, tag_w_str,(void *) new_payload_frame);

        //count the data size now
        status->metadata_memory +=  strlen(tag_w_str);
        status->data_memory += (float) size;

        zlist_purge(keys);
        zlist_destroy(&keys);
    }
    if(DEBUG_MODE) {
        print_object_hash(hash_object_SODAW);
    }
    printf("\t\tsending\n");

    send_frames_at_server(frames, worker, SEND_FINAL, 6,  SENDER, OBJECT,  ALGORITHM, PHASE, OPNUM, TAG);
    return;
}

void algorithm_SODAW_READ_COMPLETE(zhash_t *frames, void *worker) {
    char tag_r_str[BUFSIZE];
    char readerid[BUFSIZE];
    char reader_op[BUFSIZE];
    char ID[BUFSIZE];

    printf("\tREAD_COMPLETE..\n");
    Tag tag_r;
    get_string_frame(tag_r_str, frames, TAG);
    string_to_tag(tag_r_str, &tag_r);


    get_string_frame(readerid, frames, SENDER); // sender is the reader
    int opnum =  get_int_frame(frames, OPNUM);
    reader_op_create(reader_op, readerid, opnum);
    printf("\tREAD_COMPLETE..   ----- %s\n", reader_op);

    get_string_frame(ID, frames, "ID"); // sender is the reader

    RegReader *r_tr = RegReader_create(tag_r, readerid, opnum);
    char *r_tr_key = RegReader_keystring(r_tr);
    destroy_regreader(r_tr);

    void *item = zhash_lookup((void *)readerc, (const char *)r_tr_key);
    if( item != NULL) {
        zhash_delete((void *)readerc, (const char *)r_tr_key);
        zlist_t *Hr = metadata_with_reader(metadata, reader_op);
        metadata_remove_keys(metadata, Hr);
        //  zlist_destroy(&Hr);

    } else {
        Tag tag;
        init_tag(&tag);
        MetaData *h = MetaData_create(tag, ID, readerid, opnum) ;
        char *h_str = MetaData_keystring(h);
        zhash_insert((void *)metadata, (const char *)h_str, (void *)h);
        free(h_str);
    }
    free(r_tr_key);
    printf("\tREAD_COMPLETE\n");
    return;
}


/*
void algorithm_SODAW_READ_DISPERSE(zhash_t *frames,  void *worker) {
    char tag_str[BUFSIZE];
    char r_tag_pair_str[BUFSIZE];
    char serverid[BUFSIZE];
    char readerid[BUFSIZE];
    char object_name[BUFSIZE];

    printf("\tREAD_DISPERSE...\n");
    get_string_frame(object_name, frames, OBJECT);
    Tag tag;
    get_string_frame(tag_str, frames, META_TAG);
    string_to_tag(tag_str, &tag);

    get_string_frame(serverid, frames, META_SERVERID);
    get_string_frame(readerid, frames, META_READERID);

    MetaData *h = MetaData_create(tag, serverid, readerid);
    char *h_str_key = MetaData_keystring(h);
    zhash_insert(metadata, h_str_key, h);
    free(h_str_key);


    zlist_t *r_tr_keys = zhash_keys(readerc);
    void *key;
    for(key= zlist_first(r_tr_keys);  key!= NULL; key=zlist_next(r_tr_keys) ) {

        RegReader *r_tag_pair  = (RegReader *)zhash_lookup(readerc, (const char *)key);

        if(  strcmp(readerid, r_tag_pair->reader_op) == 0 ) {

            tag_to_string(tag, tag_str);

            zlist_t *Htr = metadata_with_tag_reader(metadata, tag, readerid);

            if( zlist_size(Htr) >= server_args->K) {
                char *r_tag_pair_char= RegReader_keystring(r_tag_pair);
                regreader_remove_key(readerc, r_tag_pair_char);
                free(r_tag_pair_char);

                zlist_t *Hr = metadata_with_reader(metadata, readerid);
                metadata_remove_keys(metadata, Hr);
                printf("======================================================>>>>>>>>>>>>>>>>>\n");
            }
            //          zlist_destroy(&Htr);
        }
    }
    //  zlist_destroy(&r_tr_keys);
    printf("\tREAD_DISPERSE\n");
    return;
}
*/


void algorithm_SODAW_WRITE_GET_OR_READ_GET_TAG(zhash_t *frames,const  char *phase,  void *worker) {
    char object_name[BUFSIZE];
    char tag_buf[BUFSIZE];

    printf("\t%s\n", phase);
    get_string_frame(object_name, frames, OBJECT);
    Tag tag;
    get_object_tag(hash_object_SODAW, object_name, &tag);

    tag_to_string(tag, tag_buf);

    zframe_t *tag_frame= zframe_new(tag_buf, strlen(tag_buf));
    zhash_insert(frames, TAG, (void *)tag_frame);

    int opnum = get_int_frame(frames, OPNUM);
    printf("\t\tsending\n");

    send_frames_at_server(frames, worker, SEND_FINAL, 6,  SENDER, OBJECT,  ALGORITHM, PHASE, OPNUM, TAG);

}

void algorithm_SODAW_WRITE_GET(zhash_t *frames,  void *worker) {
    algorithm_SODAW_WRITE_GET_OR_READ_GET_TAG(frames, WRITE_GET,  worker) ;
}

void algorithm_SODAW_READ_GET(zhash_t *frames,  void *worker) {
    algorithm_SODAW_WRITE_GET_OR_READ_GET_TAG(frames, READ_GET,  worker) ;
}



void algorithm_SODAW_READ_VALUE( zhash_t *frames, void *worker) {
    char buf[BUFSIZE];
    char _reader[BUFSIZE];
    char reader_opnum[BUFSIZE];
    char algorithm_name[BUFSIZE];
    char object_name[BUFSIZE];
    char tag_loc_str[BUFSIZE];
    char tag_inc_str[BUFSIZE];
    char tag_buf_str[BUFSIZE];

    printf("\tREAD_VALUE\n");
    Tag tag0;
    init_tag(&tag0);

    get_string_frame(_reader, frames, SENDER);
    int opnum =  get_int_frame(frames, OPNUM);
    get_string_frame(object_name, frames, OBJECT);

    reader_op_create(reader_opnum, _reader, opnum);

    get_string_frame(algorithm_name, frames, ALGORITHM);

    MetaData *h = MetaData_create(tag0, server_args->server_id, _reader, opnum);
    char *h_str = MetaData_keystring(h);

    Tag tag_r;
    get_tag_frame(frames, &tag_r);
    tag_to_string(tag0, buf);
    tag_to_string(tag_r, tag_inc_str);
    // printf("read value (%s,  %s,  %s,  %s)\n", h_str, buf, server_args->server_id, reader);

    if( zhash_lookup(metadata, h_str)!=NULL )  {
        zlist_t *Hr = metadata_with_reader(metadata, reader_opnum);
        metadata_remove_keys(metadata, Hr);
        zlist_purge(Hr);
    } else {
        RegReader *r_tr_pair = RegReader_create(tag_r, _reader,  opnum);

        char *r_tr_key = RegReader_keystring(r_tr_pair);
        zhash_insert(readerc, r_tr_key, (void *)r_tr_pair);
        free(r_tr_key);

        Tag tag_loc;
        get_object_tag(hash_object_SODAW, object_name, &tag_loc);
        tag_to_string(tag_loc, tag_loc_str);

        zframe_t  *payload_frame =
            (zframe_t *)get_object_frame(hash_object_SODAW, object_name, tag_loc);

        tag_to_string(tag_loc, tag_loc_str);
        if( compare_tags(tag_loc, tag_r)>=0 ) {

            tag_to_string(tag_loc, tag_loc_str);
            zframe_t *tag_loc_frame = zframe_new(tag_loc_str, strlen(tag_loc_str));

            zframe_t *item = (zframe_t *)zhash_lookup(frames, TAG);
            if(item!=NULL) zframe_destroy(&item);
            zhash_delete(frames, TAG);
            zhash_insert((void *)frames, TAG, (void *)tag_loc_frame);

            zframe_t *dup_payload_frame = zframe_dup(payload_frame);
            zhash_insert((void *)frames, PAYLOAD, (void *)dup_payload_frame);

            printf("\t\tsending...\n");
            /*
                           send_frames_at_server(frames, worker, SEND_FINAL, 6,
                                   SENDER, OBJECT,  ALGORITHM, PHASE, TAG, PAYLOAD);
            */

            printf("Relaying CODED_ELEMENT\n");
            send_reader_coded_element(worker, _reader,
                                      object_name, algorithm_name, READ_VALUE,
                                      tag_loc, payload_frame
                                     );


            MetaData *h = MetaData_create(tag_loc, server_args->server_id, _reader, opnum) ;
            char *newkey = MetaData_keystring(h);
            zhash_insert((void *)metadata, (const char *)newkey, (void *)h);

            // now do the READ-DISPERSE
            if(DEBUG_MODE) {
                print_object_hash(hash_object_SODAW);
            }

            printf("\t\tsending...\n");
            char *types[] = {OBJECT, ALGORITHM, PHASE, META_TAG,
                             META_SERVERID, META_READERID
                            };

            tag_to_string(h->t_r, tag_buf_str);

            /*
                          send_multicast_servers(
                               server_args->sock_to_servers, server_args->num_servers,
                               types,  6, object_name, "SODAW", READ_DISPERSE,
                               tag_buf_str, h->serverid, h->readerid
                          );
            */
            //  metadata_disperse(object_name, algorithm_name,  h);
        }
    }
}


void  metadata_remove_keys(zhash_t *metadata, zlist_t *Hr) {
    char *key;
    MetaData *m;

    for(key= zlist_first(Hr);  key!= NULL; key=zlist_next(Hr) ) {

        m = zhash_lookup(metadata, (const char *)key);
        free(m);

        zhash_delete((void *)metadata, (const char *)key);
    }
}

void  regreader_remove_key(zhash_t *regreaders, char *str_r_tr) {
    RegReader *r = zhash_lookup(readerc, (const char *)str_r_tr);
    free(r->reader_op);
    free(r->reader_id);
    free(r);
    zhash_delete((void *)regreaders, (const char *)str_r_tr);
}

zlist_t *metadata_with_reader(zhash_t *metadata, char *reader) {

    zlist_t *metadata_keys = zhash_keys(metadata);
    zlist_t *relevant_keys = zlist_new();

    void *key;
    for(key= zlist_first(metadata_keys);  key!= NULL; key=zlist_next(metadata_keys) ) {

        MetaData *meta  = (MetaData *)zhash_lookup(metadata, (const char *)key);
        if(  strcmp(reader, meta->reader_opnum) == 0 ) {
            zlist_append((void *)relevant_keys, key);
        }
    }

    return relevant_keys;
}

zlist_t *metadata_with_tag_reader(zhash_t *metadata, Tag tag, char *reader) {

    char tag_str[BUFSIZE];
    char tag_str_meta[BUFSIZE];
    zlist_t *metadata_keys = zhash_keys(metadata);
    zlist_t *relevant_keys = zlist_new();

    void *key;
    tag_to_string(tag, tag_str);

    for(key= zlist_first(metadata_keys);  key!= NULL; key=zlist_next(metadata_keys) ) {
        MetaData *meta  = (MetaData *)zhash_lookup(metadata, (const char *)key);
        assert(meta!=NULL);
        tag_to_string(meta->t_r, tag_str_meta);
        if(  strcmp(reader, meta->reader_opnum) == 0 &&
                strcmp(tag_str_meta, tag_str) == 0
          ) {
            zlist_append((void *)relevant_keys, key);
        }
    }
    //zlist_destroy(&metadata_keys);

    return relevant_keys;
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

    destroy_server_names(servers, num_servers);

    server_args->sock_to_servers = sock_to_servers;
}



void algorithm_SODAW(zhash_t *frames, void *worker, void *_server_args) {
    char phasebuf[BUFSIZE];
    char tag[BUFSIZE];
    char buf[BUFSIZE];
    char object_name[BUFSIZE];
    int  round;

    if(initialized==0) initialize_SODAW();

    get_string_frame(phasebuf, frames, PHASE);
    get_string_frame(object_name, frames, OBJECT);
    get_string_frame(buf, frames, SENDER);

    static int count =0;
    if( has_object(hash_object_SODAW, object_name)==0) {
        create_object(hash_object_SODAW, object_name, "SODAW", server_args->init_data, status);
    }

//    if( count++ > 10000 ) exit(EXIT_FAILURE);

    if( strcmp(phasebuf, WRITE_GET)==0)  {
        algorithm_SODAW_WRITE_GET(frames,  worker);
    } else if( strcmp(phasebuf, WRITE_PUT)==0)  {
        algorithm_SODAW_WRITE_PUT(frames, worker);
    } else if( strcmp(phasebuf, READ_GET)==0)  {
        algorithm_SODAW_READ_GET(frames, worker);
    } else if( strcmp(phasebuf, READ_VALUE)==0)  {
        algorithm_SODAW_READ_VALUE(frames, worker);
    } else if( strcmp(phasebuf, READ_COMPLETE)==0)  {
        algorithm_SODAW_READ_COMPLETE(frames, worker);
    } 

/*
else if( strcmp(phasebuf, READ_DISPERSE)==0)  {
        algorithm_SODAW_READ_DISPERSE(frames, worker);
    }
*/

}

#endif

//  The main thread simply starts several clients and a server, and then
//  waits for the server to finish.

#ifdef ASMAIN
int main (void) {
    int i ;
    zthread_new(server_task, NULL);
    zclock_sleep(60*60*1000);
    return 0;
}
#endif
