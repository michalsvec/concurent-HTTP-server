/*
 *  request.c
 *  obsahuje vse potrebne pro zpracovani HTTP pozadavku
 *
 *  Created by Michal Svec on 8.3.11.
 */

#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include "request.h"
#include "common.h"

#include "HttpHelper.h"

#include "gcd.h"
#include "pthreads.h"	
#include "fork.h"


using namespace std;


/**
 * Connection accept and buffer load
 *
 * @param int socket number
 * @param struct sockaddr_in
 * @param sin_size 
 * @param char[] buffer
 *
 * @return int pocet nactenych bytu
 */
int acceptAndLoadBuffer(reqInfo request, string *buffer) {
	int result = 1;
	char tmp[BUFSIZE];	
	
	
	// if there's anything to load - load it
	while(result > 0) {
		result = read(request.connected, (void *) tmp, BUFSIZE);
		*buffer += tmp;
		
		// if result is smaller than BUFSIZE - whole message was loaded
		// else result == BUFSIZE or result == 0
		if(result < BUFSIZE)
			break;
	}
	
	return result;
}



/**
 * Nacteni souboru ze slozky webserveru 
 * @param name of a file
 * @param buffer for content
 */
bool loadFile(string fileName, string &content) {

	string line;
	string filePath = config.documentRoot + fileName;
	ifstream file (filePath.c_str());

//  cwd detecting	
//	char path1[1000];
//	getcwd(path1, 1000);
	
	if(file) {
		while(getline(file,line)) {
			content += line;
			content += "\r\n";
		}
		return true;
	} 
	else	// file does not exists
		return false;
}



void getRequestInfo(void * req, reqInfo * data) {
	data->connected = ((reqInfo *) req)->connected;
	data->client_addr = ((reqInfo *) req)->client_addr;
	data->sin_size = ((reqInfo *) req)->sin_size;
}



/**
 * Main method for HTTP request processing
 *
 * @param void * reqInfo structure wotj request information - (void *) because pthreads needs this type as a parameter 
 */
void * processHttpRequest(void * req) {

	reqInfo data;
	bool status;
	int bytes_recvd;
	// String - document to read from webserver public folder
	string file = "";

	// Need local copy because of problem with pthreads - which tooks pointer 
	getRequestInfo(req, &data);
	HTTPHelper* http = new HTTPHelper(data.connected);

	string buffer;
	string fileContent;


	bytes_recvd = acceptAndLoadBuffer(data, &buffer);
	if (bytes_recvd < 0) {
		fprintf(stderr,("read() error\n"));
		return NULL;
	}

	if(buffer.length() < 1) {
		cerr << "Empty buffer - terminating" << endl;
		return NULL;
	}

	http->parseHttpRequest(buffer, &file);
	if(file == "") {
		printError("Wrong offset - can't parse file name.");
		return NULL;
	}


	status = loadFile(file, fileContent);
	if(!status) {
		string errMsg = "unable to load file '";
		errMsg += file;
		errMsg += "' => 404";
		printError(errMsg);
	}
	else if(isDispatchSuitable())	
		dispatchIncreaseResponded();


	http->buildResponse(status, file, fileContent);	
	http->sendResponse();

	// closing socket
	close(data.connected);
	delete http;
	http = NULL;
	return NULL;
}
