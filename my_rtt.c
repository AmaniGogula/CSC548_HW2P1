/******************************************************************************
* FILE: p1.c
* 
* Single Author info:
* agogula Amani Gogula 
*
* LAST REVISED: 09/01/2017
*
******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "my_mpi.h"

#define MIN_SIZE 32 		/* Minimum message size */
#define MAX_SIZE 262144	/* Maximum message size */
#define REPEAT 10			/* Repeat 10 times for each message size */

int main(int argc, char** argv)
{
	int my_rank;			/* My task number */
	int numprocs;			/* Number of tasks */
	int source, dest;		/* source and dest variables */
	int tag = 50;			/* MPI message tag */
    int count;				/* represents repeat times */
    int return_val;		
    int size;				/* current message size */

	double time[REPEAT],		/* stores time taken each time */
	       start_time, end_time, /* message loop initiating time and ending time */
	       avg_time, sum_time=0, diff_time, std_dev; /* variables for average time, std dev */

	MPI_Status status;

	/* initialize MPI */
	MPI_Init(&argc, &argv);
	
	/* get the number of procs in the comm */
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
	
	/* get my rank in the comm */
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

	/* Repeat for message sizes in doubles from 32B to 2MB */
    for(size = MIN_SIZE; size <= MAX_SIZE; size=size*2)
    {
		char in_message[size];    /* Buffer for receiving message */
        char out_message[size];   /* Buffer for out going message */

		/* Generate some random message */
		for(int c=0; c < size-1; c++) 
			out_message[c] = 'a';
		out_message[size-1]='\0';
			
		/* Repeat for "REPEAT" times */	
		for(int iteration=0; iteration < REPEAT; iteration++)
		{
//			if(my_rank == 0)
//				printf("Size: %d, Iteration: %d\n", size, iteration);
			/* Set destination for message */
			if(my_rank != (numprocs-1))
				dest = my_rank + 1;
			else
				dest = 0;

			/* set tag */
			tag=dest;
			
			/* All tasks other than "0" first receive and then send message */
			if(my_rank != 0)
			{
				/* Set source */
				source = my_rank-1;
				
				/* Receive message from task with rank 1 less than myself */
				MPI_Recv(in_message, size, MPI_CHAR, source, my_rank, MPI_COMM_WORLD, &status);
				
				/* Send message to task with rank 1 greater than myself */
				MPI_Send(out_message, strlen(out_message)+1, MPI_CHAR, dest, tag, MPI_COMM_WORLD);
			}
			/* At first task */
			else
			{
				/* Set source */
				source = numprocs-1;
				
				/* Record start time excluding first message send time */
				start_time = MPI_Wtime();
				
				/* Initiate message sending to next task */
				MPI_Send(out_message, strlen(out_message)+1, MPI_CHAR, dest, tag, MPI_COMM_WORLD);
				
				/* Receive message from last task */
				MPI_Recv(in_message, size, MPI_CHAR, source, my_rank, MPI_COMM_WORLD, &status);
				
				/* Record end time */
				end_time = MPI_Wtime();
				
				/* Calculate time taken */
				diff_time = end_time - start_time;
				
				/* Note the time for each iteration */
				time[iteration] = diff_time;
				//printf("Time taken - Size %d, Iteration %d: %8.8f\n", size, iteration, time[iteration]);
				sum_time += diff_time;
			}
		}
		
		/* Peform rtt and std_dev calculations at proc 0 */
		if(my_rank == 0)
		{
			sum_time = sum_time - time[0];
			avg_time = (double)sum_time/(REPEAT-1);
			std_dev = 0.0;
			for(int i=1;  i < REPEAT; i++)
				std_dev += pow(time[i]-avg_time, 2);
			std_dev = sqrt(std_dev/(double)(REPEAT-1));
			printf("%8d %8.8f %8.8f \n", size, avg_time, std_dev);
		}
	} 
	MPI_Finalize();	
}
