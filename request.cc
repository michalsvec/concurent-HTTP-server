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


std::string buildResponse(int status, std::string content) {
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
	std::ostringstream str;
	str << content.length();
	
	// cas
	time_t rawtime;
	struct tm * timeinfo;
	timeinfo = localtime ( &rawtime );
	std::string response;
	
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



void sendResponse(int connected, std::string response) {
	std::cout << "odeslani:" << response << std:: endl;
	int written = write(connected, (void *) response.c_str(), response.length());
	printf("zapsano: %d\n", written);
}



/**
 * Parsovani HTTP pozadavku a navraceni odpovedi
 * @return std::string dokument, ktery se ma nacist
 
 GET /soubor.html HTTP/1.1
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
void parseHttpRequest(std::string request, std::string *file) {
	file->assign(request.substr(5, request.find("HTTP")-6));
	
	if(*file == "")
		file->assign("index.html");
	
	std::cout << "soubor:" << *file << std::endl;
}




/**
 * Nacteni souboru ze slozky webserveru 
 * TODO: definice vlastni slozky pro webserver
 */
bool loadFile(std::string fileName, std::string *content) {
	
	std::string line;
	
	std::string filePath = ROOT_DIR+fileName;
	
	std::cout << "path:" << filePath << std::endl;
	
	std::ifstream file (filePath.c_str());
	
	
//	char path1[1000];  // This is a buffer for the text
//	getcwd(path1, 1000);
	
	
	if(file) {
		while(std::getline(file,line))
			*content += line;
		
//		std::cout << "obsah: " << content << std::endl;
		
		return true;
	} else {
		return false;
	}
}



/**
 * Celkove spolecne zpracovani HTTP pozadavku pro vsechny metody
 */
void processHttpRequest(int connected, struct sockaddr_in *client_addr, socklen_t *sin_size) {

	char buffer[BUFSIZE];
	
	// prijmuti pozadavku a nacteni bufferu
	int bytes_recvd = acceptAndLoadBuffer(connected, client_addr, sin_size, buffer);
	
	if (bytes_recvd < 0) {
		fprintf(stderr,("recv() error\n"));
		return;
	} else if (bytes_recvd == 0) {
		fprintf(stderr,"Client disconnected unexpectedly.\n");
		return;
	}

	// String z bufferu
	std::string request ((char *)buffer);
	// String - dokument, ktery nacist ze slozky webserveru
	std::string file;
	// zjisteni
	parseHttpRequest(request, &file);

	// nacteni celeho souboru
	//bool loadFile(std::string filePath, std::string content) {
	std::string fileContent;
	bool status = loadFile(file, &fileContent);
	
	std::string response = buildResponse(status, fileContent);
	sendResponse(connected, response);
}





