/*
 *  gcd.h
 *  httpserver
 *
 *  Created by Michal Svec on 21.2.11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef __GCD_H__
#define __GCD_H__


#include <dispatch/dispatch.h>
#include <netinet/in.h>
#include "request.h"

// zpracovani pozadavku pomoci GCD 
void parse_request_gcd(reqInfo);
void parse_request_gcd_own_queue(reqInfo);

#endif
