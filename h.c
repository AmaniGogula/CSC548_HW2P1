#include "my_mpi.h"
   #include <stdio.h>

   main(int argc, char *argv[])  {
   int numtasks, rank, dest, source, rc, count, tag=1;
   char inmsg[64], outmsg[64]="hello",inmsg2[64];
   MPI_Status Stat;   // required variable for receive routines

   MPI_Init(&argc,&argv);
   MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
   MPI_Comm_rank(MPI_COMM_WORLD, &rank);

   printf(" my rank %d \n",rank);

   // task 0 sends to task 1 and waits to receive a return message
   if (rank == 0) {
     dest = 1;
     source = 1;
     MPI_Send(outmsg, 6, MPI_CHAR, dest, tag, MPI_COMM_WORLD);
     MPI_Recv(inmsg, 6, MPI_CHAR, source, tag, MPI_COMM_WORLD, &Stat);
     MPI_Send(outmsg, 6, MPI_CHAR, dest, tag, MPI_COMM_WORLD);
          MPI_Recv(inmsg2, 6, MPI_CHAR, source, tag, MPI_COMM_WORLD, &Stat);
     }

   // task 1 waits for task 0 message then returns a message
   else if (rank == 1) {
     dest = 0;
     source = 0;
     MPI_Recv(inmsg, 6, MPI_CHAR, source, tag, MPI_COMM_WORLD, &Stat);
     MPI_Send(outmsg, 6, MPI_CHAR, dest, tag, MPI_COMM_WORLD);
     MPI_Recv(inmsg2, 6, MPI_CHAR, source, tag, MPI_COMM_WORLD, &Stat);
     MPI_Send(outmsg, 6, MPI_CHAR, dest, tag, MPI_COMM_WORLD);
     }

   // query recieve Stat variable and print message details
   //MPI_Get_count(&Stat, MPI_CHAR, &count);
   printf("Task %d: Received %s   \n",
          rank, inmsg);
   printf("Task %d: Received2 %s   \n",
             rank, inmsg2);

   MPI_Finalize();
   }
