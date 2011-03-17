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
#include <string.h>

//#include <iostream>
//#include <string>

#include "request.h"



/**
 * Prijeti spojeni a naplneni vstupniho bufferu
 *
 * @param int cislo socketu
 * @param struct sockaddr_in
 * @param sin_size 
 * @param char[] buffer
 *
 * @return int pocet nactenych bytu
 */
int acceptAndLoadBuffer(int connected, struct sockaddr_in *client_addr, socklen_t *sin_size, char buffer[]) {
	int bytes_received = read(connected, (void *) buffer, BUFSIZE);
    return bytes_received;
}


void sendResponse(int connected, char buffer[]) {

	char response[BUFSIZE];
	bzero(response, strlen(response));

	sprintf(response,"%s\n%s\n%s\n%s\n%s\n\n%s\n", 
			"HTTP/1.0 200 OK",
			"Date: Sat, 15 Jan 2000 14:37:12 GMT",
			"Server: Apache/2.2.3",
			"Content-Type: text/html",
			"Content-Length: 130",
			"<html><body>ahoj</body></html>");

	printf("odeslani:\n%s", response);
	int written = write(connected, (void *) response, BUFSIZE);
	printf("zapsano: %d\n", written);
}


/**
 * Prasovani HTTP pozadavku a navraceni odpovedi
 *
 *
 */
void parseHttpRequest(int connected, char buffer[]) {
	
	printf("parsovani:\n%s", buffer);
	
	sendResponse(connected, buffer);
}


