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
#include "sodaw_client.h"
#include <base64.h>
#include <rlnc_rs.h>

#define DEBUG_MODE 1

extern int s_interrupted;

#ifdef ASLIBRARY

Tag *SODAW_read_get_phase(
		char *obj_name, 
		unsigned int op_num, 
		zsock_t *sock_to_servers,  
		char **servers, 
		unsigned int num_servers, 
		char *port
)
{

	return SODAW_write_get_or_read_get_phase(
			obj_name, READ_GET, op_num, 
			sock_to_servers,  servers, 
			num_servers, port);
}


// this fetches the max tag and value
char *SODAW_read_value(
		char *obj_name, 
		unsigned int op_num, 
		zsock_t *sock_to_servers,  
		char **servers, 
		unsigned int num_servers, 
		char *port, 
		Tag read_tag
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

				void *payload_frame = zhash_lookup(frames, PAYLOAD);


				zlist_t *coded_elements = (zlist_t *)zhash_lookup(received_data, tag_str);
				assert(coded_elements!=NULL);

				zlist_append(coded_elements, payload_frame); 

				char *decodeableKey = number_responses_at_least(received_data, majority);

				if(decodeableKey!= NULL) break;
			}
			else {
				printf("\t\tOLD MESSAGES : (%s, %d)\n", phase, op_num);

			}
			zmsg_destroy (&msg);
			zlist_purge(names);
			destroy_frames(frames);
		}
	}

	return value;
}


// this is the write tag value phase of SODAW
void SODAW_read_complete_phase(  
		char *obj_name,
		char *reader_id, 
		zsock_t *sock_to_servers,  
		char **servers, 
		unsigned int num_servers, 
		char *port, 
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
		char *reader_id, 
		unsigned int op_num ,
		char *servers_str, 
		char *port
		)
{
	s_catch_signals();
	int j;
	int num_servers = count_num_servers(servers_str);
  printf("helloo\n");
	char **servers = create_server_names(servers_str);
  printf("helloooooo\n");

#ifdef DEBUG_MODE
	printf("\t\tObj name       : %s\n",obj_name);
	printf("\t\tWriter name    : %s\n",reader_id);
	printf("\t\tOperation num  : %d\n",op_num);

	/* char *myb64 = (char *)malloc(strlen(payload));
		 b64_decode(payload, myb64);
		 printf("Encoded string  : %s\n", payload);
		 free(myb64);
		 */
	printf("\t\tServer string   : %s\n", servers_str);
	printf("\t\tPort to Use     : %s\n", port);
	printf("\t\tNum of Servers  : %d\n",num_servers);

	//    printf("Decoded string  : %s\n", myb64);
	for(j=0; j < num_servers; j++) {
		printf("\t\t\tServer : %s\n", servers[j]);
	}
	printf("\n");
#endif


	zctx_t *ctx  = zctx_new();
	void *sock_to_servers = zsocket_new(ctx, ZMQ_DEALER);
	zctx_set_linger(ctx, 0);
	assert(sock_to_servers);

	zsocket_set_identity(sock_to_servers,  reader_id);

	for(j=0; j < num_servers; j++) {    
		char *destination = create_destination(servers[j], port);
		int rc = zsocket_connect(sock_to_servers, destination);
		assert(rc==0);
		free(destination);
	}

	printf("READ %d\n", op_num);
	printf("\tREAD_GET (READER)\n");

	Tag *read_tag=  SODAW_read_get_phase(
			obj_name,  
			op_num, 
			sock_to_servers, 
			servers, 
			num_servers, 
			port
			);

	printf("\t\tmax tag (%d,%s)\n\n", read_tag->z, read_tag->id);


	printf("\tREAD_VALUE (READER)\n");


	char *payload = SODAW_read_value(
			obj_name, 
			op_num, 
			sock_to_servers,  
			servers, 
			num_servers, 
			port, 
			*read_tag 
			);


	printf("\tREAD_COMPLETE (READER)\n");
	SODAW_read_complete_phase(
			obj_name, 
			reader_id,
			sock_to_servers,  
			servers, 
			num_servers, 
			port, 
			*read_tag 
			);


	free(read_tag);
	zsocket_destroy(ctx, sock_to_servers);
	zctx_destroy(&ctx);

	destroy_server_names(servers, num_servers);

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
