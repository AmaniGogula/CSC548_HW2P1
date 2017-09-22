/******************************************************************************
* FILE: my_mpi.h
* 
* Single Author info:
* agogula Amani Gogula 
*
* LAST REVISED: 09/22/2017
*
******************************************************************************/

/* Define some constants */
#define MPI_FAILURE -1			/* Failure return code */
#define MAX_PROCESSORS 16		/* Supports upto 16 nodes */
#define MPI_SUCCESS 0			/* Success return code */
#define MPI_COMM_WORLD 1		/* Constant used */

typedef int MPI_Comm;

enum _MPI_Datatype {			/* MPI_Datatype supports char, int, double */
	MPI_CHAR,
	MPI_INT,
	MPI_DOUBLE
};

typedef enum _MPI_Datatype MPI_Datatype;

struct _MPI_Status {			/* MPI_Status structure */
    int MPI_SOURCE;		
    int length;			
};

typedef struct _MPI_Status MPI_Status;

/* Function prototypes */
int MPI_Init(int *argc, char ***argv);
int MPI_Comm_size ( MPI_Comm comm, int *size );
int MPI_Comm_rank ( MPI_Comm comm, int *rank );
int MPI_Send( void *buf, int count, MPI_Datatype datatype, int dest,
                     int tag, MPI_Comm comm );
int MPI_Recv( void *buf, int count, MPI_Datatype datatype, int source,
                     int tag, MPI_Comm comm, MPI_Status *status );
int MPI_Finalize( void );
int MPI_Barrier( MPI_Comm comm );
double MPI_Wtime();
