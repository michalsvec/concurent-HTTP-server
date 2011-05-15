
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>

#include "TCPHelper.h"
#include "common.h"


// use in server apps
TCPHelper::TCPHelper(char * h, int p) {
	port = p;
	host = h;
}


// for usage in client apps
TCPHelper::TCPHelper(int p) {
	port = p;
}



int TCPHelper::socket() {
	return socketNr;
}



int TCPHelper::setSocket() {
	socketNr = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	return socketNr;
}



void TCPHelper::connect() {

	int sockfd;
    struct sockaddr_in serv_addr;
    struct hostent *server;
	
	
	sockfd = setSocket();
	if (sockfd < 0) 
		throw "Error opening socket!";
	
	server = gethostbyname(host.c_str());
    if (server == NULL) {
		throw "Can't find server!\n";
    }
    
	bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;

    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = port;
	
    if (::connect(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        throw "Chyba pri pripojovani!\n";
}



/**
 * Binds socket to port number 
 */
int TCPHelper::startServer() {
	
	// nastartovani serveru
	struct sockaddr_in serverAddr;
	int trueflag = 1;
	int sock;
	
	// vytvoreni socketu
	if ((sock = setSocket()) == -1) {
		perror("Socket");
		exit(1);
	}
	
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &trueflag, sizeof(int)) == -1) {
		perror("Setsockopt");
		exit(1);
	}
	
	serverAddr.sin_family = AF_INET;    // protocol family
	serverAddr.sin_port = htons(port); // port number
	serverAddr.sin_addr.s_addr = INADDR_ANY;	// connection from everywhere
	bzero(&(serverAddr.sin_zero), 8);
	
	// nabindovani socketu
	if (bind(sock, (struct sockaddr *) &serverAddr, sizeof(struct sockaddr)) == -1) {
		perror("Unable to bind");
		exit(1);
	}
	
	// listening start
	// 2nd parameter is backlog
	// If a connection request arrives with the queue full, the client may receive an error with an indication of ECONNREFUSED.
	// 128 is MAX
	if (listen(sock, 128) == -1) {
		perror("Listen");
		exit(1);
	}
	
	return sock;
}



int TCPHelper::write(std::string response) {

	int written = ::write(socketNr, (void *) response.c_str(), (size_t) response.length());

	if(written < 0)
		throw "Error sending response.";
	else if(showDebug)
		printf("written: %i\n", written);

	return written;
}



/**
 * Connection accept and buffer load
 *
 * @param reqInfo informations about request
 * @param string buffer
 *
 * @return int pocet nactenych bytu
 */
int TCPHelper::read(reqInfo request, std::string *buffer) {
	int result = 1;
	char tmp[BUFSIZE];	
	
	
	// if there's anything to load - load it
	while(result > 0) {
		result = ::read(request.connected, (void *) tmp, BUFSIZE);
		*buffer += tmp;
		
		// if result is smaller than BUFSIZE - whole message was loaded
		// else result == BUFSIZE or result == 0
		if(result < BUFSIZE)
			break;
	}
	
	return result;
}



