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
 * @param int cislo socketu
 * @param struct sockaddr_in
 * @param sin_size 
 * @param char[] buffer
 *
 * @return int pocet nactenych bytu
 */
int acceptAndLoadBuffer(int connected, struct sockaddr_in *client_addr, socklen_t *sin_size, string *buffer) {
	int result = 1;
	char tmp[BUFSIZE];

	while(result > 0) {
		result = read(connected, (void *) tmp, BUFSIZE);
		*buffer += tmp;
		
		if(result < BUFSIZE)
			break;
	}

	return result;
}


string buildResponse(int status, string content) {
/*
	// stara verze
	sprintf(resp,"%s\n%s\n%s\n%s\n%s\n\n%s\n", 
			"HTTP/1.0 200 OK",
			"Date: Sat, 15 Jan 2000 14:37:12 GMT",
			"Server: Apache/2.2.3",
			"Content-Type: text/html",
			"Content-Length: 130",
			"<html><body>ahoj</body></html>");
*/

	// delka souboru
	ostringstream str;
	str << content.length();
	
	// cas
	time_t rawtime;
	struct tm * timeinfo;
	timeinfo = localtime ( &rawtime );
	string response;
	
	// typ odpovedi
	if(status)
		response = "HTTP/1.0 200 OK\n";
	else
		response = "HTTP/1.0 404 Not Found\n";
	
	// datum a cas
	// HTTP protokol umoz uje vlozit asctime format
	// viz http://www.w3.org/Protocols/rfc2616/rfc2616-sec3.html 3.3.1 Full Date
//	response += "Date: ";
//	response += asctime (timeinfo);
	//response += "\n";
	
	// typ obsahu
	response += "Content-Type: text/html\n";
	response += "Host: google.com\n";

	// delka obsahu
	response += "Content-Length: ";
	response += str.str();
	response += "\n\n";
	
	// obsah souboru
	response += content;
	
	return response;
}



void sendResponse(int connected, string response) {
	//cout << "odeslani:" << response <<  endl;
	int written = write(connected, (void *) response.c_str(), response.length());
	
	if(written < 0) {
		cout << "chyba write: " << response.length() << endl;
	}
	else {

	//	printf("%i - zapsano: %d\n", reqCounter, written);
	}
}



/**
 * Parsovani HTTP pozadavku a navraceni odpovedi
 * @return string dokument, ktery se ma nacist
 
 GET /file.html HTTP/1.1
 Host: localhost:5000
 Connection: keep-alive
 Cache-Control: max-age=0
 User-Agent: Mozilla/5.0 (Macintosh; U; Intel Mac OS X 10_6_6; en-US) AppleWebKit/534.16 (KHTML, like Gecko) Chrome/10.0.648.133 Safari/534.16
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
bool loadFile(string fileName, string *content) {
	
	string line;
	string filePath = ROOT_DIR+fileName;
	
//	cout << "path:" << filePath << endl;
	
	ifstream file (filePath.c_str());
	
//  cwd detecting	
//	char path1[1000];
//	getcwd(path1, 1000);
	
	
	if(file) {
		while(getline(file,line))
			*content += line;
		
		return true;
	} else 
		return false;
}



/**
 * Celkove spolecne zpracovani HTTP pozadavku pro vsechny metody
 */
void * processHttpRequest(void * req) {
//	cout << "processHttpReq()\n";

	reqInfo * request = (reqInfo *) req;
	string buffer;

	
	// prijmuti pozadavku a nacteni bufferu
	int bytes_recvd = acceptAndLoadBuffer(request->connected, request->client_addr, request->sin_size, &buffer);
	
	if (bytes_recvd < 0) {
		fprintf(stderr,("recv() error\n"));
		return NULL;
	}

	if(buffer.length() < 1) {
		cerr << "Empty buffer - terminating" << endl;
		return NULL;
	}

	
	// String z bufferu
	string requestBuffer = buffer;
	// String - dokument, ktery nacist ze slozky webserveru
	string file;
	// vyparsovani souboru
	parseHttpRequest(requestBuffer, &file);

	// nacteni celeho souboru
	string fileContent;
	bool status = loadFile(file, &fileContent);
	
	if(fileContent.length() < 1) {
		cerr << "unable to load file" << endl;
		return NULL;
	}
		
	string response = buildResponse(status, fileContent);
	sendResponse(request->connected, response);

	close(request->connected);
	return NULL;
}





