#ifndef __REQUEST_H__
#define __REQUEST_H__

#include <netinet/in.h>
#include <string>

#define BUFSIZE	1023
// TODO:something better!
#define ROOT_DIR "/Users/misa/School/DIP/httpserver/public/"


/**
 * Structure with request info
 */
typedef struct requestInfo {
	int connected;
	struct sockaddr_in *client_addr;
	socklen_t *sin_size;
} reqInfo;

extern bool showDebug;

void parseRequest(int);

int acceptAndLoadBuffer(reqInfo, std::string *);

void * processHttpRequest(void *);

#endif
