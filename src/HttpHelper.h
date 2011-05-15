#ifndef __HTTPHELPER_H__
#define __HTTPHELPER_H__

#include <string>

#include "TCPHelper.h"

using namespace std;


/**
 * Class for working with HTTP requests and responses
 */
class HTTPHelper: public TCPHelper {

protected:
	string response;
	string request;

public:	
	enum HTTPStatus {
		HTTP_OK = 200,
		HTTP_NOTFOUND = 404,
		HTTP_INFECTED = 403
	};
	

	HTTPHelper(int);

	void parseHttpRequest(string, string *);
	void buildResponse(HTTPStatus, string, string);
	void buildRequest(string method, string file);
	HTTPStatus getFile(std::string, std::string &);
	void getStatusFile(HTTPStatus, std::string &);
	
	char * getActualtime();
	string getContentType(string);
	string getFileExtension(string);
};


#endif
