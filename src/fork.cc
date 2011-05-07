#include "fork.h"
#include "request.h"
#include "common.h"

#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <iostream>

void parse_request_fork(reqInfo request) {
	
	int child=fork();

	// zpracovani v nove procesu
	if(child == -1) {
		fprintf(stderr, "fork error!\n");
	}
	else if(child == 0) {
		processHttpRequest((void *) &request);
		exit(EXIT_SUCCESS);
	}

	return;
}
