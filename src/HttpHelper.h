#include <string>

#include "TCPHelper.h"

using namespace std;


/**
 * Class for working with HTTP requests and responses
 */
class HTTPHelper {
	
	string response;
	TCPHelper * tcp;

public:
	HTTPHelper(int);
	
	void parseHttpRequest(string, string *);
	void buildResponse(bool, string, string);
	void sendResponse();
	char * getActualtime();
	string getContentType(string);
	string getFileExtension(string);
};
