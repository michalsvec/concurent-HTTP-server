/*
 *  pthreads.c
 *  httpserver
 *
 *  Created by Michal Svec on 21.2.11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "pthreads.h"
#include "request.h"

#include <pthread.h>
#include <stdio.h>


void parse_request_pthreads(reqInfo request) {
	printf("pthreads");
	
	pthread_t thread;

	//int result = pthread_create(&thread, NULL, processHttpRequest, (void *) request);
	
	
	// funkcni
	//int result = pthread_create(&thread, NULL, NULL, &request);
	int result = pthread_create(&thread, NULL, processHttpRequest, NULL);
	
	
	if(result == 0)
		fprintf(stderr, "Thread creating error.");
	
	return;
}