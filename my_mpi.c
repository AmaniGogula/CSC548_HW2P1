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

#include "my_mpi.h"

int rank;
int comm_size;
char *hostname[MAX_PROCESSORS];
int port[16];
int socketServer;


int MPI_Init(int *argc, char ***argv)
{
	if (!argc || !argv)
		return MPI_FAILURE;
	
	if(*argc < 4)
		return MPI_FAILURE;
	
	rank = atoi((*argv)[1]);
	comm_size = atoi((*argv)[2]);
	char *filename = (*argv)[3];
	
	FILE *fh = fopen(filename, "r");
	if(fh == NULL)
	{
		printf("File doesn't exist");
		exit(-1);
	}
	
	int line_size = 32;
	char *line = malloc(sizeof(char)*line_size+1);
	int i=0;
	while(fgets(line, line_size, fh)){
		hostname[i] = strtok(line, " ");
		port[i] = atoi(strtok(NULL, " "));
		printf("Hostname: %s, Port: %s\n", hostname[i], port[i]);
		i++;
	}
	fclose(fh);
	
	struct sockaddr_in address;
	
	if((socketServer = socket(AF_INET, SOCK_STREAM, 0)) == 0){
		perror("Socket failed\n");
		return MPI_FAILURE;
	}
	//memset((void *)address, 0, sizeof(struct sockaddr_in));
        bzero((char *)&address, sizeof(address));
	address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port[rank]);
	if (bind(socketServer, (struct sockaddr *)&address,sizeof(address))<0)
    {
        perror("bind failed");
        return MPI_FAILURE;
    }
	if (listen(socketServer, MAX_PROCESSORS) < 0)
    {
        perror("listen");
        return MPI_FAILURE;
    }
	return MPI_SUCCESS;
}

int MPI_Comm_size ( MPI_Comm comm, int *size )
{
	*size = comm_size;
	return MPI_SUCCESS;
}

int MPI_Comm_rank ( MPI_Comm comm, int *my_rank )
{
	*my_rank = rank;
	return MPI_SUCCESS;
}

int MPI_Send( void *buf, int count, MPI_Datatype datatype, int dest, 
                     int tag, MPI_Comm comm )
{
	int sockfd;
	struct hostent *destination;
	struct sockaddr_in dest_address;
	
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0))== 0){
		perror("Socket failed\n");
		return MPI_FAILURE;
	}
	destination = gethostbyname(hostname[dest]);
	if(destination == NULL){
		printf("No such host\n");
		exit(0);
	}
	bzero((char *)&dest_address, sizeof(struct sockaddr_in));
	dest_address.sin_family = AF_INET;
	bcopy((char *)destination->h_addr, (char *)&dest_address.sin_addr.s_addr, 
					destination->h_length);
	dest_address.sin_port = htons(port[dest]);
	
	while(connect(sockfd, (struct sockaddr *)&dest_address, sizeof(dest_address)) < 0)
	{
		printf("Waiting for connection\n");
		sleep(1);
	}
	int n = write(sockfd, (char *)buf, sizeof(char) * count);
	if (n < 0)
		printf("Error writing to socket\n");
	printf("Sent data: %s", (char *)buf);
	close(sockfd);
	return MPI_SUCCESS;
}

int MPI_Recv( void *buf, int count, MPI_Datatype datatype, int source,
                     int tag, MPI_Comm comm, MPI_Status *status )
{
	struct sockaddr_in source_addr;
	socklen_t srclen;
	int sockfd;
	
	while (1) {
        if((sockfd = accept(socketServer, (struct sockaddr *)&source_addr, &srclen)) >= 0)
            break;
    }
	bzero((char *)buf, count+1);
	int n = read(sockfd, (char *)buf, count);
	if (n < 0)
		printf("Error reading from socket\n");
	printf("Received: %s\n", (char *)buf);
	close(sockfd);
	return MPI_SUCCESS;
}

double MPI_Wtime()
{
    struct timeval time_td;
    gettimeofday(&time_td, 0);
    return (double) (time_td.tv_sec + (1.0) * time_td.tv_usec / 1000000);
}

int MPI_Barrier(MPI_Comm comm)
{
	int source, destination;
	MPI_Status status;
	char msg[8];
	int tag = 1;
	
	source = rank - 1;
	destination = rank + 1;
	
	if (rank == comm_size - 1)
		destination = 0;
	if (rank == 0)
		source = comm_size - 1;
	
	if(rank == 0)
	{
		MPI_Send("Barrier", 8, MPI_CHAR, destination, tag, MPI_COMM_WORLD);
		MPI_Recv(msg, 8, MPI_CHAR, source, tag, MPI_COMM_WORLD, &status);
		printf("Received: %s, Rank: %d\n", msg, rank );
		MPI_Send("DONE", 5, MPI_CHAR, destination, tag, MPI_COMM_WORLD);
	}
	else if(rank == comm_size-1)
	{
		MPI_Recv(msg, 8, MPI_CHAR, source, tag, MPI_COMM_WORLD, &status);
		printf("Received: %s, Rank: %d \n",msg, rank);
		MPI_Send("Barrier", 8, MPI_CHAR, destination, tag, MPI_COMM_WORLD);
		MPI_Recv(msg, 5, MPI_CHAR, source, tag, MPI_COMM_WORLD, &status);
		printf("Received: %s, Rank: %d \n", msg, rank);
	}
	else
	{
		MPI_Recv(msg, 8, MPI_CHAR, source, tag, MPI_COMM_WORLD, &status);
		MPI_Send("Barrier", 8, MPI_CHAR, destination, tag, MPI_COMM_WORLD);
		printf("Received: %s, Rank: %d \n", msg,rank);
		MPI_Recv(msg, 5, MPI_CHAR, source, tag, MPI_COMM_WORLD, &status);
		MPI_Send("DONE", 5, MPI_CHAR, destination, tag, MPI_COMM_WORLD);
		printf("Received: %s, Rank: %d \n", msg, rank);
	}
	return MPI_SUCCESS;
}

int MPI_Finalize(void)
{
	MPI_Barrier(MPI_COMM_WORLD);
	close(socketServer);
	return MPI_SUCCESS;
}
