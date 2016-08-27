// Copyright Steinwurf ApS 2016.
// Distributed under the "STEINWURF RESEARCH LICENSE 1.0".
// See accompanying file LICENSE.rst or
// http://www.steinwurf.com/licensing

/// @example reed_solomon.cpp
///
/// Simple example showing how to encode and decode a block
/// of memory using a Reed-Solomon codec.

#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <iostream>
#include <set>
#include <vector>
#include <string.h>
#include <cmath>

#include <kodocpp/kodocpp.hpp>

#include "rlnc_rs.h"

// Prints out the elements of a vector to the
// screen for debugging purposes
// vector<uint8_t> vector the vector to be printed
void printVectorBytes(std::vector<uint8_t> vector){
    for (std::vector<uint8_t>::const_iterator i = vector.begin(); i != vector.end(); ++i){
        printf("%.2x ", unsigned(*i));
    }
    std::cout << std::endl;
} 

void printVectorChars(std::vector<uint8_t> vector){
    for (std::vector<uint8_t>::const_iterator i = vector.begin(); i != vector.end(); ++i){
        printf("%c", *i);
    }
    std::cout << std::endl;
} 

void printVectorCharsN(std::vector<uint8_t> vector, int N){
    int j=0;
    for (std::vector<uint8_t>::const_iterator i = vector.begin(); i != vector.end(); ++i){
      if( j++ >= N )
        printf("%c", *i);
    }
    std::cout << std::endl;
} 


#ifdef ASLIBRARY

ENCODED_DATA encode(uint32_t N, uint32_t max_symbols, 
         uint32_t max_symbol_size, char *data_in_char, 
          int data_size, enum CodingAlgorithm algo ) {

    kodocpp::encoder_factory *encoder_factory;
 
    if( algo==full_vector ) {
        encoder_factory = new  kodocpp::encoder_factory(
        kodocpp::codec::full_vector,
        kodocpp::field::binary8,
        max_symbols,
        max_symbol_size);
    }

    if( algo==reed_solomon ) {
        encoder_factory = new kodocpp::encoder_factory(
        kodocpp::codec::reed_solomon,
        kodocpp::field::binary8,
        max_symbols,
        max_symbol_size);
    }

    kodocpp::encoder encoder = encoder_factory->build();

    std::cout << "Payload size : " << encoder.payload_size() << std::endl;
    std::vector<uint8_t> payload(encoder.payload_size());
    // Allocate input and output data buffers
    std::vector<uint8_t> data_in(encoder.block_size());

    char *data = data_in_char; 
    uint32_t data_encoded=0;
    uint32_t block_size = max_symbols*max_symbol_size;

    uint32_t total_num_blocks = ceil(static_cast<float>(data_size)/static_cast<float>(block_size));

    std::vector< std::vector< std::vector<uint8_t> > > *encoded_data = new std::vector< std::vector< std::vector<uint8_t> > >[N];
    std::vector< std::vector<uint8_t> > *striped_data = new std::vector< std::vector<uint8_t> >[N];
    

    int block_num = 0;
    int encoded_symbol_size;
    while( data_encoded < data_size ) {
          std::generate(data_in.begin(), data_in.end(), 
                [&data, &data_encoded,data_size] () ->uint8_t{ 
                      if(data_encoded < data_size) {
                          data_encoded++;
                          return *data++; 
                       }
                          data_encoded++;
                       return 32;
                    }

                 );

         encoder.set_const_symbols(data_in.data(), encoder.block_size());


         for(int i = 0; i < N; i ++ ) {
             std::vector<uint8_t> encoded_symbol; 


             uint32_t bytes_used = encoder.write_payload(payload.data());
            
             for (std::vector<uint8_t>::const_iterator it = payload.begin(); it != payload.end(); ++it){
                encoded_symbol.push_back(*it);
             }
             striped_data[i].push_back(encoded_symbol);
             encoded_symbol_size =  payload.size();
         }
         block_num++;
   }

   for(int i =0; i < N; i++) {
       encoded_data->push_back(striped_data[i]);
   }


    uint8_t *encoded_raw_data = (uint8_t *)malloc(block_num*encoded_symbol_size*N*sizeof(uint8_t));

    uint8_t *p = encoded_raw_data;

    int count=0;
    for(int b =0; b < block_num; b++) {
        for(int s =0; s < N; s++) {
            for(std::vector<uint8_t>::const_iterator it = (*encoded_data)[s][b].begin(); 
                    it != (*encoded_data)[s][b].end(); ++it){
                *p++ = *it;
                 count++;
            }
        }
    }

    std::cout << "done writing to buffer" << std::endl;

   ENCODED_DATA data_info; 

   data_info.encoded_symbol_size = encoded_symbol_size;
   //data_info.vdata = encoded_data;
   data_info.encoded_raw_data = encoded_raw_data;
   data_info.k = max_symbols;
   data_info.N = N;
   data_info.num_blocks = block_num;

   std::cout << "num_blocks " << block_num << std::endl;
   std::cout << "max_symbols " << max_symbols << std::endl;
   std::cout << "max_symbol_size " << max_symbol_size << std::endl;
   std::cout << "encoded max_symbol_size " << encoded_symbol_size << std::endl;
   std::cout << "Total Number of uncoded symbols " << max_symbols*max_symbol_size *block_num << std::endl;
   std::cout << "Total Number of coded symbols " << block_num*(max_symbol_size+6)*N << std::endl;
   std::cout << "Total Number of coded symbols " << block_num*encoded_symbol_size*N << std::endl;
   std::cout << "======================================\n";

   return data_info;

}


