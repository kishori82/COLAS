#include "algo_utils.h"


#define DEBUG_MODE 1

#define WRITE_GET "WRITE_GET"
#define WRITE_PUT "WRITE_PUT"
#define READ_GET "READ_GET"
#define READ_VALUE "READ_VALUE"
#define READ_COMPLETE "READ_COMPLETE"
#define READ_DISPERSE "READ_DISPERSE"

#define PAYLOADBUF_SIZE 100000
#define BUFSIZE 100

void _zframe_int(zframe_t *f, int *i) {
    byte *data = zframe_data(f);
    *i = *((int *) data);
}

void _zframe_uint(zframe_t *f, unsigned  int *i) {
    byte *data = zframe_data(f);
    *i = *((unsigned int *) data);
}



void _zframe_str(zframe_t *f, char *buf) {
    byte *data = zframe_data(f);
    int s = zframe_size(f);
    int j;
    for( j = 0; j < s ; j++) {
           buf[j] = data[j]; 
     }
     buf[s]='\0';
}

void _zframe_value(zframe_t *f, char *buf) {
    byte *data = zframe_data(f);
    int s = zframe_size(f);
    int j;
    for( j = 0; j < s ; j++) {
           buf[j] = data[j]; 
     }
  //   buf[s]='\0';
}


unsigned int count_num_servers(char *servers_str) {
    int count = 0;
    if(strlen(servers_str)==0) return 0;
    count++;
    char *p = servers_str;
    while(*p !='\0') {
       if( *p==' ') count++;
       p++;
    }
    return count;
}

char **create_server_names(char *servers_str) {

    unsigned int num_servers =  count_num_servers(servers_str);

    char **servers = (char **)malloc(num_servers*sizeof(char *));
    char *p, *q;
    p = servers_str;
    int i = 0;
    while( *p!='\0') {
       servers[i] = (char *)malloc(16*sizeof(char));
       q = servers[i]; 
       while(*p !=' ' && *p !='\0') {
          *q++ = *p++; 
       }
       *q = '\0';
       if( *p == '\0') break;
       p++;
       i++;
    }
    return servers;
}

char *create_destinations(char **servers, unsigned int num_servers, char *port, char type) {
      int i,size = 0;
       
      for(i=0; i < num_servers; i++) { 
        size += strlen(servers[i]);
        size += strlen(port);
        size += 1; //for :
        size += 1; //for ,
      }
      
      char *dest = (char *)malloc( (size + 1)*sizeof(char));
      assert(dest!=0); 
      unsigned int pos =0;
      for(i=0; i < num_servers; i++) { 
         if(i==0)  dest[pos]=type;
         else  dest[pos]=',';
         pos += 1;

         memcpy(dest + pos, servers[i],  strlen(servers[i]));
         pos += strlen(servers[i]);

         dest[pos]=':';
         pos += 1;

         memcpy(dest + pos, port, strlen(port));
         pos += strlen(port);
      }
      dest[pos]='\0';

      return dest;
}

void init_tag(TAG *tag) {
       tag->z = 0;
       sprintf(tag->id, "%s", "client_0");

}

char *create_destination(char *server, char *port) {
      int size = 0;
      size += strlen(server);
      size += strlen(port);
      
      char *dest = (char *)malloc( (size + 8)*sizeof(char));
      assert(dest!=0);
      sprintf(dest, "tcp://%s:%s", server, port);
      return dest;
}

/*
   returns -1 if a < b
            0 if a = b
           +1 if a > b


*/

int compare_tag_ptrs(TAG *a, TAG *b) {
     if( a->z < b->z) return -2;

     if( a->z > b->z) return 1;

     return strcmp(a->id, b->id);
}


int compare_tags(TAG a, TAG b) {
     if( a.z < b.z) return -1;

     if( a.z > b.z) return +1;

     return strcmp(a.id, b.id);
}

void string_to_tag(char *str, TAG *tag) {
    char temp_buf[16];

    char *p, *q;
    p = temp_buf;
    q = str;
    while( *q!='_') {
      *p = *q; 
      q++;p++;
    }
    q++;  //skip '_' 
    *p='\0';
    tag->z = atoi(temp_buf);

    p = tag->id;
    while( *q!='\0') {
      *p = *q;
      q++;p++;
    }
    *p='\0';
    
}

void tag_to_string(TAG tag, char *buf) {

    sprintf(buf, "%d_%s", tag.z, tag.id);
}


