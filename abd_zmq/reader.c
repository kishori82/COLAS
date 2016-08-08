#include "reader.h"
#include <math.h>
#include <zmq.h>
#include <string.h>

// TODO:
// The server needs to understand routing from the messages
// The server needs to send tags and such as well as parse them on this side
// If we close the ZMQ environment, what happens to the messages in flight or in the queue?
// How should we correctly error handle in zmq?
// How do we send a struct in ZMQ and parse it back into a struct on the other side?

// clusterSize: number of servers in the cluster, regardless or dead or alive
// servers: an array of "tcp://server-ip-address:8080"
int read(int clusterSize, char** servers) {

	// zmq preamble, setup connections to the servers
	// asynchronous requests (outgoing)
	void *ctx_dealer = zmq_ctx_new();
	void *dealer = zmq_socket(ctx_dealer, ZMQ_DEALER);
	int i;
	// setup binds to the servers
	for(i=0; i<clusterSize; i++) {
		zmq_connect(dealer, servers[i]);
	}
	// asynchronous replies (incoming)
	void *ctx_router = zmq_ctx_new();
	void *router = zmq_socket(ctx_router, ZMQ_ROUTER);
	zmq_connect(router, "tcp://*:8080");

	// Asynchronously ask the servers for their state variables
	for(i = 0; i<clusterSize; i++) {
		zmq_msg_t req;
		zmq_msg_init_size(&req, 5);
		zmq_msg_send((&req), dealer, 0);
		zmq_msg_close(&req);
	}

	// Wait on a completion of the quorum of servers to reply with their messages
	// Maintain the maximum tag
	double majority = ceil(( clusterSize+1) / 2 );
	Tag maxTag = {0, 0};
	zmq_pollitem_t reqs[] = { {dealer, 0, ZMQ_POLLIN, 0} };
	for(i = 0; i<majority; i++) {
		zmq_poll(reqs, 1, 10); // 10ms
		zmq_msg_t req;
		zmq_msg_init_size(&req, 5);
		zmq_msg_recv((&req), dealer, 0);

		Tag tag;
		memcpy( zmq_msg_data(&req), &tag, 100 ); //arbitrarily defined tag struct size of 100
		if(maxTag.id < tag.id) {
			maxTag = tag;
		}

		zmq_msg_close(&req);
	}

	// Asynchronously update the servers of the state variable
	// Hence send the tag and the value
	for(i = 0; i<clusterSize; i++) {
		zmq_msg_t req;
		zmq_msg_init_size(&req, sizeof(Tag));
		memcpy( zmq_msg_data(&req), &maxTag, sizeof(Tag)); 
		zmq_msg_send((&req), router, 0);
		zmq_msg_close(&req);
	}

	// Wait on a completion of the quorum of servers
	zmq_pollitem_t reps[] = { {router, 0, ZMQ_POLLIN, 0} };
	for(i = 0; i<majority; i++) {
		zmq_poll(reps, 1, 10); // 10ms
		zmq_msg_t rep;
		zmq_msg_init_size(&rep, 5);
		zmq_msg_send((&rep), router, 0);
		zmq_msg_close(&rep);
	}

	// zmq postamble, teardown connections to the servers
	zmq_close(router);
	zmq_ctx_destroy(ctx_router);
	zmq_close(dealer);
	zmq_ctx_destroy(ctx_dealer);
}
