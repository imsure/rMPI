/**
 * MPI Environment Management test.
 */

#include "mpi.h"
#include <stdio.h>
#include <unistd.h>

int main( int argc, char *argv[] )
{
  int numtasks, myrank, msg;

  MPI_Init( &argc, &argv ); // must be called and should be called only once
  MPI_Comm_size( MPI_COMM_WORLD, &numtasks ); // get number of MPI tasks
  MPI_Comm_rank( MPI_COMM_WORLD, &myrank ); // get MPI rank for current MPI task

  if ( myrank == 0 ) {
    msg = 99;
    MPI_Send( &msg, 1, MPI_INT, 1, 5, MPI_COMM_WORLD );
  } else {
    MPI_Recv( &msg, 1, MPI_INT, 0, 5, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
    printf( "Rank %d receives %d from rank 0.\n", myrank, msg );
  }

  //  MPI_Pcontrol( 3 ); // kill a replica
  MPI_Pcontrol( 1 ); // kill a primary rank
  //MPI_Pcontrol( 2 ); // kill a replica rank
  MPI_Barrier( MPI_COMM_WORLD );

  if ( myrank == 0 ) {
    msg = 9999;
    MPI_Send( &msg, 1, MPI_INT, 1, 5, MPI_COMM_WORLD );
  } else {
    MPI_Recv( &msg, 1, MPI_INT, 0, 5, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
    printf( "Rank %d receives %d from rank 0.\n", myrank, msg );
  }

  MPI_Pcontrol( 2 ); // kill a replica rank
  MPI_Barrier( MPI_COMM_WORLD );

  if ( myrank == 1 ) {
    msg = 1234;
    MPI_Send( &msg, 1, MPI_INT, 0, 5, MPI_COMM_WORLD );
  } else {
    MPI_Recv( &msg, 1, MPI_INT, 1, 5, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
    printf( "Rank %d receives %d from rank 1.\n", myrank, msg );
  }
   
  MPI_Finalize();
}
