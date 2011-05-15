#ifndef __TCPHELPER_H__
#define __TCPHELPER_H__

#include <string>


class TCPHelper {
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
};


#endif