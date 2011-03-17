#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

#include "request.h"

#include "gcd.h"
#include "pthreads.h"
#include "openmpi.h"
#include "fork.h"


#define PORT_NR 5000

// globalni socket
int sock;

typedef enum mode {
	GCD,
	PTHREADS,
	OPENMPI,
	FORK
} ModeType;





void print_help() {

	printf("Benchmarking paralelnich  metod s knihovnou GCD\n");
	printf("\t-h - zobrazeni napovedy\n");
	printf("\t-m <mode> - vyber metody zpracovani paralelnich pozadavku PTHREADS,OPENMPI,GCD,FORK\n");
}



void print_error(char * err) {
	fprintf(stderr, "ERROR: %s", err);
}



/**
 * handler na obsluhu ctrl+c
 * @param int signum
 */
void signal_callback_handler(int signum)
{
	printf("Terminating on signal: %d\n", signum);
	
	close(sock);
	exit(signum);
}



/* parsovani vstupnich argumentu z prikazove radky */
int parseArguments(int argc, const char * argv[], ModeType *mode) {
	for (int i=0; i < argc; i++) {
		if(strcmp(argv[i], "-h") == 0) {
			print_help();
			return 0;
		}
		else if(strcmp(argv[i], "-m") == 0) {
			if(strcmp(argv[i+1], "PTHREADS") == 0)
				*mode = PTHREADS;
			else if(strcmp(argv[i+1], "OPENMPI"))
				*mode = OPENMPI;	
			else if(strcmp(argv[i+1], "FORK"))
				*mode = FORK;	
			else 
				*mode = GCD;
		}
	}
	return 1;
}



int main (int argc, const char * argv[]) {

	ModeType mode = GCD;
	
	if(parseArguments(argc, argv, &mode) == 0)
		return 0;

	
	// ukazatel na funkci zpracovavajici pozadavek
	void (*parse_request)(int, struct sockaddr_in *client_addr, socklen_t *sin_size);
	
	// naplneni ukazatele
	switch (mode) {
		case OPENMPI:
//			parse_request = parse_request_openmpi;
			break;
		case PTHREADS:
//			parse_request = parse_request_pthreads;
			break;
		case FORK:
//			parse_request = parse_request_fork;
			break;
		case GCD:
		default:
			parse_request = parse_request_gcd;
			break;
	}

	
	
	// nastartovani serveru
	int trueflag = 1;
	struct sockaddr_in server_addr, client_addr;
	socklen_t sin_size;
	
	// vytvoreni socketu
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Socket");
		exit(1);
	}
	
	
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &trueflag, sizeof(int)) == -1) {
		perror("Setsockopt");
		exit(1);
	}
	
	server_addr.sin_family = AF_INET;    
	server_addr.sin_port = htons(PORT_NR);
	server_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(server_addr.sin_zero), 8);
	
	// nabindovani socketu
	if (bind(sock, (struct sockaddr *) &server_addr, sizeof(struct sockaddr)) == -1) {
		perror("Unable to bind");
		exit(1);
	}
	
	// zacatek odposlechu
	if (listen(sock, 5) == -1) {
		perror("Listen");
		exit(1);
	}
	
	printf("\nTCPServer started on port %i\n", PORT_NR);
	fflush(stdout);	
	
	sin_size = sizeof(struct sockaddr_in);

	// handler na ukonceni po stisku ctrl+c
	signal(SIGINT, signal_callback_handler);

	
	while(1) {

		// prijmuti pripojeni
		int connected = accept(sock, (struct sockaddr *) &client_addr, &sin_size);
		if (connected == -1) {
			fprintf(stderr, "Problem s prijetim spojeni");
			return -1;
		}

		// int sock, struct sockaddr_in *client_addr, socklen_t *sin_size)
		parse_request(connected, &client_addr, &sin_size);

	}

    return 0;
}
