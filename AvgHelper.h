#ifndef __AVGHELPER_H__
#define __AVGHELPER_H__
#include <string>
#include "TCPHelper.h"

class AVGHelper: public TCPHelper  {
	
	
public:
	AVGHelper(char *, int);
	int checkFile(std::string);
	
};


#endif