TAG get_max_tag( zlist_t *tag_list) {
    TAG *tag;
    TAG max_tag;

    max_tag.z = -1;  // the z in the algorith starts from 0
    while( (tag = (TAG *)zlist_next(tag_list))!=NULL) {
         if(compare_tag_ptrs(tag, &max_tag)==1) { 
            max_tag.z = tag->z;
            strcpy(max_tag.id, tag->id);
          }
    }
    return max_tag;
}
     
void free_items_in_list( zlist_t *list) {
    void *item;

    while( (item = zlist_next(list))!=NULL) {
       free(item);
    }
}



int  get_object_tag(zhash_t *hash, char * object_name, TAG *tag) {
    char tag_str[64];
    
    void *item = zhash_lookup(hash, object_name);

    if( item==NULL) {
        return 0;
    }

    zhash_t *temp_hash = (zhash_t *)item;

    zhash_first(temp_hash);
    strcpy(tag_str, zhash_cursor(temp_hash)); 

    string_to_tag(tag_str, tag);
    return 1;
}

char *get_object_value(zhash_t *hash, char * object_name, TAG tag) {
    char tag_str[64];
    
    tag_to_string(tag, tag_str);

    void *item = zhash_lookup(hash, object_name);

    if( item==NULL) {
        return 0;
    }

    zhash_t *temp_hash = (zhash_t *)item;

    zhash_first(temp_hash);
    strcpy(tag_str, zhash_cursor(temp_hash)); 


    void *stored_value = zhash_lookup(temp_hash, tag_str);

    int size = strlen(stored_value);

    char *new_value = (char *)malloc( (size +1 ) *sizeof(char));
    strncpy(new_value, stored_value, size);
    new_value[size]='\0';

    return new_value;
}

zframe_t *get_object_frame(zhash_t *hash, char * object_name, TAG tag) {
    char tag_str[64];
    
    tag_to_string(tag, tag_str);

    void *item = zhash_lookup(hash, object_name);

    if( item==NULL) {
        return 0;
    }

    zhash_t *temp_hash = (zhash_t *)item;

    zhash_first(temp_hash);
    strcpy(tag_str, zhash_cursor(temp_hash)); 


    zframe_t *stored_value = zhash_lookup(temp_hash, tag_str);

    return stored_value;
}


int  get_string_frame(char *buf, zhash_t *frames, const char *str)  {
      zframe_t *frame= (zframe_t *)zhash_lookup(frames, str);
      if( frame==NULL) { buf[0]='\0'; return 0;}
      _zframe_str(frame, buf) ;
      return 1;     
}

int  get_int_frame(zhash_t *frames, const char *str)  {
      zframe_t *frame= zhash_lookup(frames, str);
      int val;
      if( frame==0) { return 0;}
      _zframe_int(frame, &val) ;
      return val;     
}

unsigned int  get_uint_frame(zhash_t *frames, const char *str)  {
      zframe_t *frame= zhash_lookup(frames, str);
      unsigned int val;
      if( frame==0) { return 0;}
      _zframe_uint(frame, &val) ;
      return val;     
}

int  get_tag_frame(zhash_t *frames, TAG *tag)  {
      char tag_str[BUFSIZE];
      get_string_frame(tag_str, frames, "tag");
      string_to_tag(tag_str, tag);
      return 1;     
}


void print_out_hash(zhash_t *frames) {
    unsigned int temp_int;
    char buf[PAYLOADBUF_SIZE];

     zlist_t *keys = zhash_keys(frames);

     char *key;
     for( key = (char *)zlist_first(keys);  key != NULL; key = (char *)zlist_next(keys)) {
          if( strcmp(key, "opnum")==0) {
            temp_int=get_uint_frame(frames, key);
            printf("\t\t\t%s : %d\n", key, temp_int);
            assert(temp_int >=0);
          }
          else if( strcmp(key, "payload")==0) {
             get_string_frame(buf, frames, key);
             printf("\t\t\t%s : %d\n", key, zframe_size(zhash_lookup(frames, key)));
          }
          else {
              get_string_frame(buf, frames, key);
              printf("\t\t\t%s : %s\n", key, buf);
          }
     }
}


