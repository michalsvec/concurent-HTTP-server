#ifndef __AVGHELPER_H__
#define __AVGHELPER_H__

#include <string>
#include "HTTPHelper.h"

class AVGHelper: public HTTPHelper  {
	
	
public:
	
	bool useAVG;
	
	AVGHelper();
	HTTPHelper::HTTPStatus checkFile(std::string);
	HTTPHelper::HTTPStatus getFile(std::string, std::string &);
	void buildRequest(std::string, std::string);
	
	std::string getResponseCode();
};


#endif