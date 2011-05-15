

#include <string>


class TCPHelper {

	std::string host;
	int port;
public:	
	TCPHelper(std::string, int);
	void connect();
	int getSocket();
	int startServer(int);
	
	
};