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
typedef std::vector< std::vector< std::vector<uint8_t> > >   VECTOR_VECTOR_VECTOR_UINT8;
typedef std::vector< std::vector<uint8_t> >                  VECTOR_VECTOR_UINT8;
typedef std::vector<uint8_t>                              VECTOR_UINT8;

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

    std::vector<uint8_t> payload(encoder.payload_size());
    // Allocate input and output data buffers
    std::vector<uint8_t> data_in(encoder.block_size());

    char *data = data_in_char; 
    uint32_t block_size = max_symbols*max_symbol_size;

    uint32_t total_num_blocks = ceil(static_cast<float>(data_size)/static_cast<float>(block_size));

    std::vector< std::vector< std::vector<uint8_t> > > *encoded_data = new std::vector< std::vector< std::vector<uint8_t> > >[N];
    std::vector< std::vector<uint8_t> > *striped_data = new std::vector< std::vector<uint8_t> >[N];
    

    int block_num = 0;
    uint32_t data_encoded=0;
    int encoded_symbol_size;
    while( data_encoded < data_size ) {
          // prepare a block into input buffer
          uint32_t data_encoded_in_block=0;
          std::generate(data_in.begin(), data_in.end(), 
                [&data, &data_encoded,data_size, &data_encoded_in_block] () ->uint8_t{ 
                      if(data_encoded < data_size) {
                          data_encoded++;
                          data_encoded_in_block++;
                          return *data++; 
                       }
                       data_encoded++;
                       return 32;
                    }

                 );

          // pass input into the encode
         encoder.set_const_symbols(data_in.data(), encoder.block_size());


          // genereate N encoded symbols
         for(int i = 0; i < N; i ++ ) {
             uint32_t bytes_used = encoder.write_payload(payload.data());
            
              
              // write out the encoded symbol
             std::vector<uint8_t> encoded_symbol; 
             for (std::vector<uint8_t>::const_iterator it = payload.begin(); it != payload.end(); ++it){
                encoded_symbol.push_back(*it);
             }
             // store it in a stripe
             striped_data[i].push_back(encoded_symbol);
            //size of an encoded symbol
/*
             std::cout << "data encoded " << data_encoded_in_block << " (" << max_symbol_size  << ", "
                        << encoded_symbol.size()
                        << ") " << max_symbols  << std::endl;
             std::cout << "encoded symbol size " << encoded_symbol.size() << std::endl;
*/
             encoded_symbol_size =  encoded_symbol.size();
         }
         block_num++;
   }

    for(int i =0; i < N; i++) {
       encoded_data->push_back(striped_data[i]);
    }

    //uint8_t *encoded_raw_data = (uint8_t *)malloc(block_num*encoded_symbol_size*N*sizeof(uint8_t));
    uint8_t **encoded_raw_data = (uint8_t **)malloc(N*sizeof(uint8_t *));
    for(int i =0; i < N; i ++) {
        encoded_raw_data[i] = (uint8_t *)malloc(block_num*encoded_symbol_size*sizeof(uint8_t));
    }

    uint8_t **p =(uint8_t **)malloc( N*sizeof(uint8_t *));
    for(int i =0; i < N; i ++) {
      p[i] = encoded_raw_data[i];
    }

    uint8_t *q;
    int count=0;
    for(int b =0; b < block_num; b++) {
        for(int s =0; s < N; s++) {
            q =  p[s];
            for(std::vector<uint8_t>::const_iterator it = (*encoded_data)[s][b].begin(); 
                    it != (*encoded_data)[s][b].end(); ++it){
                *q++ = *it;
                 count++;
            }
            p[s] = q;
        }
    }

   ENCODED_DATA data_info; 

   data_info.encoded_symbol_size = encoded_symbol_size;
   data_info.symbol_size = max_symbol_size;
   data_info.encoded_raw_data = encoded_raw_data;
   data_info.K = max_symbols;
   data_info.N = N;
   data_info.num_blocks = block_num;
   data_info.actual_data_size = data_size;
   data_info.padded_data_size = data_encoded - data_size;
   data_info.total_data_size = data_encoded;


