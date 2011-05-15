
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



std::string AVGHelper::getResponseCode() {
	return response.substr(0,3);
}



/**
 * Check file for virus infiltration
 * sends request to avgtcpd and parse the result
 */
HTTPHelper::HTTPStatus AVGHelper::checkFile(std::string file) {

	AVGHelper * request = new AVGHelper();
	

	request->setPort(config.avgPort);
	request->setHost((char *) config.avgHost.c_str());
	
	try {
		request->connect();
		request->read();	// read initial messages of AVG Tcpd
	} catch (char * e) {
		printf("%s\n", e);
		return STATUS_UNDEFINED;
	}

	request->buildRequest("SCAN", file);
	request->write(request->getRequest());
	request->read();

	std::string code = request->getResponseCode();
	delete request;

	if(code == "200")
		return HTTP_OK;
	else if(code == "403")
		return HTTP_INFECTED;
	else
		return STATUS_UNDEFINED;
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
		if( checkFile(filePath) == HTTP_INFECTED) {
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
