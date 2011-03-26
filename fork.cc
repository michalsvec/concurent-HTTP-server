/*
 *  fork.c
 *  httpserver
 *
 *  Created by Michal Svec on 21.2.11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "fork.h"
#include "request.h"

#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>

void parse_request_fork(reqInfo request) {
	printf("Fork\n");
	
	int child=fork();

	// zpracovani
	if(child == 0) {
		processHttpRequest((void *) &request);
	}
	return;
}
