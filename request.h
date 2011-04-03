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



void parseRequest(int);

int acceptAndLoadBuffer(int, struct sockaddr_in *client_addr, socklen_t *sin_size, std::string *);

void * processHttpRequest(void *);

#endif
