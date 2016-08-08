/*
	Code to run on each of the server instances
*/
#include "server.h"
#include <zmq.h>

// Does a REP socket HAVE to reply to a REQ?
// So is an "ack" essentially always required in a round of synchronous communication?
// Why do we even have the concept of a cache in this experiment? 
// How do we stop/start this server if we have a while(1) loop? Do we need to send a start/stop ZMQ message from teh HTTP side (Go) or can we just "collect" the routine and be fine with it?

#define READ  1 // alternative is WRITE but that does not need to be explicit...

// This algorithm is constructed in TCP/IP so an explicit ack is not required. If the message is sent then the ack is implicit in the exit of the send function. 

void serve() {

	// zmq preamble, setup connections to the servers
  // Create sentinel server that figures out who to respond to in this round of communication.
  // Each round of communication is synchronous, the sentinel however is asynchronous. It exists only to server as a queue on if the request is a Read/Write and who its coming from.
	void *ctx_router = zmq_ctx_new();
	void *router = zmq_socket(ctx_router, ZMQ_ROUTER);
	zmq_connect(router, "tcp://*:8080");

	zmq_pollitem_t reqs[] = { {router, 0, ZMQ_POLLIN, 0} };
  while(1) {
		zmq_poll(reqs, 1, 10); // 10ms
    // The message is composed of the ip-address of the client in this round as well as if it's a read or a write.

    //if(msg == READ) {
	    void *ctx_read_rep = zmq_ctx_new();
	    void *read_rep = zmq_socket(ctx_read_rep, ZMQ_REP);
	    void *ctx_read_req = zmq_ctx_new();
	    void *read_req = zmq_socket(ctx_read_req, ZMQ_REQ);

      // Send local latest tag
		  zmq_msg_t rep;
		  zmq_msg_init_size(&rep, 5);
		  zmq_msg_send((&rep), read_rep, 0);

      // Get latest tag
		  zmq_msg_t req;
		  zmq_msg_init_size(&req, 5);
		  zmq_msg_recv((&req), read_req, 0);

	    zmq_close(read_req);
	    zmq_ctx_destroy(ctx_read_req);
	    zmq_close(read_rep);
	    zmq_ctx_destroy(ctx_read_rep);
    //} else {  // Write is empty for now.
    //}
  }

	// Send state variable

	// Update with newest tag and value


	// zmq postamble, teardown connections to the servers
	zmq_close(router);
	zmq_ctx_destroy(ctx_router);
}