#ifdef DEBUG_MODE
   if(algo==full_vector)
   std::cout << "\t\t===========  RLNC ===========================\n";
   else
   std::cout << "\t\t========== REED-SOLOMON ======================\n";

   std::cout << "\t\tnum_blocks                           " << data_info.num_blocks << std::endl;
   std::cout << "\t\t#bytes per block                     " << data_info.K*data_info.symbol_size << std::endl;
   std::cout << "\t\t#symbols per block (K)               " << data_info.K << std::endl;
   std::cout << "\t\t#encoded stripes (N)                 " << data_info.N << std::endl;
   std::cout << "\t\t#symbols per stripe                  " << data_info.num_blocks << std::endl;
   std::cout << std::endl;
   std::cout << "\t\tsymbol size                          " << data_info.symbol_size << std::endl;
   std::cout << "\t\tcoded symbol size                    " << data_info.encoded_symbol_size << std::endl;
   std::cout << std::endl;
   std::cout << "\t\tTotal bytes in of uncoded symbols    " << 
         data_info.K*data_info.symbol_size*data_info.num_blocks << std::endl;
   std::cout << "\t\tTotal bytes in of coded symbols      " << 
         data_info.N*data_info.encoded_symbol_size*data_info.num_blocks << std::endl;

   std::cout << std::endl;
   std::cout << "\t\tTotal Number of uncoded symbols      " << data_info.num_blocks*data_info.K << std::endl;
   std::cout << "\t\tTotal Number of coded symbols        " << data_info.num_blocks*data_info.N << std::endl;
   std::cout << std::endl;
   std::cout << "\t\tRaw data size                        " << data_info.actual_data_size << std::endl;
   std::cout << "\t\tTotal data size                      " << data_info.total_data_size << std::endl;
   std::cout << "\t\tPadded data size                     " << data_info.padded_data_size << std::endl;
   if(algo==full_vector)
   std::cout << "\t\t===========  RLNC ===========================\n";
   else
   std::cout << "\t\t========== REED-SOLOMON ======================\n";
#endif
  
   delete [] striped_data;
   delete [] encoded_data;

   delete encoder_factory;

   return data_info;

}


std::vector< std::vector < std::vector<uint8_t> > > *convert_from_C_to_vector(ENCODED_DATA encoded_data_info) {

     int N = encoded_data_info.N;
     int K = encoded_data_info.K;
     int encoded_symbol_size = encoded_data_info.encoded_symbol_size;
     int num_blocks = encoded_data_info.num_blocks;
     uint8_t **encoded_raw_data= encoded_data_info.encoded_raw_data;

     std::vector< std::vector< std::vector<uint8_t> > > *encoded_data = new std::vector< std::vector< std::vector<uint8_t> > >[N];
     std::vector< std::vector<uint8_t> > *striped_data = new std::vector< std::vector<uint8_t> >[N];
    
    int c=0;
    for(int b =0; b < num_blocks; b++) {
        for(int s =0; s < K; s++) {
            std::vector<uint8_t> encoded_symbol; 
            c=b*encoded_symbol_size;
            for(int e =0; e < encoded_symbol_size; e++) {
                encoded_symbol.push_back(encoded_raw_data[s][c++]);
            }
            striped_data[s].push_back(encoded_symbol);
        }
    }

    for(int s =0; s < N; s++) {
       encoded_data->push_back(striped_data[s]);
    }

    delete [] striped_data;
    return encoded_data;
}



uint8_t *decode(uint32_t N, uint32_t max_symbols,uint32_t M,  uint32_t max_symbol_size, 
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
       //setting up the decoder
       kodocpp::decoder decoder = decoder_factory->build();
       std::vector<uint8_t> data_out(decoder.block_size());
       decoder.set_mutable_symbols(data_out.data(), decoder.block_size());

     //  std::cout << "=========== " << std::endl;
       for(int j=0; j < M; j++) {
         decoder.read_payload( (*encoded_data)[j][i].data());
       }

       uint8_t *decoded_symbol =  new uint8_t[max_symbol_size*max_symbols + 1];
  
       if( decoder.is_complete() ) {
         memcpy(decoded_symbol, data_out.data(), data_out.size());
       }
      decoded_symbol[max_symbol_size*max_symbols]='\0';
      data_size += data_out.size();
      decoded_symbols.push_back(decoded_symbol);

    }
   
    delete [] encoded_data; 
    delete decoder_factory;

    uint8_t *decoded_data = (uint8_t *)malloc( sizeof(uint8_t)*(data_size + 1) ); 

    uint8_t *p = decoded_data;
    for(int i=0; i < decoded_symbols.size(); i++) {
        memcpy(p, decoded_symbols[i], strlen((char *)decoded_symbols[i]));
        p = p + strlen((char *)decoded_symbols[i]);
        delete decoded_symbols[i];
   }
   decoded_data[encoded_data_info.actual_data_size] = '\0';
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
