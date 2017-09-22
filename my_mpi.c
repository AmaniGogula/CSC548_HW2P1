/******************************************************************************
* FILE: my_mpi.c
* 
* Single Author info:
* agogula Amani Gogula 
*
* LAST REVISED: 09/22/2017
*
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>
#include <netdb.h>

#include "my_mpi.h"				/* Include custom MPI library */

int rank;						/* Rank of the process */
int comm_size;					/* Number of processes */
char *hostname[MAX_PROCESSORS]; /* Data structure to hold hostnames of all nodes */
int port[16];					/* DS to hold port numbers of corresponding nodes */
int socketServer;

/* Read hostname and port from sring passed from nodefile.txt */
void setHostname(char *str, int index)
{
	char delim = ' ';
	int i=0, j=0;
	char temp[32];
	char p[6];
	for(i=0; str[i] != 0 && str[i] != delim; i++)
		temp[i] = str[i];
	temp[i++]=0;
	memcpy(hostname[index],temp, sizeof(temp));
	for(;str[i]!=0 && str[i] != delim; i++)
		p[j++]=str[i];
	p[j]=0;
	port[index] = atoi(p);
}

int MPI_Init(int *argc, char ***argv)
{
	if (!argc || !argv)
		return MPI_FAILURE;
	
	if(*argc < 4)
		return MPI_FAILURE;

	/* Maximum size of each line in nodefile.txt 
	   which includes hostname and port number */
	int line_size = 32;				
	char line[32];
	int i=0;
	
	/* Get rank, comm_size filename from command line arguments */
	rank = atoi((*argv)[1]);
	comm_size = atoi((*argv)[2]);
	char *filename = (*argv)[3];
	
	FILE *fh = fopen(filename, "r");
	if(fh == NULL)
	{
		printf("File doesn't exist");
		exit(-1);
	}
	
	while(fgets(line, line_size, fh)){
		hostname[i] = (char *)malloc(32*sizeof(char));
       	setHostname(line, i);
		//printf("Hostname: %s, Port: %d\n", hostname[i], port[i]);
		i++;
	}
	fclose(fh);
	
	struct sockaddr_in address;
	
	if((socketServer = socket(AF_INET, SOCK_STREAM, 0)) == 0){
		perror("Socket failed\n");
		return MPI_FAILURE;
	}
	
    bzero((char *)&address, sizeof(address));
	/* Set required structure variables */
	address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port[rank]);
	/* Bind socket to the port number specified */
	if (bind(socketServer, (struct sockaddr *)&address,sizeof(address))<0)
    {
        perror("bind failed");
        return MPI_FAILURE;
    }
	/* Listening for clients upto max nodes */
	if (listen(socketServer, MAX_PROCESSORS) < 0)
    {
        perror("listen");
        return MPI_FAILURE;
    }
	return MPI_SUCCESS;
}

int MPI_Comm_size ( MPI_Comm comm, int *size )
{
	*size = comm_size;		/* Return size from global variable */
	return MPI_SUCCESS;
}

int MPI_Comm_rank ( MPI_Comm comm, int *my_rank )
{
	*my_rank = rank;		/* Return rank from global variable my_rank */
	return MPI_SUCCESS;
}

