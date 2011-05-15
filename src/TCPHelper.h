#ifndef __TCPHELPER_H__
#define __TCPHELPER_H__

#include <string>
#include "request.h"

class TCPHelper {
protected:
	int socketNr;
	int port;

	std::string host;

public:	
	TCPHelper(char *, int);
	TCPHelper(int);

	int socket();

	void connect();
	int setSocket();
	int startServer();
	int write(std::string);
	int read(reqInfo, std::string *);
};


#endif