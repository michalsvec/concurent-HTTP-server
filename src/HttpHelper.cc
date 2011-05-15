#include <string>
#include <iostream>
#include <sstream>

#include "common.h"
#include "request.h"
#include "TCPHelper.h"
#include "HttpHelper.h"

using namespace std;



HTTPHelper::HTTPHelper(): TCPHelper() {

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
	


string HTTPHelper::getContentType(string e) {
	if(e == "html" || e == "htm")
		return "text/html";
	if(e == "css")
		return "text/css";
	if(e == "jpg" || e == "jpeg")
		return "image/jpeg";
	if(e == "js")
		return "application/x-javascript";
	if(e == "txt")
		return "text/plain";
	if(e == "png")
		return "image/png";
	if(e == "ico")
		return "image/x-icon";
	
	return "text/plain";
}



string HTTPHelper::getFileExtension(string filename) {
	return filename.substr(filename.find_last_of(".") + 1);
}



/**
 * HTTP response builder
 *
 * @param status true if file was loaded or false on 404
 * @param content file content
 */
void HTTPHelper::buildResponse(HTTPStatus status, string filename, string content) {
	
	// delka souboru je int - pro prekonvertovani na string pouzit ostringstream
	ostringstream output;
	
	switch (status) {
		case HTTP_NOTFOUND:
			output << "HTTP/1.0 404 Not Found\n";
			break;
		case HTTP_INFECTED:
			output << "HTTP/1.0 123 Infected\n";
		case HTTP_OK:
		default:
			output << "HTTP/1.0 200 OK\n";
			break;
	}
	
	output << "Date: " << ::getActualtime();
	
	// server info and content type
	std::string extension = this->getFileExtension(filename);
	if(status != HTTP_OK)
		extension = "html";
	output << "Content-Type: " << this->getContentType(extension) << "\n";
	output << "Server: GCDForkThreadServer\n";
	output << "Host: michalsvec.cz\n";
	
	// content length
	output << "Content-Length: " << content.length() << "\n\n";
	
	// file content
	output << content;
	
	response = output.str();
}



/**
 * Very very very simple request builder
 * in this version only builds header with desired method and filename
 */
void HTTPHelper::buildRequest(std::string method, std::string file) {

	ostringstream output;		
	output << method << " " << file << " HTTP/1.0\n";
	request = output.str();
}



/**
 * Loads status file. e.g. in case of missing or infected file
 * from webserver internal folder
 */
void HTTPHelper::getStatusFile(HTTPStatus status, std::string & fileContent) {
	bool result;
	std::string fileName;
	
	fileName = config.internalRoot;
	
	switch(status) {
		case HTTP_INFECTED:
			fileName += "403.html";
			result = ::loadFile(fileName, fileContent);			
			break;
		case HTTP_NOTFOUND:
			fileName += "404.html";
			result = ::loadFile(fileName, fileContent);			
			break; 
		case HTTP_OK:
			break;
	}
}



/**
 * Loads file from webserver public folder or status file
 * in case of some error
 */
HTTPHelper::HTTPStatus HTTPHelper::getFile(std::string fileName, std::string & fileContent) {

	string filePath = config.documentRoot + fileName;
	
	bool status = ::loadFile(filePath, fileContent);

	if(status)
		return HTTP_OK;
	else {
		getStatusFile(HTTP_NOTFOUND, fileContent);
		return HTTP_NOTFOUND;
	}
}





