
#include <sstream>

#include "AvgHelper.h"
#include "HTTPHelper.h"




AVGHelper::AVGHelper(): HTTPHelper() {

}



/**
 * Very very very simple request builder
 * for AVG the request is only scan filename
 * nothing else
 */
void AVGHelper::buildRequest(std::string method, std::string file) {
	
	ostringstream output;		
	output << method << " " << file;
	request = output.str();
}



/**
 * Check file for virus infiltration
 * sends request to avgtcpd and parse the result
 */
bool AVGHelper::checkFile(std::string file) {

	buildRequest("SCAN", file);
	
	
	
	
	return true;
}


/**
 * Overloaded method from parent TCPHelper
 * loads file from webservers public directory and checks for virus infection
 * in case of some error or infection loads status page from internal folder
 */
int AVGHelper::getFile(std::string fileName, std::string & fileContent) {
	
	bool status = ::loadFile(fileName, fileContent);
	
	if(status) {
		bool virus = checkFile(fileName);

		if(virus) {
			getStatusFile(HTTP_INFECTED, fileContent);
			return HTTP_INFECTED;
		}
		else 		
			return HTTP_OK;
	}
	else {
		getStatusFile(HTTP_NOTFOUND, fileContent);
		return HTTP_NOTFOUND;
	}
}
