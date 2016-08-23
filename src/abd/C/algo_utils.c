#include "algo_utils.h"

void _zframe_int(zframe_t *f, int *i) {
    byte *data = zframe_data(f);
    *i = *((int *) data);
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

unsigned int count_num_servers(char *servers_str) {
    int count = 0;
    if( servers_str==NULL) return 0;
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



char *create_destination(char *server, char *port) {
      int size = 0;
      size += strlen(server);
      size += strlen(port);
      
      char *dest = (char *)malloc( (size + 2)*sizeof(char));
      assert(dest!=0);
      sprintf(dest, "%s:%s", server, port);
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

