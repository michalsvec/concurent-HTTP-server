/*
 *  fork.h
 *  httpserver
 *
 *  Created by Michal Svec on 21.2.11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */


#ifndef __FORK_H__
#define __FORK_H__


#include <dispatch/dispatch.h>
#include <netinet/in.h>

#include "request.h"

void parse_request_fork(reqInfo);


#endif
