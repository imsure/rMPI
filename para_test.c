/**
 * MPI Environment Management test.
 */

#include "mpi.h"
#include <stdio.h>
#include <unistd.h>

int sendrecv(int myrank, int numtasks)
{
  int i, msg;
  
  if ( myrank == 0 ) {
    for ( i = 1; i < numtasks; ++i ) {
      msg = i * 1000;
      MPI_Send( &msg, 1, MPI_INT, i, 5, MPI_COMM_WORLD );
    }
  } else {
    MPI_Recv( &msg, 1, MPI_INT, 0, 5, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
    printf( "Rank %d receives %d from rank 0.\n", myrank, msg );
  }
}

int main( int argc, char *argv[] )
{
  int numtasks, myrank, msg, i;

  MPI_Init( &argc, &argv ); // must be called and should be called only once
  MPI_Comm_size( MPI_COMM_WORLD, &numtasks ); // get number of MPI tasks
  MPI_Comm_rank( MPI_COMM_WORLD, &myrank ); // get MPI rank for current MPI task

  sendrecv( myrank, numtasks );

  MPI_Finalize();
}
