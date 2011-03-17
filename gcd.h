/*
 *  gcd.h
 *  httpserver
 *
 *  Created by Michal Svec on 21.2.11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include <dispatch/dispatch.h>
#include <netinet/in.h>


// zpracovani pozadavku pomoci GCD 
void parse_request_gcd(int connected, struct sockaddr_in *client_addr, socklen_t *sin_size);