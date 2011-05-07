/**
 *	Version with infinite loop to process requests
 *
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

#include <dispatch/dispatch.h>
#include <dispatch/source.h>

#include "common.h"
#include "request.h"

#include "lib/configfile/configfile.h"

#include "gcd.h"
#include "pthreads.h"	
#include "fork.h"



int sock;
bool showDebug = false;	// turn off debug mode by default
std::string documentRoot = "";

ConfigVals config;

dispatch_queue_t commonQ;
dispatch_queue_t requestCountQ;

dispatch_source_t timer;


int requestsAccepted = 0;
int requestsResponded = 0;


/**
 * Method of parallel processing
 */
typedef enum {
	GCD,
	GCD_OWN,
	PTHREADS,
	OPENMPI,
	FORK
} ModeType;



/**
 * Method of waiting for requests requests
 */
typedef enum {
	WHILE,
	SOURCE
} RequestType;



void deb(char * msg) {
	if(showDebug)
		printf("%s\n", msg);
}



void printHelp() {
printf("Benchmarking paralelnich metod s knihovnou GCD\n \
\t-h - zobrazeni napovedy\n	\
\t-m <mode> - vyber metody zpracovani paralelnich pozadavku PTHREADS, GCD, FORK, GCD_OWN\n \
\t-p <mode> - vyber metody cekani na pozadavky WHILE, SOURCE\n");
}



/**
 * handler na obsluhu ctrl+c
 * @param int signum
 */
void signalCallbackHandler(int signum) {
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
int parseArguments(int argc, const char * argv[], ModeType *pMode, RequestType * rMode) {
	for (int i=0; i < argc; i++) {

		if(strcmp(argv[i], "-h") == 0) {
			printHelp();
			return 1;
		}
		else if(strcmp(argv[i], "-d") == 0) {
			showDebug = true;
		}
		else if(strcmp(argv[i], "-m") == 0) {
			if(strcmp(argv[i+1], "PTHREADS") == 0)
				*pMode = PTHREADS;
			else if(strcmp(argv[i+1], "FORK") == 0)
				*pMode = FORK;	
			else if(strcmp(argv[i+1], "GCD") == 0)
				*pMode = GCD;
			else if(strcmp(argv[i+1], "GCD_OWN") == 0)
				*pMode = GCD_OWN;
			else 
				return 2;
			i++;
		}
		else if(strcmp(argv[i], "-p") == 0) {
			if(strcmp(argv[i+1], "WHILE") == 0)
				*rMode = WHILE;
			else if(strcmp(argv[i+1], "SOURCE") == 0)
				*rMode = SOURCE;
			else
				return 3;
		}
	}
	return 0;
}



/**
 * Load config file
 */
void loadConfig() {
	ConfigFile cfg("config.cfg");

	config.documentRoot = cfg.getvalue<std::string>("document_root");
	config.portNr = cfg.getvalue<int>("port");
	config.reqInfoInterval = cfg.getvalue<int>("reqInfoInterval");
}



int main (int argc, const char * argv[]) {

	ModeType parallelMode = GCD;
	RequestType requestProcess = WHILE;	
	
	int argResult = parseArguments(argc, argv, &parallelMode, &requestProcess);
	if(argResult == 2) {
		printError("Invalid parallel mode!");
		return EXIT_FAILURE;
	}
	if(argResult == 3) {
		printError("Invalid waiting mode!");
		return EXIT_FAILURE;
	}

	loadConfig();
	
	// ukazatel na funkci zpracovavajici pozadavek
	void (*parse_request)(reqInfo);
	
	// naplneni ukazatele
	switch (parallelMode) {
		case PTHREADS:
			parse_request = parse_request_pthreads;
			printf("mode: PTHREADS");
			break;
		case FORK:
			// avoiding zombies, when using fork()
			signal(SIGCHLD, SIG_IGN);
			parse_request = parse_request_fork;
			printf("mode: FORK");
			break;
		case GCD_OWN:
			parse_request = parse_request_gcd_own_queue;
			printf("mode: GCD_OWN");
			break;
		case GCD:
		default:
			parse_request = parse_request_gcd;
			printf("mode: GCD");
			break;
	}

	
	
	// nastartovani serveru
	struct sockaddr_in server_addr;
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
	server_addr.sin_port = htons(config.portNr); // port number
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
	
	printf("\nTCPServer started on port %i\n", config.portNr);
	fflush(stdout);	
	
	sin_size = sizeof(struct sockaddr_in);

	// handler na ukonceni po stisku ctrl+c
	signal(SIGINT, signalCallbackHandler);

	
	commonQ = dispatch_queue_create("cz.vutbr.fit.xsvecm07.common", NULL);
	requestCountQ = dispatch_queue_create("cz.vutbr.fit.xsvecm07.count", NULL);

	// timer which will write requests number to stdout
	timer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, commonQ);
	dispatch_source_set_timer(timer, dispatch_walltime(NULL, 0), config.reqInfoInterval*NSEC_PER_SEC, 0);
	dispatch_source_set_event_handler_f(timer, dispatchPrintStatus);
	dispatch_resume(timer);
	
	
	if(requestProcess == WHILE)
		serverMainLoop(sock, (void *) parse_request);
	if(requestProcess == SOURCE)
		serverMainSources(sock, (void *) parse_request);
	
	
    return 0;
}
