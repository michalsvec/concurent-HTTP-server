
#include <sstream>

#include "common.h"
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
	output << method << " " << file << "\n";
	request = output.str();
}



/**
 * Check file for virus infiltration
 * sends request to avgtcpd and parse the result
 */
bool AVGHelper::checkFile(std::string file) {

	AVGHelper * request = new AVGHelper();
	request->setSocket(avg->getSocket());
	request->buildRequest("SCAN", file);
	request->write(request->getRequest());

	
	request->read();
//	printf("response: %s\n", request->getResponse().c_str());
	
	return true;
}


/**
 * Overloaded method from parent TCPHelper
 * loads file from webservers public directory and checks for virus infection
 * in case of some error or infection loads status page from internal folder
 */
HTTPHelper::HTTPStatus AVGHelper::getFile(std::string fileName, std::string & fileContent) {
	
	string filePath = config.documentRoot + fileName;
	
	bool status = ::loadFile(filePath, fileContent);
	
	if(status) {
		bool virus = checkFile(filePath);

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
