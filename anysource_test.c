/**
 * MPI Environment Management test.
 */

#include "mpi.h"
#include <stdio.h>
#include <unistd.h>

int main( int argc, char *argv[] )
{
  int numtasks, myrank, msg, counter = 0;
  MPI_Status status;

  MPI_Init( &argc, &argv ); // must be called and should be called only once
  MPI_Comm_size( MPI_COMM_WORLD, &numtasks ); // get number of MPI tasks
  MPI_Comm_rank( MPI_COMM_WORLD, &myrank ); // get MPI rank for current MPI task

  if ( myrank == 2 ) {
    msg = 200;
    MPI_Send( &msg, 1, MPI_INT, 3, 5, MPI_COMM_WORLD );
  } else if ( myrank == 1 ) {
    msg = 100;
    MPI_Send( &msg, 1, MPI_INT, 3, 5, MPI_COMM_WORLD );
  } else if ( myrank == 0 ) {
    msg = 0;
    MPI_Send( &msg, 1, MPI_INT, 3, 5, MPI_COMM_WORLD );
  } else {
    MPI_Recv( &msg, 1, MPI_INT, MPI_ANY_SOURCE, 5, MPI_COMM_WORLD, &status );
    printf( "Recv %d: Rank %d receives %d from rank %d.\n", counter++, myrank, msg, status.MPI_SOURCE );
  }

  if ( myrank == 0 )
    printf( "killing replica 5 ......\n" );
  
  MPI_Pcontrol( 5 );
  MPI_Barrier( MPI_COMM_WORLD );

  if ( myrank == 2 ) {
    msg = 200;
    MPI_Send( &msg, 1, MPI_INT, 3, 5, MPI_COMM_WORLD );
  } else if ( myrank == 1 ) {
    msg = 100;
    MPI_Send( &msg, 1, MPI_INT, 3, 5, MPI_COMM_WORLD );
  } else if ( myrank == 0 ) {
    msg = 0;
    MPI_Send( &msg, 1, MPI_INT, 3, 5, MPI_COMM_WORLD );
  } else {
    MPI_Recv( &msg, 1, MPI_INT, MPI_ANY_SOURCE, 5, MPI_COMM_WORLD, &status );
    printf( "Recv %d: Rank %d receives %d from rank %d.\n", counter++, myrank, msg, status.MPI_SOURCE );
  }

  if ( myrank == 0 )
    printf( "killing primary 0 ......\n" );

  MPI_Pcontrol( 0 );
  MPI_Barrier( MPI_COMM_WORLD );

  if ( myrank == 2 ) {
    msg = 200;
    MPI_Send( &msg, 1, MPI_INT, 3, 5, MPI_COMM_WORLD );
  } else if ( myrank == 1 ) {
    msg = 100;
    MPI_Send( &msg, 1, MPI_INT, 3, 5, MPI_COMM_WORLD );
  } else if ( myrank == 0 ) {
    msg = 0;
    MPI_Send( &msg, 1, MPI_INT, 3, 5, MPI_COMM_WORLD );
  } else {
    MPI_Recv( &msg, 1, MPI_INT, MPI_ANY_SOURCE, 5, MPI_COMM_WORLD, &status );
    printf( "Recv %d: Rank %d receives %d from rank %d.\n", counter++, myrank, msg, status.MPI_SOURCE );
  }
  
  MPI_Finalize();
}
