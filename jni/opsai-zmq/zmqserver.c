#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "zmq.h"

int main (int argc, char const *argv[]) {

 void* context = zmq_init(1);

 void* request = zmq_socket(context, ZMQ_REQ);

 printf("Connecting to server\n");

 zmq_connect(request, "tcp://localhost:4040");

 

 zmq_close(request);

 zmq_term(context);

 return 0;

}