#include "algo_utils.h"


extern int s_interrupted;
void s_signal_handler(int signal_value)
{
    s_interrupted=1;
}

void s_catch_signals ()
{
    struct sigaction action;
    action.sa_handler = s_signal_handler;
    //  Doesn't matter if SA_RESTART set because self-pipe will wake up zmq_poll
    //  But setting to 0 will allow zmq_read to be interrupted.
    s_interrupted=0;
    action.sa_flags = 0;
    sigemptyset (&action.sa_mask);
    sigaction (SIGINT, &action, NULL);
    sigaction (SIGTERM, &action, NULL);
}



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

void  destroy_server_names(char **servers, int num_servers) {
   int i =0; 
   for(i=0; i < num_servers; i++) {
      free(servers[i]);
   }
   free(servers);

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

void init_tag(Tag *tag) {
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

int compare_tag_ptrs(Tag *a, Tag *b) {
     if( a->z < b->z) return -2;

     if( a->z > b->z) return 1;

     return strcmp(a->id, b->id);
}


int compare_tags(Tag a, Tag b) {
     if( a.z < b.z) return -1;

     if( a.z > b.z) return +1;

     return strcmp(a.id, b.id);
}

void string_to_tag(char *str, Tag *tag) {
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

void tag_to_string(Tag tag, char *buf) {

    sprintf(buf, "%d_%s", tag.z, tag.id);
}


Tag *get_max_tag( zlist_t *tag_list) {
    Tag *tag;
    Tag *max_tag = (Tag *)malloc(sizeof(Tag));

    max_tag->z = -1;  // the z in the algorith starts from 0
    while( (tag = (Tag *)zlist_next(tag_list))!=NULL) {
         if(compare_tag_ptrs(tag, max_tag)==1) { 
            max_tag->z = tag->z;
            strcpy(max_tag->id, tag->id);
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



int  get_object_tag(zhash_t *hash, char * object_name, Tag *tag) {
    char tag_str[64];
    
    void *item = zhash_lookup(hash, object_name);

    if( item==NULL) {
        return 0;
    }

    zhash_t *temp_hash = (zhash_t *)item;
    assert(temp_hash!=NULL);

    assert(zhash_first(temp_hash)!=NULL);

    strcpy(tag_str, zhash_cursor(temp_hash)); 
    string_to_tag(tag_str, tag);

    return 1;
}

char *get_object_value(zhash_t *hash, char * object_name, Tag tag) {
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

zframe_t *get_object_frame(zhash_t *hash, char * object_name, Tag tag) {
    char tag_str[100];
    
    tag_to_string(tag, tag_str);
    zhash_t *single_object_hash = (zhash_t *)zhash_lookup(hash, object_name);
    assert(single_object_hash!=NULL);

    zframe_t *stored_value = (zframe_t *)zhash_lookup(single_object_hash, tag_str);

    assert(stored_value!=NULL);
    zframe_is(stored_value);

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

int  get_tag_frame(zhash_t *frames, Tag *tag)  {
      char tag_str[BUFSIZE];
      get_string_frame(tag_str, frames, TAG);
      string_to_tag(tag_str, tag);
      return 1;     
}


void print_out_hash(zhash_t *frames) {
    unsigned int temp_int;
    char buf[PAYLOADBUF_SIZE];

     zlist_t *keys = zhash_keys(frames);

     char *key;
     for( key = (char *)zlist_first(keys);  key != NULL; key = (char *)zlist_next(keys)) {
          printf("\t\t\t%s : ", key);
          if( strcmp(key, OPNUM)==0) {
            temp_int=get_uint_frame(frames, key);
            printf("%d\n", temp_int);
            assert(temp_int >=0);
          }
          else if( strcmp(key, PAYLOAD)==0) {
             zframe_t *s = zhash_lookup(frames,key);
             if(s!=NULL) printf("%lu\n", zframe_size(s));
          }
          else {
              get_string_frame(buf, frames, key);
              printf("%s\n", buf);
          }
     }
     zlist_purge(keys);
     zlist_destroy(&keys);
}


void print_out_hash_in_order(zhash_t *frames, zlist_t* names) {
    unsigned int temp_int;
    char buf[PAYLOADBUF_SIZE];

     char *key;
     for( key = (char *)zlist_first(names);  key != NULL; key = (char *)zlist_next(names)) {
          if( strcmp(key, OPNUM)==0) {
            temp_int=get_uint_frame(frames, key); 
            printf("\t\t\t%s : %d\n", key, temp_int);
            assert(temp_int >=0);
          }
          else if( strcmp(key, PAYLOAD)==0) {
             printf("\t\t\t%s : %lu\n", key, zframe_size((zframe_t *)zhash_lookup(frames, key)));
             if( zframe_size(zhash_lookup(frames, key)) < 100) {  printf("ERROR : small payload\n"); exit(0); }
          }
          else {
              get_string_frame(buf, frames, key);
              printf("\t\t\t%s : %s\n", key, buf);
          }
     }


}



void destroy_frames(zhash_t *frames) {

   //  printf("\t\tDeleting frames\n");
     zlist_t *keys = zhash_keys(frames);
     char *key;
     for( key = (char *)zlist_first(keys);  key != NULL; key = (char *)zlist_next(keys)) {
           zframe_t *frame = (zframe_t *)zhash_lookup(frames, key);         
           if( frame!= NULL) {
    //          printf("\t\t\t%s : %d\n",key, zframe_size(frame));
              zhash_delete(frames, key);
              if( zframe_is(frame) ) zframe_destroy(&frame);
           }
     }
     //printf("\t\t\t%s : %d\n","frames", zhash_size(frames));
     zlist_destroy(&keys);
     zhash_destroy(&frames);
}


/*
void destroy_zlist(zlist_t *list) {

     zlist_t *keys = zhash_keys(frames);
     char *key;
     for( key = (char *)zlist_first(keys);  key != NULL; key = (char *)zlist_next(keys)) {
         zlist_destroy(&keys);
     }

     zhash_destroy(&list);
}
*/




int has_object(zhash_t *object_hash,  char *obj_name) {
    void *item;
    item = zhash_lookup(object_hash, obj_name);
    if( item==NULL) {
       return 0;
    }
    return 1;
}

bool is_equal(char *payload1, char*payload2, unsigned int size) {
    int i =0;
//    bool final=true;
    for(i=0; i <size ; i++) {
        if(payload1[i]!=payload2[i]) {
           printf("INFO: Mismatch at index %d  (%c %c)\n",i, payload1[i], (char )payload2[i]);
      //     final=false;
           return false;
        }
    }
    return true;
   // return final; //true;
}




int print_object_hash(zhash_t *object_hash) {

     printf("\tprinting the object hash....\n");
     zlist_t *keys = zhash_keys(object_hash);
     char *key;
     printf("\t ========================================\n");
     for( key = (char *)zlist_first(keys);  key != NULL; key = (char *)zlist_next(keys)) {
           printf("\t Object : %s\n", key);
           zhash_t *single_object_hash = (zhash_t *)zhash_lookup(object_hash, key);         
           assert(single_object_hash!=NULL);

           char *key1;
           zlist_t *keys1 = zhash_keys(single_object_hash);
          
           assert(keys1!=NULL);

           for( key1 = (char *)zlist_first(keys1);  key1 != NULL; key1 = (char *)zlist_next(keys1)) {
            zhash_lookup(single_object_hash, key1);
              printf("\t\t Tag: %s :   %lu\n", key1, zframe_size((zframe_t *)zhash_lookup(single_object_hash, key1)));
           }
           zlist_purge(keys1);
           zlist_destroy(&keys1);
     }
     printf("\t ========================================\n");
     zlist_purge(keys);
     zlist_destroy(&keys);
     return 1;
}


uint32_t simple_hash(const void *buf, size_t buflength) {
     const uint8_t *buffer = (const uint8_t*)buf;

     uint32_t s1 = 1;
     uint32_t s2 = 0;

     for (size_t n = 0; n < buflength; n++) {
        s1 = (s1 + buffer[n]) % 65521;
        s2 = (s2 + s1) % 65521;
     }     
     return (s2 << 16) | s1;
}

