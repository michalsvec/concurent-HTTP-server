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
	
	
//	cout << "reading from: " << request.connected << endl;

	// if there's anything to load - load it
	while(result > 0) {
		result = read(request.connected, (void *) tmp, BUFSIZE);
		*buffer += tmp;
		
		// if result is smaller than BUFSIZE - whole message was loaded
		// else result == BUFSIZE or result == 0
		if(result < BUFSIZE)
			break;
	}
	
//	cout << result << endl;
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
	ostringstream str;
	str << content.length();
	
	// response time
	time_t rawtime;
	struct tm * timeinfo;
	time ( &rawtime );
	timeinfo = localtime ( &rawtime );

	string response = "HTTP/1.0 200 OK\n";
	
	// 404, in case of missing file
	if(!status) {
		response = "HTTP/1.0 404 Not Found\n";
		return response;
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
	response += "Content-Length: ";
	response += str.str();
	response += "\n\n";
	
	// file content
	response += content;
	
	return response;
}



void sendResponse(int connected, string response) {
	int written = write(connected, (void *) response.c_str(), (size_t) response.length());
	
	if(written < 0)
		cout << "Error sending response. Response length: " << response.length() << endl;
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
	file->assign(request.substr(5, request.find("HTTP")-6));
	
	if(*file == "")
		file->assign("index.html");
	
	//cout << "file: " << *file << endl;
}




/**
 * Nacteni souboru ze slozky webserveru 
 * @param name of a file
 * @param buffer for content
 */
bool loadFile(string fileName, string &content) {
	
	string line;
	string filePath = ROOT_DIR+fileName;
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



/**
 * Main method for HTTP request processing
 *
 * @param void * reqInfo structure wotj request information - (void *) because pthreads needs this type as a parameter 
 */
void * processHttpRequest(void * req) {

	// Need local copy because of problems with pthreads - which tooks pointer 
	reqInfo data;
	data.connected = ((reqInfo *) req)->connected;
	data.client_addr = ((reqInfo *) req)->client_addr;
	data.sin_size = ((reqInfo *) req)->sin_size;

	
	string buffer;
	string fileContent;
	
	
	if(showDebug)
		printf("Request: %i\n", data.connected);
	
	int bytes_recvd = acceptAndLoadBuffer(data, &buffer);
	
	if (bytes_recvd < 0) {
		fprintf(stderr,("read() error\n"));
		return NULL;
	}

	if(buffer.length() < 1) {
		cerr << "Empty buffer - terminating" << endl;
		return NULL;
	}
	
	// String - dokument, ktery nacist ze slozky webserveru
	string file;
	// get the file name
	parseHttpRequest(buffer, &file);

	bool status = loadFile(file, fileContent);
	
	if(fileContent.length() < 1)
		cerr << "unable to load file " << file << " => 404" << endl;

	string response = buildResponse(status, fileContent);
	sendResponse(data.connected, response);

	// closing socket
	close(data.connected);
//	cout << "closing: " << data.connected << endl;
	return NULL;
}


