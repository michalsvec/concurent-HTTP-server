#include <string>
#include <iostream>
#include <sstream>

#include "common.h"
#include "HttpClass.h"

using namespace std;



HTTPHelper::HTTPHelper(int sock) {
	socket = sock;
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
void HTTPHelper::parseHttpRequest(string request, string *file) {
	
	int offset = request.find("HTTP");
	if(offset < 5) {
		return;
	}	
	
	file->assign(request.substr(5, request.find("HTTP")-6));
	
	if(*file == "")
		file->assign("index.html");
}



// HTTP enables asctime format in response
//  http://www.w3.org/Protocols/rfc2616/rfc2616-sec3.html 3.3.1 Full Date
char * HTTPHelper::getActualtime() {
	time_t rawtime;
	struct tm * timeinfo;
	time ( &rawtime );
	timeinfo = localtime ( &rawtime );

	return asctime (timeinfo);
}	
	


/**
 * HTTP response builder
 *
 * @param status true if file was loaded or false on 404
 * @param content file content
 */
void HTTPHelper::buildResponse(bool status, string content) {
	
	// delka souboru je int - pro prekonvertovani na string pouzit ostringstream
	ostringstream output;
	
	// 404, in case of missing file
	if(!status) {
		output << "HTTP/1.0 404 Not Found\n";
		content = "<h1>404 not found :(</h1> <br />Please try another document.";
	}
	else {
		output << "HTTP/1.0 200 OK\n";
	}
	
	output << "Date: " << this->getActualtime();
	
	// server info and content type
	// TODO: content type detection
	output << "Content-Type: text/html\n";
	output << "Server: GCDForkThreadServer\n";
	output << "Host: michalsvec.cz\n";
	
	// content length
	output << "Content-Length: " << content.length() << "\n\n";
	
	// file content
	output << content;
	
	response = output.str();
}





void HTTPHelper::sendResponse() {
	
	int written = write(socket, (void *) response.c_str(), (size_t) response.length());
	
	if(written < 0)
		cout << "Error sending response. Socket: " << socket << ". Response length: " << response.length() << endl;
	else if(showDebug)
		printf("written: %i to %i\n", written, this->socket);
}


