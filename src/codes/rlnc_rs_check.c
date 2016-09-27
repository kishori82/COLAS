#include "rlnc_rs.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define REED_SOLOMON
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

void destroy_EncodedData(EncodeData *encoded_data_info) {
    int i;

    for(i=0; i < encoded_data_info->N; i++) {
        free(encoded_data_info->encoded_data[i]);
    }

    free(encoded_data_info->raw_data);
    free(encoded_data_info->decoded_data);
}


uint8_t *create_random_data(int size) {
    int i;
    uint8_t *a = (uint8_t *)malloc((size)*sizeof(uint8_t));
    uint8_t * p = a;
    for( i=0; i < size; i++ ) {
        *p  = (uint8_t)rand()/256;
        p++;
    }
    return a;
}


int main() {


    int data_size = 1000000;
    int size=0;
    int i =0;
    char buf[20];
    srand(time(NULL));

    /*
        *p ='<';
        p++;

        for( i=0; i < data_size; i++ ) {
            sprintf(buf, " %d", i);
            sprintf(p, " %d", i);
            p += strlen(buf);
        }

        *p ='>'; p++; *p='\0';
    */

    int K = 40; //K
    int N = 55;
    int symbol_size = 1024;
    int j;

    // printf("UNENCODED DATA : %s\n",a);
    printf("UNENCODED DATA SIZE: %d\n",data_size);
    EncodeData encoded_data_info;

    for(j=0; j < 200; j++) {
        uint8_t *a;
        encoded_data_info.N = N;
        encoded_data_info.symbol_size = symbol_size;
        encoded_data_info.raw_data_size = data_size;
        encoded_data_info.offset_index = 6;

        encoded_data_info.fieldsize=2;
#ifdef REED_SOLOMON
        encoded_data_info.K = K;
        printf("CHECKING REED-SOLOMON CODE  \n");
//    printf("UNCODED:%s\n",a);
        a =  create_random_data(data_size);
        encoded_data_info.raw_data = a;
        encoded_data_info.algorithm = reed_solomon;

        //encode(N, K, symbol_size, a, strlen(a), reed_solomon) ;
        printf("SET PARAMSENCODED DATA : %d\n",data_size);
        encode(&encoded_data_info) ;

        printf("ENCODED DATA : %d\n",data_size);
        //decode(N, K, K, symbol_size, encoded_data_info, reed_solomon);
        if(decode(&encoded_data_info)==0) {
            perror("Failed to decode\n");
        }

        if( decoded_correctly(encoded_data_info.decoded_data, encoded_data_info.raw_data, encoded_data_info.raw_data_size)) {
            printf("REED-SOLOMON encoder/decoder worked correctly for %d bytes. CONGRATULATIONS!!\n",encoded_data_info.raw_data_size);
        } else {
            printf("REED-SOLOMON encoder/decoder failed!\n") ;
        }
        destroy_EncodedData(&encoded_data_info);
//    printf("DECODED:%s\n",decoded);

        printf("\n");
#endif

#ifdef RLNC


        a =  create_random_data(data_size);
        encoded_data_info.raw_data = a;


        encoded_data_info.K =K+8;
        encoded_data_info.algorithm = full_vector;

        printf("CHECKING RLNC CODE \n");
        encode(&encoded_data_info) ;

        if( decode(&encoded_data_info)==0) {
            perror("Failed to decode\n");
            continue;
        }


        if( decoded_correctly(encoded_data_info.decoded_data, encoded_data_info.raw_data, encoded_data_info.raw_data_size)) {
            printf("RLNC encoder/decoder worked correctly for %d bytes. CONGRATULATIONS!!\n",encoded_data_info.raw_data_size);
        } else {
            printf("RLNC encoder/decoder failed!\n") ;
        }

        destroy_EncodedData(&encoded_data_info);
        // printf("DECODED DATA : %s\n",decoded);
#endif
    }



    return 0;

}
