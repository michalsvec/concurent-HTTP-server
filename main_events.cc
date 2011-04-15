/**
 *	Version with event handlers to process requests
 *
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <dispatch/dispatch.h>

#include "request.h"

#include "gcd.h"
#include "pthreads.h"
#include "openmpi.h"
#include "fork.h"


#define PORT_NR 35123

// globalni socket
int sock;
bool showDebug = false;	// turn off debug mode by default

typedef enum mode {
	GCD,
	PTHREADS,
	OPENMPI,
	FORK
} ModeType;


void deb(char * msg) {
	if(showDebug)
		printf("%s\n", msg);
}



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



/**
 * command line arguments parsing
 *
 * @return int result 
 *		0 - parsing okay
 *		1 - print help
 *		2 - invalid mode
 */
int parseArguments(int argc, const char * argv[], ModeType *mode) {
	for (int i=0; i < argc; i++) {
		
		if(strcmp(argv[i], "-h") == 0) {
			print_help();
			return 1;
		}
		else if(strcmp(argv[i], "-d") == 0) {
			showDebug = true;
		}
		else if(strcmp(argv[i], "-m") == 0) {
			if(strcmp(argv[i+1], "PTHREADS") == 0)
				*mode = PTHREADS;
			else if(strcmp(argv[i+1], "OPENMPI") == 0)
				*mode = OPENMPI;	
			else if(strcmp(argv[i+1], "FORK") == 0)
				*mode = FORK;	
			else if(strcmp(argv[i+1], "GCD") == 0)
				*mode = GCD;
			else 
				return 2;
			i++;
		}
	}
	return 0;
}



int main (int argc, const char * argv[]) {
	
	ModeType mode = GCD;
	
	
	int argResult = parseArguments(argc, argv, &mode);
	if(argResult > 1) {
		fprintf(stderr, "Invalid parallel mode!");
		return EXIT_FAILURE;
	}
	
	
	// ukazatel na funkci zpracovavajici pozadavek
	void (*parse_request)(reqInfo);
	
	// naplneni ukazatele
	switch (mode) {
		case OPENMPI:
			parse_request = parse_request_openmpi;
			break;
		case PTHREADS:
			parse_request = parse_request_pthreads;
			break;
		case FORK:
			// avoiding zombies, when using fork()
			signal(SIGCHLD, SIG_IGN);
			parse_request = parse_request_fork;
			break;
		case GCD:
		default:
			parse_request = parse_request_gcd;
			break;
	}
	
	
	
	// nastartovani serveru
	struct sockaddr_in server_addr, client_addr;
	socklen_t sin_size;
	int trueflag;
	
	// vytvoreni socketu
	if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		perror("Socket");
		exit(1);
	}
	
	
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &trueflag, sizeof(int)) == -1) {
		perror("Setsockopt");
		exit(1);
	}
	
	server_addr.sin_family = AF_INET;    // protocol family
	server_addr.sin_port = htons(PORT_NR); // port number
	server_addr.sin_addr.s_addr = INADDR_ANY;	// connection from everywhere
	bzero(&(server_addr.sin_zero), 8);
	
	// nabindovani socketu
	if (bind(sock, (struct sockaddr *) &server_addr, sizeof(struct sockaddr)) == -1) {
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
	
	printf("\nTCPServer started on port %i\n", PORT_NR);
	fflush(stdout);	
	
	sin_size = sizeof(struct sockaddr_in);
	
	// handler na ukonceni po stisku ctrl+c
	signal(SIGINT, signal_callback_handler);
	
	
	dispatch_queue_t queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
	dispatch_source_t readSource = dispatch_source_create(DISPATCH_SOURCE_TYPE_READ, sock, 0, queue);

	if (!readSource) {
		close(sock);
		return NULL;
	}
	
	sigset_t sigs;
    sigemptyset(&sigs);
    sigaddset(&sigs, SIGINFO);
    sigaddset(&sigs, SIGPIPE);
	
	dispatch_source_set_event_handler(readSource, ^{
		
		int connected = accept(sock, (struct sockaddr *) &client_addr, (socklen_t *)&sin_size);
		if (connected == -1) {
			fprintf(stderr, "Problem s prijetim spojeni");
		}
		
		reqInfo request;
		request.connected = connected;
		request.client_addr = (sockaddr_in *) &client_addr;
		request.sin_size = (socklen_t *) &sin_size;
		
		parse_request(request);
	});
	
	// Call the dispatch_resume function to start processing events
    dispatch_resume(readSource);
	dispatch_main();	
	
    return 0;
}