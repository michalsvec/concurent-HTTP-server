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


#include "gcd.h"
#include "pthreads.h"	
#include "fork.h"


using namespace std;


/**
 * Prijeti spojeni a naplneni vstupniho bufferu
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
 * HTTP response builder
 *
 * @param status true if file was loaded or false on 404
 * @param content file content
 */
string buildResponse(bool status, string content) {

	// delka souboru je int - pro prekonvertovani na string pouzit ostringstream
	ostringstream contentLength;
	
	// response time
	time_t rawtime;
	struct tm * timeinfo;
	time ( &rawtime );
	timeinfo = localtime ( &rawtime );

	string response = "HTTP/1.0 200 OK\n";
	
	// 404, in case of missing file
	if(!status) {
		response = "HTTP/1.0 404 Not Found\n";
		content = "<h1>404 not found :(</h1> <br />Please try another document.";
	}
	
	// HTTP protokol umoznuje vlozit asctime format
	// viz http://www.w3.org/Protocols/rfc2616/rfc2616-sec3.html 3.3.1 Full Date
	response += "Date: ";
	response += asctime (timeinfo);
	
	// server info and content type
	// TODO: content type detection
	response += "Content-Type: text/html\n";
	response += "Server: GCDForkThreadServer\n";
	response += "Host: michalsvec.cz\n";

	// content length
	contentLength << content.length();
	response += "Content-Length: ";
	response += contentLength.str();
	response += "\n\n";
	
	// file content
	response += content;
	response += "\n";
	
	return response;
}



void sendResponse(int connected, string response) {
	int written = write(connected, (void *) response.c_str(), (size_t) response.length());
	
	if(written < 0)
		cout << "Error sending response. Response length: " << response.length() << endl;
	else if(showDebug)
		printf("written: %i to %i\n", written, connected);
}



/**
 * Parsovani HTTP pozadavku a navraceni odpovedi
 * @return string dokument, ktery se ma nacist
 
 GET /file.html HTTP/1.1
 Host: localhost:5000
 Connection: keep-alive
 Cache-Control: max-age=0
 User-Agent: Mozilla/5.0 (Macintosh; U; Intel Mac OS X 10_6_6; en-US) .....
 Accept: application/xml,application/xhtml+xml,text/html;q=0.9,text/plain;q=0.8,image/png,* / *;q=0.5
 Accept-Encoding: gzip,deflate,sdch
 Accept-Language: cs-CZ,cs;q=0.8
 Accept-Charset: windows-1250,utf-8;q=0.7,*;q=0.3
 Cookie: ...nejake cookie data...
 */
void parseHttpRequest(string request, string *file) {
	
	int offset = request.find("HTTP");
	if(offset < 5) {
		return;
	}	

	file->assign(request.substr(5, request.find("HTTP")-6));
	
	if(*file == "")
		file->assign("index.html");
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
		while(getline(file,line))
			content += line;
		return true;
	} 
	else	// file does not exists
		return false;
}



void getRequestInfo(void * req, reqInfo * data) {
	data->connected = ((reqInfo *) req)->connected;
	data->client_addr = ((reqInfo *) req)->client_addr;
	data->sin_size = ((reqInfo *) req)->sin_size;
	data->commonQ = ((reqInfo *) req)->commonQ;
	data->requestCountQ = ((reqInfo *) req)->requestCountQ;
	data->requestsAccepted = ((reqInfo *) req)->requestsAccepted;
	data->requestsResponded = ((reqInfo *) req)->requestsResponded;
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
	string response, file = "";


	
	// Need local copy because of problem with pthreads - which tooks pointer 
	getRequestInfo(req, &data);
	
	commonQ = *(data.commonQ);
	
	string buffer;
	string fileContent;
	

	if(showDebug)
		printf("Request: %i\n", data.connected);
	
	
	bytes_recvd = acceptAndLoadBuffer(data, &buffer);
	if (bytes_recvd < 0) {
		fprintf(stderr,("read() error\n"));
		return NULL;
	}

	if(buffer.length() < 1) {
		cerr << "Empty buffer - terminating" << endl;
		return NULL;
	}
	
	parseHttpRequest(buffer, &file);

	if(file == "") {
		printError("Wrong offset - can't parse file name.");
		return NULL;
	}

	
	status = loadFile(file, fileContent);
	if(!status) {
		string errMsg = "unable to load file '";
		errMsg += file += "' => 404";
		printError(errMsg);
	}
	else if(isDispatchSuitable())	
		dispatchIncreaseResponded(*(data.requestCountQ), data.requestsResponded);


	response = buildResponse(status, fileContent);	
	sendResponse(data.connected, response);

	// closing socket
	close(data.connected);
	if(showDebug)
		printf("closed socket: %i\n", data.connected);

	return NULL;
}
