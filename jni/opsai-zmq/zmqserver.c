#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "zmq.h"

int main (int argc, char const *argv[]) {

	void* context = zmq_ctx_new();
	void* respond = zmq_socket(context, ZMQ_REP);
	zmq_bind(respond, "tcp://*:35901");
	printf("Starting…\n");
	int k = -1;

	while(1) { 
		zmq_msg_t request; 
		zmq_msg_init(&request);
		int check;
		check = zmq_msg_recv(&request, respond, 0);
		zmq_msg_close(&request);

		if (check != -1 && k == -1) {
			k = 0;
			system("/system/lib/opsaivncserver.so &");
			
		}
		zmq_msg_t reply;
		zmq_msg_init_size(&reply, strlen("world"));
		memcpy(zmq_msg_data(&reply), "world", 5);
		zmq_msg_send(&reply, respond, 0);
		zmq_msg_close(&reply);
	}

	zmq_close(respond);
	zmq_ctx_destroy(context);

	return 0;

}