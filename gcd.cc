/*
 *  gcd.c
 *  httpserver
 *
 *  Created by Michal Svec on 21.2.11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "gcd.h"
#include "request.h"

#include <stdio.h>
#include <unistd.h>



void parse_request_gcd(int connected, struct sockaddr_in *client_addr, socklen_t *sin_size) {
	printf("GCD parsing\n");
	
	dispatch_queue_t dqueue;	// fronta zpracovavajici pozadavky serveru
	
	// ziskani identifikatoru gloablni konkurentni fronty stredni priority
	dqueue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);


	
	dispatch_async(
				   dqueue,
				   ^{
					   // zpracovani
					   processHttpRequest(connected, client_addr, sin_size);
					   
					   close(connected);
				   }
			   );
	
	return;
}
