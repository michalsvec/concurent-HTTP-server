#ifndef __COMMON_H__
#define __COMMON_H__


#include <string>
#include "request.h"

#define PORT_NR 35123



void printError(std::string);



/**
 * Accept connection and fill reqInfo structure
 *
 * @param int socket number
 * @param reqInfo information about requesst
 */
void acceptRequest(int, reqInfo*);



/**
 * Accepts request in infinite loop 
 *
 * @param socket number
 * @param pointer to the request processing function
 */
void serverMainLoop(int, void *);



/**
 * Accepts request with dispatch source
 *
 * @param socket number
 * @param pointer to the request processing function
 */
void serverMainSources(int sock, void *);

#endif