void print_out_hash_in_order(zhash_t *frames, zlist_t* names) {
    unsigned int temp_int;
    char buf[PAYLOADBUF_SIZE];

     char *key;
     for( key = (char *)zlist_first(names);  key != NULL; key = (char *)zlist_next(names)) {
          if( strcmp(key, "opnum")==0) {
            temp_int=get_uint_frame(frames, key);
            printf("\t\t\t%s : %d\n", key, temp_int);
            assert(temp_int >=0);
          }
          else if( strcmp(key, "payload")==0) {
             get_string_frame(buf, frames, key);
             printf("\t\t\t%s : %d\n", key, zframe_size(zhash_lookup(frames, key)));
             if( zframe_size(zhash_lookup(frames, key)) < 100) {  printf("ERROR : small payload\n"); exit(0); }
          }
          else {
              get_string_frame(buf, frames, key);
              printf("\t\t\t%s : %s\n", key, buf);
          }

     }
}


zhash_t *receive_message_frames_at_server(zmsg_t *msg, zlist_t *names)  {
     char algorithm_name[BUFSIZE];
     char phase_name[BUFSIZE];
     zhash_t *frames = zhash_new();

     zframe_t *sender_frame = zmsg_pop (msg);
     zhash_insert(frames, "sender", (void *)sender_frame);
     if( names!= NULL) zlist_append(names, "sender");
   
     zframe_t *object_name_frame= zmsg_pop (msg);
     zhash_insert(frames, "object", (void *)object_name_frame);
     if(names!=NULL) zlist_append(names, "object");
       //    zframe_send (&object_name_frame, worker, ZFRAME_REUSE +ZFRAME_MORE );

     zframe_t *algorithm_frame= zmsg_pop (msg);
     zhash_insert(frames, "algorithm", (void *)algorithm_frame);
     get_string_frame(algorithm_name, frames, "algorithm");
     if(names!=NULL) zlist_append(names, "algorithm");

     zframe_t *phase_frame= zmsg_pop (msg);
     zhash_insert(frames, "phase", (void *)phase_frame);
     get_string_frame(phase_name, frames, "phase");
     if(names!=NULL) zlist_append(names, "phase");

     if( strcmp(algorithm_name, "ABD") ==0 ) {

         if( strcmp(phase_name, WRITE_VALUE) ==0 ) {
           zframe_t *opnum_frame= zmsg_pop (msg);
           zhash_insert(frames, "opnum", (void *)opnum_frame);
           if(names!=NULL) zlist_append(names, "opnum");

           zframe_t *tag_frame= zmsg_pop (msg);
           zhash_insert(frames, "tag", (void *)tag_frame);
           if(names!=NULL) zlist_append(names, "tag");

           zframe_t *payload_frame= zmsg_pop (msg);
           zhash_insert(frames, "payload", (void *)payload_frame);
           if(names!=NULL) zlist_append(names, "payload");

         }
         if( strcmp(phase_name, GET_TAG) ==0 ) {
           zframe_t *opnum_frame= zmsg_pop (msg);
           zhash_insert(frames, "opnum", (void *)opnum_frame);
           if(names!=NULL) zlist_append(names, "opnum");
         }

         if( strcmp(phase_name, GET_TAG_VALUE) ==0 ) {
            zframe_t *opnum_frame= zmsg_pop (msg);
            zhash_insert(frames, "opnum", (void *)opnum_frame);
           if(names!=NULL) zlist_append(names, "opnum");
         }

     }

     if( strcmp(algorithm_name, "SODAW") ==0 ) {

         if( strcmp(phase_name, WRITE_GET) ==0 ) {
           zframe_t *opnum_frame= zmsg_pop (msg);
           zhash_insert(frames, "opnum", (void *)opnum_frame);
           if(names!=NULL) zlist_append(names, "opnum");
         }

         if( strcmp(phase_name, WRITE_PUT) ==0 ) {
           zframe_t *opnum_frame= zmsg_pop (msg);
           zhash_insert(frames, "opnum", (void *)opnum_frame);
           if(names!=NULL) zlist_append(names, "opnum");

           zframe_t *tag_frame= zmsg_pop (msg);
           zhash_insert(frames, "tag", (void *)tag_frame);
           if(names!=NULL) zlist_append(names, "tag");

           zframe_t *payload_frame= zmsg_pop (msg);
           zhash_insert(frames, "payload", (void *)payload_frame);
           printf("size of payload %d\n", zframe_size(payload_frame));
           if(names!=NULL) zlist_append(names, "payload");
         }

         if( strcmp(phase_name, READ_VALUE) ==0 ) {
           zframe_t *tag_frame= zmsg_pop (msg);
           zhash_insert(frames, "tag", (void *)tag_frame);
           if(names!=NULL) zlist_append(names, "tag");
         }

         if( strcmp(phase_name, READ_DISPERSE) ==0 ) {
            zframe_t *meta_tag_frame= zmsg_pop (msg);
            zhash_insert(frames, "meta_tag", (void *)meta_tag_frame);
           if(names!=NULL) zlist_append(names, "meta_tag");

            zframe_t *serverid_frame= zmsg_pop (msg);
            zhash_insert(frames, "meta_serverid", (void *)serverid_frame);
           if(names!=NULL) zlist_append(names, "meta_serverid");

            zframe_t *meta_readerid_frame= zmsg_pop (msg);
            zhash_insert(frames, "meta_readerid", (void *)meta_readerid_frame);
           if(names!=NULL) zlist_append(names, "meta_readerid");
         }

          if( strcmp(phase_name, READ_COMPLETE) ==0 ) {
            zframe_t *tag_frame= zmsg_pop (msg);
            zhash_insert(frames, "tag", (void *)tag_frame);
           if(names!=NULL) zlist_append(names, "tag");
          }
     }
     return frames;
}