int MPI_Send( void *buf, int count, MPI_Datatype datatype, int dest, 
                     int tag, MPI_Comm comm )
{
	int sockfd;
	struct hostent *destination;
	struct sockaddr_in dest_address;
	
	/* Create socket for sending data */
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0))== 0){
		perror("Socket failed\n");
		return MPI_FAILURE;
	}
	
	/* Get IP addr from hostname */
	destination = gethostbyname(hostname[dest]);
	if(destination == NULL){
		printf("No such host\n");
		exit(0);
	}
	
	/* Set destination address and port number from global structure */
	bzero((char *)&dest_address, sizeof(struct sockaddr_in));
	dest_address.sin_family = AF_INET;
	bcopy((char *)destination->h_addr, (char *)&dest_address.sin_addr.s_addr, 
					destination->h_length);
	dest_address.sin_port = htons(port[dest]);
	
	/* Connect to destination socket */
	while(connect(sockfd, (struct sockaddr *)&dest_address, sizeof(dest_address)) < 0)
	{
		printf("Waiting for connection\n");
		sleep(1);
	}
	
	/* Send data to destination */
	int n = write(sockfd, (char *)buf, sizeof(char) * count);
	if (n < 0)
		printf("Error writing to socket\n");
	//printf("Sent data: %s", (char *)buf);
	
	/* Close client connection */
	close(sockfd);
	return MPI_SUCCESS;
}

int MPI_Recv( void *buf, int count, MPI_Datatype datatype, int source,
                     int tag, MPI_Comm comm, MPI_Status *status )
{
	struct sockaddr_in source_addr;
	socklen_t srclen;
	int sockfd;
	
	/* Wait for socket connect call on server socket */
	while (1) {
        if((sockfd = accept(socketServer, (struct sockaddr *)&source_addr, &srclen)) >= 0)
            break;
    }
	bzero((char *)buf, count+1);
	
	/* Read data from client socket */
	int n = read(sockfd, (char *)buf, count);
	if (n < 0)
		printf("Error reading from socket\n");
	//printf("Received: %s\n", (char *)buf);
	
	/* Close the connection */
	close(sockfd);
	return MPI_SUCCESS;
}

double MPI_Wtime()
{
    struct timeval time_td;
	/* Get current time */
    gettimeofday(&time_td, 0);
	/* Convert it to sec and send */
    return (double) (time_td.tv_sec + (1.0) * time_td.tv_usec / 1000000);
}

int MPI_Barrier(MPI_Comm comm)
{
	int source, destination;
	MPI_Status status;
	char msg[8];				/* Holds final message for synchronization */
	int tag = 1;
	
	/* Set source and destination */
	source = rank - 1;
	destination = rank + 1;
	
	if (rank == comm_size - 1)
		destination = 0;
	if (rank == 0)
		source = comm_size - 1;
	
	if(rank == 0)
	{
		/* Send initial message */
		MPI_Send("Barrier", 8, MPI_CHAR, destination, tag, MPI_COMM_WORLD);
		/*Receive echo from source set above */
		MPI_Recv(msg, 8, MPI_CHAR, source, tag, MPI_COMM_WORLD, &status);
		/* Initiate send final DONE message */
		MPI_Send("DONE", 5, MPI_CHAR, destination, tag, MPI_COMM_WORLD);
	}
	else if(rank == comm_size-1)
	{
		/* Receive message from src and pass it to destination */
		MPI_Recv(msg, 8, MPI_CHAR, source, tag, MPI_COMM_WORLD, &status);
		MPI_Send("Barrier", 8, MPI_CHAR, destination, tag, MPI_COMM_WORLD);
		/* Since this is the last process, just receive */
		MPI_Recv(msg, 5, MPI_CHAR, source, tag, MPI_COMM_WORLD, &status);
	}
	else
	{
		/* Receive from previous node and pass it to next node as in token ring */
		MPI_Recv(msg, 8, MPI_CHAR, source, tag, MPI_COMM_WORLD, &status);
		MPI_Send("Barrier", 8, MPI_CHAR, destination, tag, MPI_COMM_WORLD);
		MPI_Recv(msg, 5, MPI_CHAR, source, tag, MPI_COMM_WORLD, &status);
		MPI_Send("DONE", 5, MPI_CHAR, destination, tag, MPI_COMM_WORLD);
 	}
	return MPI_SUCCESS;
}

int MPI_Finalize(void)
{
	/* Synchronize all tasks */
	MPI_Barrier(MPI_COMM_WORLD);
	/* Close Server */
	close(socketServer);
	return MPI_SUCCESS;
}
