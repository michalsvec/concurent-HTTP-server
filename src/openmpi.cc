/*
 *  openmpi.c
 *  httpserver
 *
 *  Created by Michal Svec on 21.2.11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "openmpi.h"
#include "request.h"
#include <stdio.h>

// OpenMPI is compiled separately with makefile
#ifdef MPI
#include <mpi.h>
#endif

//TODO: implementovat!!!!1
void parse_request_openmpi(reqInfo) {
	printf("OpenMPI");

	// OpenMPI is compiled separately with makefile
#ifdef MPI
	
//	int numprocs, rank, namelen;
//	char processor_name[MPI_MAX_PROCESSOR_NAME];
	
//	
//	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
//	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
//	MPI_Get_processor_name(processor_name, &namelen);
	
//	printf("Process %d on %s out of %d\n", rank, processor_name, numprocs);
	
//	MPI_Finalize();

#endif
	return;
}