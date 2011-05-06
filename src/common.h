#ifndef __COMMON_H__
#define __COMMON_H__


#include <string>
#include <dispatch/dispatch.h>

#include "request.h"



/**
 * Structure with values from configuration file
 */
typedef struct config {
	std::string documentRoot;
	int portNr;
} ConfigVals;


extern ConfigVals config;

/**
 * queues for global operations
 *
 * writing to stdout and writing requests count
 * incrementing accepted and answered requests counter
 * timerf for firing events 
 */
extern dispatch_queue_t commonQ;
extern dispatch_queue_t requestCountQ;
extern dispatch_source_t timer;

extern int requestsAccepted;
extern int requestsResponded;


void printError(std::string);

void dispatchPrint(std::string);
void dispatchIncreaseAccepted();
void dispatchIncreaseResponded();
void dispatchPrintStatus(void *);

/**
 * Accept connection and fill reqInfo structure
 *
 * @param int socket number
 * @param reqInfo information about requesst
 */
void acceptRequest(int, reqInfo*);



/**
 * Accepts request in infinite loop 
 *
 * @param socket number
 * @param pointer to the request processing function
 */
void serverMainLoop(int, void *);



/**
 * Accepts request with dispatch source
 *
 * @param socket number
 * @param pointer to the request processing function
 */
void serverMainSources(int sock, void *);

#endif
