/*
 *  request.c
 *  obsahuje vse potrebne pro zpracovani HTTP pozadavku
 *
 *  Created by Michal Svec on 8.3.11.
 *
 */

#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>

#include "request.h"



int acceptAndLoadBuffer(int sock, struct sockaddr_in *client_addr, socklen_t *sin_size, char buffer[], int * connected) {
	
	
    // prijmuti pripojeni
    *connected = accept(sock, (struct sockaddr *) client_addr, sin_size);
    if (*connected == -1) {
        fprintf(stderr, "Problem s prijetim spojeni");
        return -1;
    }

	int bytes_received = read(*connected, (void *) buffer, BUFSIZE);
	
    return bytes_received;
}


void parseHttpRequest(int connected) {
	
	int bytes_received;
	char buffer[BUFSIZE];
	
	bytes_received = read(connected, (void *) buffer, BUFSIZE);
	if (bytes_received < 0)
        fprintf(stderr,("recv() error\n"));  
    else if (bytes_received == 0) 
        fprintf(stderr,"Client disconnected unexpectedly.\n");  
    else {  
		printf("%s", buffer);
	}
}



