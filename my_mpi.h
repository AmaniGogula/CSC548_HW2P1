
#define MPI_FAILURE -1
#define MAX_PROCESSORS 16
#define MPI_SUCCESS 1
#define MPI_COMM_WORLD 1

typedef int MPI_Comm;

enum _MPI_Datatype {
	MPI_CHAR,
	MPI_INT,
	MPI_DOUBLE
};

typedef enum _MPI_Datatype MPI_Datatype;

struct _MPI_Status {
    int MPI_SOURCE;		
    int length;			
};

typedef struct _MPI_Status MPI_Status;

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
