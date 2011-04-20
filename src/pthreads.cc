#include "pthreads.h"
#include "request.h"

#include <pthread.h>
#include <stdio.h>


void parse_request_pthreads(reqInfo request) {
	
	pthread_t thread;

	int result = pthread_create(&thread, NULL, processHttpRequest, (void *) &request);
	
	if(result != 0)
		fprintf(stderr, "Thread creating error. Error number: %i\n", result);
	
	pthread_detach(thread);
	
	return;
}