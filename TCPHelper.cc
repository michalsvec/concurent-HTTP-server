
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>

#include "TCPHelper.h"




TCPHelper::TCPHelper(std::string h, int p) {
	port = p;
	host = h;
}




int TCPHelper::getSocket() {
	return socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
}



void TCPHelper::connect() {

	int sockfd;
    struct sockaddr_in serv_addr;
    struct hostent *server;
	
	
	sockfd = getSocket();
	if (sockfd < 0) 
		throw "Error opening socket!";
	
	server = gethostbyname(host.c_str());
    if (server == NULL) {
		throw "Can't find server!\n";
		exit(0);
    }
    
	bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;

    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = port;
	
    if (::connect(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        throw "Chyba pri pripojovani!\n";
}




int TCPHelper::startServer(int portNr) {
	
	// nastartovani serveru
	struct sockaddr_in serverAddr;
	int trueflag = 1;
	int sock;
	
	// vytvoreni socketu
	if ((sock = getSocket()) == -1) {
		perror("Socket");
		exit(1);
	}
	
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &trueflag, sizeof(int)) == -1) {
		perror("Setsockopt");
		exit(1);
	}
	
	serverAddr.sin_family = AF_INET;    // protocol family
	serverAddr.sin_port = htons(portNr); // port number
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
