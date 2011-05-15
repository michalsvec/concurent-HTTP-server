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

#include <iostream>

#include <dispatch/dispatch.h>
#include <dispatch/source.h>

#include <sys/shm.h>
#include <sys/stat.h>

#include "common.h"
#include "request.h"
#include "TCPHelper.h"
#include "AvgHelper.h"


#include "lib/configfile/configfile.h"

#include "gcd.h"
#include "pthreads.h"	
#include "fork.h"



int sock;
bool showDebug = false;	// turn off debug mode by default
std::string documentRoot = "";

ConfigVals config;

int avgSock;
AVGHelper * avg;

dispatch_queue_t commonQ;
dispatch_queue_t requestCountQ;
dispatch_source_t timer;

int requestsAccepted = 1;
int requestsResponded = 1;

ModeType parallelMode = GCD;




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
	printf("Terminating on signal: %d, closing socket:%d\n", signum, sock);
	
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
int parseArguments(int argc, const char * argv[], RequestType * rMode) {
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
				parallelMode = PTHREADS;
			else if(strcmp(argv[i+1], "FORK") == 0)
				parallelMode = FORK;	
			else if(strcmp(argv[i+1], "GCD") == 0)
				parallelMode = GCD;
			else if(strcmp(argv[i+1], "GCD_OWN") == 0)
				parallelMode = GCD_OWN;
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
	config.internalRoot = cfg.getvalue<std::string>("internal_root");
	config.portNr = cfg.getvalue<int>("port");
	config.reqInfoInterval = cfg.getvalue<int>("info_interval");
	config.useAVG = cfg.getvalue<bool>("avg_check");
	config.avgHost = cfg.getvalue<std::string>("avg_host");
	config.avgPort = cfg.getvalue<int>("avg_port");
}



int main (int argc, const char * argv[]) {

	RequestType requestProcess = WHILE;	
	TCPHelper * server;

	
	commonQ = dispatch_queue_create("cz.vutbr.fit.xsvecm07.common", NULL);
	requestCountQ = dispatch_queue_create("cz.vutbr.fit.xsvecm07.count", NULL);

	
	int argResult = parseArguments(argc, argv, &requestProcess);
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
			printf("mode: PTHREADS\n");
			break;
		case FORK:
			// avoiding zombies, when using fork()
			signal(SIGCHLD, SIG_IGN);
			parse_request = parse_request_fork;
			printf("mode: FORK\n");
			break;
		case GCD_OWN:
			parse_request = parse_request_gcd_own_queue;
			printf("mode: GCD_OWN\n");
			break;
		case GCD:
		default:
			parse_request = parse_request_gcd;
			printf("mode: GCD\n");
			break;
	}

	server = new TCPHelper();
	server->setPort(config.portNr);
	server->startServer();

	
	printf("\nTCPServer started on port %i\n", config.portNr);
	fflush(stdout);	

	// handler to stop the programm on ctrl+c
	signal(SIGINT, signalCallbackHandler);



	// initialize connection to avg Tcpd daemon
	if(config.useAVG) {
		avg = new AVGHelper();
		avg->setPort(config.avgPort);
		avg->setHost((char *) config.avgHost.c_str());

		try {
			avg->connect();
			avg->read();	// read initial messages of AVG Tcpd
		} catch (char * e) {
			printf("%s\n", e);
			return EXIT_FAILURE;
		}
		
		printf("AVG control active and running\n");
	}


	// timer which will write requests number to stdout
	timer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, commonQ);
	dispatch_source_set_timer(timer, dispatch_walltime(NULL, 0), config.reqInfoInterval * NSEC_PER_SEC, 0);
	dispatch_source_set_event_handler_f(timer, dispatchPrintStatus);
	dispatch_resume(timer);
	
	
	if(requestProcess == WHILE)
		serverMainLoop(server->getSocket(), (void *) parse_request);
	if(requestProcess == SOURCE)
		serverMainSources(server->getSocket(), (void *) parse_request);
	
	delete avg;
	delete server;
    return 0;
}
