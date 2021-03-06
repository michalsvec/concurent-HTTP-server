#ifndef __REQUEST_H__
#define __REQUEST_H__

#include <netinet/in.h>
#include <dispatch/dispatch.h>
#include <string>

#define BUFSIZE	1023



/**
 * Directory with files served by webserver
 */
extern std::string documentRoot;



/**
 * Structure with request info
 */
typedef struct requestInfo {
	int socket;
	sockaddr_in *client_addr;
	socklen_t *sin_size;
	dispatch_queue_t * commonQ;
	dispatch_queue_t * requestCountQ;
	int * requestsAccepted;
	int * requestsResponded;
	
	bool useAVG;
	std::string avgHost;
	int avgPort;
} reqInfo;

extern bool showDebug;

void parseRequest(int);

int acceptAndLoadBuffer(reqInfo, std::string *);

void * processHttpRequest(void *);

char * getActualtime();

bool loadFile(std::string, std::string &);

#endif
