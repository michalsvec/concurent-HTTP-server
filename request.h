/*
 *  request.h
 *  httpserver
 *
 *  Created by Michal Svec on 8.3.11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef __REQUEST_H__
#define __REQUEST_H__

#include <netinet/in.h>
#include <string>

#define BUFSIZE	1023
// TODO: make better!
#define ROOT_DIR "/Users/misa/School/DIP/httpserver/public/"


/**
 * Structure with request info
 *
 * int socket number
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