std::vector< std::vector < std::vector<uint8_t> > > *convert_from_C_to_vector(ENCODED_DATA encoded_data_info) {

     int N = encoded_data_info.N;
     int K = encoded_data_info.k;
     int encoded_symbol_size = encoded_data_info.encoded_symbol_size;
     int num_blocks = encoded_data_info.num_blocks;
     uint8_t *encoded_raw_data= encoded_data_info.encoded_raw_data;

     std::vector< std::vector< std::vector<uint8_t> > > *encoded_data = new std::vector< std::vector< std::vector<uint8_t> > >[N];
     std::vector< std::vector<uint8_t> > *striped_data = new std::vector< std::vector<uint8_t> >[N];
    
    uint8_t *p = encoded_raw_data;

    int c=0;
    for(int b =0; b < num_blocks; b++) {
        for(int s =0; s < N; s++) {
            std::vector<uint8_t> encoded_symbol; 
            for(int e =0; e < encoded_symbol_size; e++) {
                encoded_symbol.push_back(encoded_raw_data[c++]);
            }
            striped_data[s].push_back(encoded_symbol);
        }
    }
    for(int s =0; s < K; s++) {
       encoded_data->push_back(striped_data[s]);
    }

    return encoded_data;
}



uint8_t *decode(uint32_t N, uint32_t max_symbols, uint32_t max_symbol_size, 
         ENCODED_DATA encoded_data_info, enum CodingAlgorithm algo) {

    kodocpp::decoder_factory *decoder_factory;
    if( algo==full_vector ) {
        decoder_factory= new kodocpp::decoder_factory(
        kodocpp::codec::full_vector,
        kodocpp::field::binary8,
        max_symbols,
        max_symbol_size);
    }

    if( algo==reed_solomon ) {
        decoder_factory = new kodocpp::decoder_factory(
        kodocpp::codec::reed_solomon,
        kodocpp::field::binary8,
        max_symbols,
        max_symbol_size);
    }

   //std::vector< std::vector < std::vector<uint8_t> > > *encoded_data  = encoded_data_info.vdata;
   std::vector< std::vector < std::vector<uint8_t> > > *encoded_data  
       = convert_from_C_to_vector(encoded_data_info);
     
    std::vector< uint8_t *> decoded_symbols;

    int num_blocks  = encoded_data_info.num_blocks;
    int data_size =0;


    for(int i=0; i < num_blocks; i++) {

       kodocpp::decoder decoder = decoder_factory->build();
       std::vector<uint8_t> data_out(decoder.block_size());
       decoder.set_mutable_symbols(data_out.data(), decoder.block_size());

     //  std::cout << "=========== " << std::endl;
       for(int j=0; j < max_symbols; j++) {
         decoder.read_payload( (*encoded_data)[j][i].data());
       }

       uint8_t *decoded_symbol =  new uint8_t[max_symbol_size*max_symbols + 1];
  
       if( decoder.is_complete() ) {
         memcpy(decoded_symbol, data_out.data(), data_out.size());
       }
      decoded_symbol[max_symbol_size*max_symbols + 1]='\0';
      data_size += data_out.size();
      decoded_symbols.push_back(decoded_symbol);
    }

    uint8_t *decoded_data = (uint8_t *)malloc( sizeof(uint8_t)*(data_size + 1) ); 
    uint8_t *p = decoded_data;
    for(int i=0; i < decoded_symbols.size(); i++) {
        memcpy(p, decoded_symbols[i], strlen((char *)decoded_symbols[i]));
        p = p + strlen((char *)decoded_symbols[i]);
        free( decoded_symbols[i]);
   }
   decoded_data[data_size] = '\0';
   decoded_symbols.clear();
   return decoded_data;
}
#endif

#ifdef ASMAIN
int main() {

    char a[] = " Tip: When you define a named closure, the compiler generates a corresponding function class for it. Every time you call the lambda through its named variable, the compiler instantiates a closure object at the place of call. Therefore, named closures are useful for reusable functionality (factorial, absolute value, etc.), whereas unnamed lambdas are more suitable for inline ad-hoc computations. Unquestionably, the rising popularity of functional programming will make lambdas widely-used in new C++ projects. It’s true that lambdas don’t offer anything you haven’t been able to do before with function objects. However, lambdas are more convenient than function objects because the tedium of writing boilerplate code for every function class (a constructor, data members and an overloaded operator() among the rest) is relegated to compiler. Additionally, lambdas tend to be more efficient because the compiler is able to optimize them more aggressively than it would a user-declared function or class. Finally, lambdas provide a higher level of security because they let you localize (or even hide) functionality from other clients and modules.";

   std::cout << "ENCODED :" << a << std::endl;
   ENCODED_DATA encoded_data_info = 
         encode(15, 10, 30,  a, strlen(a)) ;

   //std::cout << "Total Num coded symbols " << encoded_data_info.vdata->size() << std::endl;
  char *decoded = (char *)decode(15, 10, 30,  encoded_data_info);
  std::cout << "DECODED :" << decoded << std::endl;
 //  std::cout << "Total Num coded symbols " << encoded_data->size() << std::endl;
}
#endif
