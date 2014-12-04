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

  //MPI_Pcontrol( 0 ); // kill a primary node
  //MPI_Pcontrol( 1 ); // kill a primary node
  //MPI_Pcontrol( 2 ); // kill a primary node
  //MPI_Pcontrol( 7 ); // kill a primary node
  //MPI_Barrier( MPI_COMM_WORLD );
  
  MPI_Pcontrol( 1 ); // kill a replica node
  //MPI_Pcontrol( 6 ); // kill a replica node
  //MPI_Pcontrol( 7 ); // kill a replica node  
  sendrecv( myrank, numtasks );

  MPI_Pcontrol( 4 ); // kill a replica node
  MPI_Barrier( MPI_COMM_WORLD );
  
  if ( myrank == 1 ) {
    msg = 999;
    MPI_Send( &msg, 1, MPI_INT, 0, 5, MPI_COMM_WORLD );
  } else if ( myrank == 0 ) {
    MPI_Recv( &msg, 1, MPI_INT, 1, 5, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
    printf( "Rank %d receives %d from rank 1.\n", myrank, msg );
  }

  MPI_Finalize();
}
