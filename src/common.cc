
#include <dispatch/dispatch.h>
#include <sys/time.h>
#include <time.h>
#include <string>
#include <iostream>
#include <sstream>

#include "request.h"
#include "common.h"

#include "gcd.h"
#include "pthreads.h"	
#include "fork.h"


/** 
 * simple detection if paralle mode is one of the "global variable safe" method
 */
int isDispatchSuitable() {
	return parallelMode == GCD || parallelMode == GCD_OWN || parallelMode == PTHREADS;
}



void printError(std::string err) {
	if(isDispatchSuitable())
		dispatch_async(commonQ, ^{ 
			fprintf(stderr, "ERROR: %s\n", err.c_str());
		});
	else
		fprintf(stderr, "ERROR: %s\n", err.c_str());
}



void dispatchPrint(std::string msg) {
	if(isDispatchSuitable())
		dispatch_async(commonQ, ^{
			printf("%s\n", msg.c_str());
		});
	else
		printf("%s\n", msg.c_str());
}



void dispatchIncreaseAccepted() {
	dispatch_async(requestCountQ, ^{ requestsAccepted++; });
}



/**
 * Increase global variable in serial queue
 */
void dispatchIncreaseResponded() {
	__block int * tmp = &requestsResponded;
	dispatch_async(requestCountQ, ^{ 
		(*tmp)++; 
	});
}



void dispatchPrintStatus(void * param) {

	std::ostringstream output;
	
	output << "accepted:  " << requestsAccepted << "\n";
	output << "responded: " << requestsResponded << "\n";
	
	dispatchPrint(output.str());
	
	return;
}



// HTTP enables asctime format in response
//  http://www.w3.org/Protocols/rfc2616/rfc2616-sec3.html 3.3.1 Full Date
char * getActualtime() {
	time_t rawtime;
	struct tm * timeinfo;
	time ( &rawtime );
	timeinfo = localtime ( &rawtime );
	
	return asctime (timeinfo);
}



void acceptRequest(int sock, reqInfo * req){

	if(showDebug) { 
		std::ostringstream msg;
		msg << "from sock: " << sock;
		dispatchPrint(msg.str());
	}
	
	// accept connection
	req->connected = accept(sock, (struct sockaddr *) &req->client_addr, (socklen_t *) &req->sin_size);
	if (req->connected == -1) {
		printError("Connection accept problem.");
	}
	
	if(isDispatchSuitable())
		dispatchIncreaseAccepted();
}



void serverMainLoop(int sock, void * function) {
	void (*parse_request)(reqInfo) = (void (*)(reqInfo))function;

	timeval time;
	gettimeofday(&time, NULL);
	double start, end;

	while(1) {
		reqInfo request;
		acceptRequest(sock, &request);

		gettimeofday(&time, NULL);
		start = time.tv_sec+(time.tv_usec/1000000.0);

		if(showDebug) {
			std::ostringstream msg;
			msg << "serverMainLoop - reqInfo.connected: " << request.connected;
			dispatchPrint(msg.str());
		}

		parse_request(request);

		gettimeofday(&time, NULL);
		end = time.tv_sec+(time.tv_usec/1000000.0);

		if(showDebug) {
			std::ostringstream msg;
			float delta = end-start;
			msg << "time delta: " << delta;
			dispatchPrint(msg.str());
		}
	}
}



void serverMainSources(int sock, void * function) {
	void (*parse_request)(reqInfo) = (void (*)(reqInfo))function;	
	
	// creating dispatch source and geting the queue
	dispatch_queue_t queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
	dispatch_source_t readSource = dispatch_source_create(DISPATCH_SOURCE_TYPE_READ, sock, 0, queue);

	if (!readSource) {
		printError("Unable to create source!");
		close(sock);
		return;
	}
	
	dispatch_source_set_cancel_handler(readSource, ^{
		close(sock); 
	});
	
	
	dispatch_source_set_event_handler(readSource, ^{
		reqInfo request;
		acceptRequest(sock, &request);

		parse_request(request);
	});
	
	// Call the dispatch_resume function to start processing events
    dispatch_resume(readSource);
	dispatch_main();	
}