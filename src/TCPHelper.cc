
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>

#include "TCPHelper.h"
#include "common.h"



// for usage in client apps
TCPHelper::TCPHelper() {
}


TCPHelper::~TCPHelper() {
	close(socketNr);
}


int TCPHelper::socket() {
	return socketNr;
}



int TCPHelper::callSocket() {
	socketNr = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	return socketNr;
}



void TCPHelper::setSocket(int s) {
	socketNr = s;
}


void TCPHelper::setHost(std::string h) {
	host = h;
}



void TCPHelper::setPort(int p) {
	port = p;
}



std::string TCPHelper::getResponse() {
	return response;
}

std::string TCPHelper::getRequest() {
	return request;
}




void TCPHelper::connect() {

	int sockfd;
    struct sockaddr_in serv_addr;
    struct hostent *server;
	
	callSocket();
	if (sockfd < 0) 
		throw "Error opening socket!";
	
	server = gethostbyname(host.c_str());
    if (server == NULL)
		throw "Can't find AVG Tcpd server!\n";

    
	bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);

    if (::connect(socketNr, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        throw "Error in connecting to AVG Tcpd!\n";
}



/**
 * Binds socket to port number 
 */
int TCPHelper::startServer() {
	
	// nastartovani serveru
	struct sockaddr_in serverAddr;
	int trueflag = 1;
	
	// vytvoreni socketu
	if (callSocket() == -1) {
		perror("Socket");
		exit(1);
	}
	
	if (setsockopt(socketNr, SOL_SOCKET, SO_REUSEADDR, &trueflag, sizeof(int)) == -1) {
		perror("Setsockopt");
		exit(1);
	}
	
	serverAddr.sin_family = AF_INET;    // protocol family
	serverAddr.sin_port = htons(port); // port number
	serverAddr.sin_addr.s_addr = INADDR_ANY;	// connection from everywhere
	bzero(&(serverAddr.sin_zero), 8);
	
	// nabindovani socketu
	if (bind(socketNr, (struct sockaddr *) &serverAddr, sizeof(struct sockaddr)) == -1) {
		perror("Unable to bind");
		exit(1);
	}
	
	// listening start
	// 2nd parameter is backlog
	// If a connection request arrives with the queue full, the client may receive an error with an indication of ECONNREFUSED.
	// 128 is MAX
	if (listen(socketNr, 128) == -1) {
		perror("Listen");
		exit(1);
	}
	printf("server started on socket: %d", socketNr);
	return socketNr;
}



/**
 * string is passed in argument. Sometimes we need request, sometimes response
 */
int TCPHelper::write(std::string data) {

	int written = ::write(socketNr, (void *) data.c_str(), (size_t) data.length());

	if(written < 0)
		throw "Error sending response.";
	else if(showDebug)
		printf("written: %i\n", written);

	return written;
}



/**
 * Connection accept and buffer load
 *
 * @param string buffer
 *
 * @return int pocet nactenych bytu
 */
int TCPHelper::read() {
	int result = 1;
	char tmp[BUFSIZE];	
	
	response = "";
	// if there's anything to load - load it
	while(result > 0) {
		result = ::read(socketNr, (void *) tmp, BUFSIZE);
		response += tmp;
		
		// if result is smaller than BUFSIZE - whole message was loaded
		// else result == BUFSIZE or result == 0
		if(result < BUFSIZE)
			break;
	}
	
	return result;
}



