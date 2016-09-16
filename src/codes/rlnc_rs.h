#ifndef _RLNC_RS_
#define _RLNC_RS_

#include <stdint.h>
#include <string.h>
#include <stdio.h>


enum CodingAlgorithm {
   full_vector = 0,
   reed_solomon = 1
};


#ifdef __cplusplus
extern "C" {
#endif

#define TRUE 1
#define FALSE 0

typedef struct _ENCODED_DATA {
     uint8_t *decoded_data;
     uint8_t **encoded_data;
     uint8_t *raw_data;
    // std::vector< std::vector< std::vector<uint8_t> > >  *vdata;
     int encoded_symbol_size;
     unsigned int offset_index;
     int num_blocks;
     int symbol_size;
     int raw_data_size;
     int padded_data_size;
     int total_data_size;
     unsigned int fieldsize;
     int N;
     int K; // max symbols
     enum CodingAlgorithm algorithm;
} EncodeData;


unsigned short encode(EncodeData *encode_data) ;


unsigned short decode(EncodeData *encode_data) ;

#ifdef __cplusplus
   }
#endif


#endif
