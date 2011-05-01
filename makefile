#CC=mpicc #pro gcc
CC=mpic++ #pro g++
OMCC=$(CC) 

CFLAGS= -g -Wall -Wno-unknown-pragmas 
LIBS= -lm

PROGRAMS = httpserver-mpi 

all: $(PROGRAMS)

httpserver-mpi: src/main.cc src/common.cc src/openmpi.cc src/common.cc src/request.cc src/lib/configfile/configfile.cpp
	$(OMCC) $(CFLAGS) $(LIBS) -DMPI -o build/Release/$@ src/main.cc src/common.cc src/openmpi.cc src/pthreads.cc src/fork.cc src/gcd.cc src/request.cc src/lib/configfile/configfile.cpp

clean:
	rm -f *.o
	rm -f *~
	rm -f $(PROGRAMS)