zhash_t *receive_message_frames_at_client(zmsg_t *msg, zlist_t *names)  {
     char algorithm_name[100];
     char object_name[100];
     char phase_name[100];
     zhash_t *frames = zhash_new();

     zframe_t *object_name_frame= zmsg_pop (msg);
     zhash_insert(frames, "object", (void *)object_name_frame);
     get_string_frame(object_name, frames, "object");
     if( names!= NULL) zlist_append(names, "object");
 
     zframe_t *algorithm_frame= zmsg_pop (msg);
     zhash_insert(frames, "algorithm", (void *)algorithm_frame);
     get_string_frame(algorithm_name, frames, "algorithm");
     if( names!= NULL) zlist_append(names, "algorithm");

     zframe_t *phase_frame= zmsg_pop (msg);
     zhash_insert(frames, "phase", (void *)phase_frame);
     get_string_frame(phase_name, frames, "phase");
     if( names!= NULL) zlist_append(names, "phase");

     if( strcmp(algorithm_name, "ABD") ==0 ) {

         zframe_t *opnum_frame= zmsg_pop (msg);
         zhash_insert(frames, "opnum", (void *)opnum_frame);
         if( names!= NULL) zlist_append(names, "opnum");

         if( strcmp(phase_name, GET_TAG) ==0 ) {
           zframe_t *tag_frame= zmsg_pop (msg);
           zhash_insert(frames, "tag", (void *)tag_frame);
           if( names!= NULL) zlist_append(names, "tag");

         }

         if( strcmp(phase_name, WRITE_VALUE) ==0 ) {

           zframe_t *tag_frame= zmsg_pop (msg);
           zhash_insert(frames, "tag", (void *)tag_frame);
           if( names!= NULL) zlist_append(names, "tag");
         }

         if( strcmp(phase_name, GET_TAG_VALUE) ==0 ) {
           zframe_t *tag_frame= zmsg_pop (msg);
           zhash_insert(frames, "tag", (void *)tag_frame);
           if( names!= NULL) zlist_append(names, "tag");

           zframe_t *payload_frame= zmsg_pop (msg);
           zhash_insert(frames, "payload", (void *)payload_frame);
           if( names!= NULL) zlist_append(names, "payload");
         }
     }

     if( strcmp(algorithm_name, "SODAW") ==0 ) {
         if( strcmp(phase_name, WRITE_GET) ==0 ) {
           zframe_t *opnum_frame= zmsg_pop (msg);
           zhash_insert(frames, "opnum", (void *)opnum_frame);
           if( names!= NULL) zlist_append(names, "opnum");

           zframe_t *tag_frame= zmsg_pop (msg);
           zhash_insert(frames, "tag", (void *)tag_frame);
           if( names!= NULL) zlist_append(names, "tag");
         }

         if( strcmp(phase_name, WRITE_PUT) ==0 ) {
           zframe_t *opnum_frame= zmsg_pop (msg);
           zhash_insert(frames, "opnum", (void *)opnum_frame);
           if( names!= NULL) zlist_append(names, "opnum");

           zframe_t *tag_frame= zmsg_pop (msg);
           zhash_insert(frames, "tag", (void *)tag_frame);
           if( names!= NULL) zlist_append(names, "tag");
         }

         if( strcmp(phase_name, READ_VALUE) ==0 ) {

           zframe_t *tag_frame= zmsg_pop (msg);
           zhash_insert(frames, "tag", (void *)tag_frame);
           if( names!= NULL) zlist_append(names, "tag");

           zframe_t *payload_frame= zmsg_pop (msg);
           zhash_insert(frames, "payload", (void *)payload_frame);
           if( names!= NULL) zlist_append(names, "payload");
         }
     }
 
     return frames;
}



