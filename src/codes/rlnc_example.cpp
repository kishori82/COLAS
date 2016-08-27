// Copyright Steinwurf ApS 2016.
// Distributed under the "STEINWURF RESEARCH LICENSE 1.0".
// See accompanying file LICENSE.rst or
// http://www.steinwurf.com/licensing

#include <cstdint>
#include <algorithm>
#include <iostream>
#include <vector>

#include <kodocpp/kodocpp.hpp>
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


int main()
{
    //! [0]
    // Set the number of symbols (i.e. the generation size in RLNC
    // terminology) and the size of a symbol in bytes
    uint32_t max_symbols = 6;
    uint32_t max_symbol_size = 10;
    //! [1]
    // In the following we will make an encoder/decoder factory.
    // The factories are used to build actual encoders/decoders
    kodocpp::encoder_factory encoder_factory(
        kodocpp::codec::full_vector,
        kodocpp::field::binary8,
        max_symbols,
        max_symbol_size);

    kodocpp::encoder encoder = encoder_factory.build();

    kodocpp::decoder_factory decoder_factory(
        kodocpp::codec::full_vector,
        kodocpp::field::binary8,
        max_symbols,
        max_symbol_size);

    //! [2]
    std::vector<uint8_t> payload(encoder.payload_size());
    std::vector<uint8_t> data_in(encoder.block_size());

    char a[] = "My name is Mary! I am a member of the local choir at the St Bishop's church";
    char *b;
    b = a;
    std::generate(data_in.begin(), data_in.end(), [&b]{ return *b++; });
    printVectorChars(data_in);
    kodocpp::decoder decoder = decoder_factory.build();

    //! [3]
    // Assign the data buffer to the encoder so that we may start
    // to produce encoded symbols from it
    encoder.set_const_symbols(data_in.data(), encoder.block_size());
    //! [4]
    // Create a buffer which will contain the decoded data, and we assign
    // that buffer to the decoder
    std::vector<uint8_t> data_out(decoder.block_size());
    decoder.set_mutable_symbols(data_out.data(), decoder.block_size());
    //! [5]
    bool DEBUG_MODE = true;
    uint32_t lost_payloads = 0;
    uint32_t encoded_count = 0;

    while (!decoder.is_complete())
    {
        // Encode a packet into the payload buffer
        uint32_t bytes_used = encoder.write_payload(payload.data());
        std::cout << "Bytes used = " << bytes_used << std::endl;

        ++encoded_count;

        if(DEBUG_MODE){
            //    printf("%.2d: ", j);
                printVectorBytes(payload);
                printVectorChars(payload);
                printVectorCharsN(payload, 7);
         if ( (rand() % 2) == 0)
        {
            lost_payloads++;
            continue;
        }

       }
 
        // Pass that packet to the decoder
        decoder.read_payload(payload.data());
    }

    std::cout << "Number of lost payloads: " << lost_payloads << std::endl;
    std::cout << "Encoded count = " << encoded_count << std::endl;
    //! [6]
    // Check if we properly decoded the data
    if (data_in == data_out)
    {
        printVectorChars(data_in);
        std::cout << "Data decoded correctly" << std::endl;
        printVectorChars(data_out);
    }
    //! [7]
    return 0;
}
