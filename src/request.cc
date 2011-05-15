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
 * Nacteni souboru ze slozky webserveru 
 * @param name of a file
 * @param buffer for content
 */
bool loadFile(string filePath, string &content) {

	string line;
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
	int bytesRecvd;
	// String - document to read from webserver public folder
	string file = "";

	// Need local copy because of problem with pthreads - which tooks pointer 
	getRequestInfo(req, &data);

	HTTPHelper* http;
	if(config.useAVG) {
		http = avg;
	}
	else {
		http = new HTTPHelper();
	}

	http->setSocket(data.connected);
	
	string buffer;
	string fileContent;
	string errMsg;
	
	bytesRecvd = http->read(data, &buffer);
	if (bytesRecvd < 0) {
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

	
	HTTPHelper::HTTPStatus status = http->getFile(file, fileContent);
	
	switch (status) {
		case HTTPHelper::HTTP_INFECTED:
			errMsg = "403 ";
			break;
		case HTTPHelper::HTTP_NOTFOUND:
			errMsg = "404 ";
			break;
		case HTTPHelper::HTTP_OK:
		default:
			errMsg = "200 ";
			break;
	}
	errMsg += file;
	dispatchPrint(errMsg);

	
	if(isDispatchSuitable())	
		dispatchIncreaseResponded();


	http->buildResponse(status, file, fileContent);	
	try {
		http->write();
	} catch (char * e) {
		printError(e);
	}
	

	// closing socket
	close(data.connected);
	if(!config.useAVG) {
		delete http;
	}
	http = NULL;
	return NULL;
}



