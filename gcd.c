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
	printf("GCD parsing");
	
	dispatch_queue_t dqueue;	// fronta zpracovavajici pozadavky serveru
	
	// ziskani identifikatoru gloablni konkurentni fronty stredni priority
	dqueue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);


	
	dispatch_async(
				   dqueue,
				   ^{
					   //int id = ntohs(*client_addr.sin_port);
					   char buffer[BUFSIZE];
					   
					   // prijmuti pozadavku a nacteni bufferu
					   int bytes_recvd = acceptAndLoadBuffer(connected, client_addr, sin_size, buffer);
					   
					   // zpracovani
					   
					   // odeslani odpovedi
					   printf("buffer: %s", buffer);
//					   int loadBuffer(int sock, struct sockaddr_in *client_addr, socklen_t *sin_size, &connected);
//					   parseRequest();
					   
//					   bytes_received = write(connected, (void *) buffer, BUFSIZE);
					   close(connected);
				   }
			   );
	
	return;
}
