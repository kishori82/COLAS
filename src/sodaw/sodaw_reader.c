//  Asynchronous client-to-server (DEALER to ROUTER)
//
//  While this example runs in a single process, that is to make
//  it easier to start and stop the example. Each task has its own
//  context and conceptually acts as a separate process.

#include "czmq.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <algo_utils.h>
#include "sodaw_client.h"
#include "client.h"
#include "sodaw_reader.h"
#include <base64.h>
#include <rlnc_rs.h>

#define DEBUG_MODE 1

extern int s_interrupted;

#ifdef ASLIBRARY

Tag *SODAW_read_get_phase(
		char *obj_name, 
		unsigned int op_num, 
		zsock_t *sock_to_servers,  
		unsigned int num_servers
)
{

	return SODAW_write_get_or_read_get_phase(
			obj_name, READ_GET, op_num, 
			sock_to_servers, num_servers);
}


// this fetches the max tag and value
char *SODAW_read_value(
		char *obj_name, 
		unsigned int op_num, 
		zsock_t *sock_to_servers,  
		unsigned int num_servers, 
		Tag read_tag, 
        EncodeData *encoding_info
		)
{

	// send out the messages to all servers

	char algorithm[100];
	char phase[100];
	char tag_str[100];
	char *value=NULL;
	unsigned int round;
	int size;

	zmq_pollitem_t items [] = { { sock_to_servers, 0, ZMQ_POLLIN, 0 } };

	char *types[] = {OBJECT, ALGORITHM, PHASE, OPNUM, TAG};
	tag_to_string(read_tag, tag_str); 
	send_multicast_servers(sock_to_servers, num_servers, types,  5, obj_name, SODAW, READ_VALUE, &op_num,   tag_str) ;

	unsigned int majority =  ceil(((float)num_servers+1)/2);
	unsigned int responses =0;
	zlist_t *tag_list = zlist_new();

	zhash_t *received_data= zhash_new();
    char *decodeableKey ;
	while (true) {
		//  Tick once per second, pulling in arriving messages

		// zmq_pollitem_t items [] = { { sock_to_servers, 0, ZMQ_POLLIN, 0 } };
		printf("\t\twaiting for data..\n");
		int rc = zmq_poll(items, 1, -1);
		if(rc < 0 ||  s_interrupted ) {
			printf("Interrupted!\n");
			exit(0);
		}
		if (items [0].revents & ZMQ_POLLIN) {
			zmsg_t *msg = zmsg_recv (sock_to_servers);

			zlist_t *names = zlist_new();
			zhash_t* frames = receive_message_frames_at_client(msg, names);

			get_string_frame(phase, frames, PHASE);
			get_string_frame(tag_str, frames, TAG);
			Tag tag;
			get_tag_frame(frames, &tag);

			if( zhash_lookup(received_data, tag_str)==NULL) {
				zlist_t *data_list = zlist_new();
				zhash_insert(received_data, tag_str,  (void *)data_list);
			}


			if(compare_tags(tag, read_tag) >=0 && strcmp(phase, READ_VALUE)==0) {
				if(DEBUG_MODE) print_out_hash_in_order(frames, names);

				zframe_t *payload_frame = (zframe_t *)zhash_lookup(frames, PAYLOAD);

				zlist_t *coded_elements = (zlist_t *)zhash_lookup(received_data, tag_str);
				assert(coded_elements!=NULL);

                zframe_t *dup_payload_frame =zframe_dup(payload_frame);
				zlist_append(coded_elements, dup_payload_frame); 

				decodeableKey = number_responses_at_least(received_data, majority);

				if(decodeableKey!= NULL) {
			       zmsg_destroy (&msg);
			       zlist_purge(names);
			       destroy_frames(frames);
                   break;
                }
			}
			else {
				printf("\t\tOLD MESSAGES : (%s, %d)\n", phase, op_num);

			}
			zmsg_destroy (&msg);
			zlist_purge(names);
			destroy_frames(frames);
		}
	}
    get_encoded_info(received_data, decodeableKey, encoding_info);

   
    printf("Key to decode %s\n", decodeableKey);

    if(decode(encoding_info)==0) {
       printf("Failed to decode for key %s\n", decodeableKey);
       exit(0);
    }

	return encoding_info->decoded_data;
}


int get_encoded_info(zhash_t *received_data, char *decodeableKey, EncodeData *encoding_info) {
    printf("N : %d\n", encoding_info->N); 
    printf("K : %d\n", encoding_info->K); 
    printf("Symbol size : %d\n", encoding_info->symbol_size);
    printf("actual datasize %d\n", encoding_info->raw_data_size);
    printf("num_blocks %d\n", encoding_info->num_blocks);


	zlist_t *coded_elements = (zlist_t *)zhash_lookup(received_data, decodeableKey);
    
    zframe_t *data_frame;
    int frame_size=0, cum_size=0;;
    for(data_frame= (zframe_t *) zlist_first(coded_elements); data_frame!=NULL; data_frame=zlist_next(coded_elements)) {
    //    printf("Length of data %lu\n", zframe_size(data_frame));
        frame_size = zframe_size(data_frame); 
        cum_size += frame_size; 
    } 

    int i;
    encoding_info->encoded_data = (uint8_t **)malloc( encoding_info->K *sizeof(uint8_t*));
    for(i =0; i < encoding_info->K; i++) {
         encoding_info->encoded_data[i] = (uint8_t *)malloc(frame_size*sizeof(uint8_t));
    }

    i=0;
    for(data_frame= (zframe_t *) zlist_first(coded_elements); data_frame!=NULL; data_frame=zlist_next(coded_elements)) {
        frame_size = zframe_size(data_frame); 
        memcpy(encoding_info->encoded_data[i++], zframe_data(data_frame), frame_size);
    }


    printf("encoded symbol size %d\n", cum_size/(encoding_info->num_blocks*encoding_info->K));
    encoding_info->encoded_symbol_size = ceil(cum_size/(encoding_info->num_blocks*encoding_info->K));

}

