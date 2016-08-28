#include "rlnc_rs.h"
#include <stdlib.h>
#include <string.h>

#define RLNC

unsigned int decoded_correctly(char *a, char *b, int size) {
     int i;
    for(i=0; i < size; i++) {
        if( a[i]!=b[i] ) { 
          printf("ERROR : %d\n",i);
          return 0;
        }
    }
    return 1;
}

void destroy_encoded_data(ENCODED_DATA encoded_data_info){
    int i;
    for(i=0; i < encoded_data_info.N; i++) {
        free(encoded_data_info.encoded_raw_data[i]);
    }
    free(encoded_data_info.encoded_raw_data);
}


int main() {


    int data = 10000000;    
    int size=0;
    int i =0;
    char buf[20];
    for( i=0; i < data; i++ ) {
        sprintf(buf, " %d", i);
        size += strlen(buf);
    }
    size += 2;

    char *a = (char *)malloc((size)*sizeof(char));
    char * p = a;
    *p ='<';
    p++;

    for( i=0; i < data; i++ ) {
        sprintf(buf, " %d", i);
        sprintf(p, " %d", i);
        p += strlen(buf);
    }
    *p ='>'; p++; *p='\0';

    int K = 40; //K
    int N = 55;
    int symbol_size = 1024;
    int j;
    char *decoded;

   // printf("UNENCODED DATA : %s\n",a);
    printf("UNENCODED DATA SIZE: %d\n",(int)strlen(a));
    printf("UNENCODED DATA SIZE: %d\n",size);
    ENCODED_DATA encoded_data_info;

    for(j=0; j < 4; j++) {
#ifdef RS
    printf("CHECKING REED-SOLOMON CODE  \n");
//    printf("UNCODED:%s\n",a);
    
    encoded_data_info = encode(N, K, symbol_size, a, strlen(a), reed_solomon) ;
    decoded = (char *)decode(N, K, K, symbol_size, encoded_data_info, reed_solomon);
    destroy_encoded_data(encoded_data_info);

    printf("DECODED DATA SIZE: %d\n",(int)strlen(decoded));

    if( decoded_correctly(decoded, a, size)) {
       printf("REED-SOLOMON encoder/decoder worked correctly for %d bytes. CONGRATULATIONS!!\n",encoded_data_info.actual_data_size); 
    }
    else {
       printf("REED-SOLOMON encoder/decoder failed!\n") ;
    }
//    printf("DECODED:%s\n",decoded);
    free(decoded);

    printf("\n");
#endif

#ifdef RLNC
    printf("CHECKING RLNC CODE \n");
    encoded_data_info = encode(N, K, symbol_size, a, strlen(a), full_vector) ;
    decoded = (char *)decode(N, K, K+ 2,  symbol_size, encoded_data_info, full_vector);
    destroy_encoded_data(encoded_data_info);

    if( decoded_correctly(decoded, a, size)) {
       printf("RLNC encoder/decoder worked correctly for %d bytes. CONGRATULATIONS!!\n",encoded_data_info.actual_data_size); 
    }
    else {
       printf("RLNC encoder/decoder failed!\n") ;
    }
    free(decoded);

   // printf("DECODED DATA : %s\n",decoded);
#endif
}



    return 0;

}
