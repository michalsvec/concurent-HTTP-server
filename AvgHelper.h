#ifndef __AVGHELPER_H__
#define __AVGHELPER_H__

#include <string>
#include "HTTPHelper.h"

class AVGHelper: public HTTPHelper  {
	
	
public:
	AVGHelper();
	bool checkFile(std::string);
	HTTPHelper::HTTPStatus getFile(std::string, std::string &);
	void buildRequest(std::string, std::string);
};


#endif