// this is the write tag value phase of SODAW
void SODAW_read_complete_phase(  
		char *obj_name,
		char *reader_id, 
		zsock_t *sock_to_servers,  
		unsigned int num_servers, 
		Tag max_tag   // for read it is max and for write it is new
		)
{
	// send out the messages to all servers
	char tag_str[100];

	char *types[] = {OBJECT, ALGORITHM, PHASE, TAG};
	tag_to_string(max_tag, tag_str); 
	send_multicast_servers(sock_to_servers, num_servers, types,  4, obj_name, SODAW, READ_COMPLETE, tag_str) ;


}


char *SODAW_read(
		char *obj_name,
		unsigned int op_num ,
        EncodeData *encoded_data,
        ClientArgs *client_args
		)
{
	s_catch_signals();
	int j;

	int num_servers = count_num_servers(client_args->servers_str);
    void *sock_to_servers= get_socket_servers(client_args);
#ifdef DEBUG_MODE
	printf("\t\tObj name       : %s\n",obj_name);
	printf("\t\tWriter name    : %s\n",client_args->client_id);
	printf("\t\tOperation num  : %d\n",op_num);

	/* char *myb64 = (char *)malloc(strlen(payload));
		 b64_decode(payload, myb64);
		 printf("Encoded string  : %s\n", payload);
		 free(myb64);
		 */
	printf("\t\tServer string   : %s\n", client_args->servers_str);
	printf("\t\tPort to Use     : %s\n", client_args->port);
	printf("\t\tNum of Servers  : %d\n",num_servers);

	//    printf("Decoded string  : %s\n", myb64);
/*
	for(j=0; j < num_servers; j++) {
		printf("\t\t\tServer : %s\n", servers[j]);
	}
	printf("\n");
*/
#endif

/*

	zctx_t *ctx  = zctx_new();
	void *sock_to_servers = zsocket_new(ctx, ZMQ_DEALER);
	zctx_set_linger(ctx, 0);
	assert(sock_to_servers);

	zsocket_set_identity(sock_to_servers,  client_args->client_id);

	for(j=0; j < num_servers; j++) {    
		char *destination = create_destination(servers[j], client_args->port);
		int rc = zsocket_connect(sock_to_servers, destination);
		assert(rc==0);
		free(destination);
	}
*/

	printf("READ %d\n", op_num);
	printf("\tREAD_GET (READER)\n");

	Tag *read_tag=  SODAW_read_get_phase(
			obj_name,  
			op_num, 
			sock_to_servers, 
			num_servers
			);

	printf("\t\tmax tag (%d,%s)\n\n", read_tag->z, read_tag->id);


	printf("\tREAD_VALUE (READER)\n");


	char *payload = SODAW_read_value(
			obj_name, 
			op_num, 
			sock_to_servers,  
			num_servers, 
			*read_tag, 
            encoded_data 
			);


	printf("\tREAD_COMPLETE (READER)\n");
	SODAW_read_complete_phase(
			obj_name, 
			client_args->client_id,
			sock_to_servers,  
			num_servers, 
			*read_tag 
			);

	free(read_tag);


/*
	zsocket_destroy(ctx, sock_to_servers);
	zctx_destroy(&ctx);
	destroy_server_names(servers, num_servers);
*/

	return payload;
}



#endif


//  The main thread simply starts several clients and a server, and then
//  waits for the server to finish.
//#define ASMAIN

#ifdef ASMAIN

int main (void)
{
	int i ; 

	char *payload = (char *)malloc(100000000*sizeof(char));
	unsigned int size = 100000000*sizeof(char);

	/*
		 char *servers[]= {
		 "172.17.0.7", "172.17.0.5", 
		 "172.17.0.4", "172.17.0.6",
		 "172.17.0.3"
		 };

*/

	/*
		 char *servers[] = {
		 "172.17.0.22", "172.17.0.21", "172.17.0.18", "172.17.0.17", "172.17.0.20", "172.17.0.16", "172.17.0.19", "172.17.0.15", "172.17.0.14", "172.17.0.13", "172.17.0.12", "172.17.0.11", "172.17.0.10", "172.17.0.9", "172.17.0.7", "172.17.0.8", "172.17.0.6", "172.17.0.5", "172.17.0.4", "172.17.0.3"
		 };
		 */

	char *servers[]= {
		"172.17.0.2"
	};


	unsigned int num_servers = 1;
	char port[]= {"8081"};

	char writer_id[] = { "writer_1"};
	char obj_name[] = {OBJECT};

	unsigned int op_num;
	s_catch_signals();

	for( i=0; i < 5; i++) {
		printf("\nWRITE %d\n", i);
		//SODAW_write(obj_name, writer_id, i,  payload, size, servers, port);
	}

	//   zclock_sleep(50*1000); 
	return 0;
}




#endif
