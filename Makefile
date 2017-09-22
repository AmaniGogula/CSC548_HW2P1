CC=gcc

CFLAGS=-g -Wall -Werror

DFLAGS=

EXECUTABLE=h



all:my_mpi.o 

	$(CC) $(CFLAGS) $(DFLAGS) h.c my_mpi.o -o $(EXECUTABLE)

my_mpi.o:my_mpi.c my_mpi.h

	$(CC) $(CFLAGS) $(DFLAGS) -c my_mpi.c

clean:

	rm -rf mympi.o h tags a.out

tags:

	ctags *


