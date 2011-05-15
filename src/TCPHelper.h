#ifndef __TCPHELPER_H__
#define __TCPHELPER_H__

#include <string>
#include "request.h"

class TCPHelper {

protected:
	std::string response;
	std::string request;
	std::string host;

	int socketNr;
	int port;

public:	
	TCPHelper();
	~TCPHelper();

	int socket();

	void setHost(std::string);
	void setPort(int);
	
	void connect();
	int callSocket();
	void setSocket(int);
	int startServer();
	int write();
	int read(reqInfo, std::string *);
};


#endif