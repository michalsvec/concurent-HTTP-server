//
//  AvgHelper.cc
//  httpserver
//
//  Created by Michal Svec on 15.5.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "AvgHelper.h"
#include "TCPHelper.h"



// use in server apps
AVGHelper::AVGHelper(char * h, int p) : TCPHelper(h,p) {

}


int AVGHelper::checkFile(std::string file) {

	return 1;
}
