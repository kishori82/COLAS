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

typedef struct _ENCODED_DATA {
     uint8_t **encoded_raw_data;
    // std::vector< std::vector< std::vector<uint8_t> > >  *vdata;
     int encoded_symbol_size;
     int num_blocks;
     int symbol_size;
     int actual_data_size;
     int padded_data_size;
     int total_data_size;
     int K, N;
     
} ENCODED_DATA;


ENCODED_DATA encode(uint32_t N, uint32_t max_symbols, uint32_t max_symbol_size,  
              char *data_in_char, int data_size, enum CodingAlgorithm) ;


uint8_t *decode(uint32_t N, uint32_t max_symbols, uint32_t M,  uint32_t max_symbol_size, 
        ENCODED_DATA encoded_data_info, enum CodingAlgorithm) ;

#ifdef __cplusplus
   }
#endif


#endif
