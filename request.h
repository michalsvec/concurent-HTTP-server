/*
 *  request.h
 *  httpserver
 *
 *  Created by Michal Svec on 8.3.11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include <netinet/in.h>
#include <string>

#define BUFSIZE	1023
#define ROOT_DIR "/Users/misa/School/DIP/httpserver/public/"


void parseRequest(int);

int acceptAndLoadBuffer(int, struct sockaddr_in *client_addr, socklen_t *sin_size, char buffer[]);

void processHttpRequest(int, struct sockaddr_in *client_addr, socklen_t *sin_size);