void send_frames_at_server(zhash_t *frames, void *worker,  enum SEND_TYPE type, int n, ...) {
    char *key;
    va_list valist;
    int i =0;
    char buf[PAYLOADBUF_SIZE];
    unsigned int temp_int;

    va_start(valist, n);
     
    zlist_t *names = zlist_new();

    for(i=0; i < n; i++ ) {
        key = va_arg(valist, char *); 
        zframe_t *frame = (zframe_t *)zhash_lookup(frames, key);

        assert(zframe_is(frame));
        zlist_append(names, key);
/*
        if( strcmp(key, "opnum")==0) {
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



void destroy_frames(zhash_t *frames) {
     zlist_t *keys = zhash_keys(frames);
     char *key;
     for( key = (char *)zlist_first(keys);  key != NULL; key = (char *)zlist_next(keys)) {
           zframe_t *frame = (zframe_t *)zhash_lookup(frames, key);         
           if( frame!= NULL) {
              zhash_delete(frames, key);

              if( zframe_is(frame) ) zframe_destroy(&frame);
           }
     }
     zlist_destroy(&keys);
}


int has_object(zhash_t *object_hash,  char *obj_name) {
    void *item;
    item = zhash_lookup(object_hash, obj_name);
    if( item==NULL) {
       return 0;
    }
    return 1;
}


int create_object(zhash_t *object_hash, char *obj_name, char *algorithm, char *init_data, SERVER_STATUS *status) {
    void *item =NULL;
    char tag_str[BUFSIZE];
    TAG tag;

    item = zhash_lookup(object_hash, obj_name);
    if( item!= NULL) return 0;

    
    if( strcmp(algorithm, "ABD")==0) {
       zhash_t *hash_hash = zhash_new();

       init_tag(&tag);
       tag_to_string(tag, tag_str);

       char *value =(void *)malloc(strlen(init_data)+1);
       strcpy(value, init_data);
       value[strlen(init_data)]= '\0';
       zhash_insert(hash_hash, tag_str, (void *)value); 

       status->metadata_memory += (float) strlen(tag_str);
       status->data_memory += (float) strlen(init_data);
        
       printf("\tCreated %s (size %d) \n", obj_name, status->data_memory);
       //add it to the main list 
       zhash_insert(object_hash, obj_name, (void *)hash_hash); 

       return 1;
    }

    if( strcmp(algorithm, "SODAW")==0) {
       zhash_t *hash_hash = zhash_new();

       init_tag(&tag);
       tag_to_string(tag, tag_str);

       char *value =(void *)malloc(strlen(init_data)+1);
       strcpy(value, init_data);
       value[strlen(init_data)]= '\0';

       zframe_t *value_frame = zframe_new((void *)value, strlen(value));
       zhash_insert(hash_hash, tag_str, (void *)value_frame); 

       free(value);

       status->metadata_memory += (float) strlen(tag_str);
       status->data_memory += (float) strlen(init_data);
        
       printf("\tCreated %s (size %d) \n", obj_name, status->data_memory);
       //add it to the main list 
       zhash_insert(object_hash, obj_name, (void *)hash_hash); 

       return 1;
    }

    return 0;
}

int print_object_hash(zhash_t *object_hash) {

     printf("printing the object hash....\n");
     zlist_t *keys = zhash_keys(object_hash);
     char *key;
     printf("\t ========================================\n");
     for( key = (char *)zlist_first(keys);  key != NULL; key = (char *)zlist_next(keys)) {
           printf("\t Object : %s\n", key);
           zhash_t *single_object_hash = (zframe_t *)zhash_lookup(object_hash, key);         

           char *key1;
           zlist_t *keys1 = zhash_keys(single_object_hash);
           for( key1 = (char *)zlist_first(keys1);  key1 != NULL; key1 = (char *)zlist_next(keys1)) {
              printf("\t\t Tag: %s :   %d\n", key1, zframe_size((zframe_t *)zhash_lookup(single_object_hash, key1)));
           }
           zlist_purge(keys1);
     }
     printf("\t ========================================\n");
     zlist_purge(keys);
}
