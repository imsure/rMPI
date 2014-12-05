/**
 * MPI Environment Management test.
 */

#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MSG_SIZE 1024*1024

int sr_normal(int myrank, int numtasks)
{
  int i, j;
  double *msg = (double *) malloc( MSG_SIZE * sizeof(double) );

  for ( i = 0; i < numtasks; ++i ) {
    if ( myrank == i ) { // send msg to other nodes
      for ( j = 0; j < numtasks; ++j ) {
	if ( j != i )
	  MPI_Send( msg, MSG_SIZE, MPI_DOUBLE, j, 5, MPI_COMM_WORLD );	
      }
      for ( j = 0; j < numtasks; ++j ) {
	if ( j != i ) // node i waits for msg from other nodes
	  MPI_Recv( msg, MSG_SIZE, MPI_DOUBLE, j, 5, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
      }
    } else { // other nodes wait for msg and send it back to rank i
      MPI_Recv( msg, MSG_SIZE, MPI_DOUBLE, i, 5, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
      MPI_Send( msg, MSG_SIZE, MPI_DOUBLE, i, 5, MPI_COMM_WORLD );
    }
  }
}

int sr_anysource(int myrank, int numtasks)
{
  int i, j;
  double *msg = (double *) malloc( MSG_SIZE * sizeof(double) );

  for ( i = 0; i < numtasks; ++i ) {
    if ( myrank == i ) { // wait on any source
      for ( j = 0; j < numtasks; ++j ) {
	if ( j != i )
	  MPI_Recv( msg, MSG_SIZE, MPI_DOUBLE, MPI_ANY_SOURCE, 5, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
      }
    } else { // other nodes send msg to node i
      MPI_Send( msg, MSG_SIZE, MPI_DOUBLE, i, 5, MPI_COMM_WORLD );
    }
  }
}

int main( int argc, char *argv[] )
{
  int numtasks, myrank, phy_rank;
  double start, end;

  MPI_Init( &argc, &argv ); // must be called and should be called only once
  MPI_Comm_size( MPI_COMM_WORLD, &numtasks ); // get number of MPI tasks
  MPI_Comm_rank( MPI_COMM_WORLD, &myrank ); // get MPI rank for current MPI task
  PMPI_Comm_rank( MPI_COMM_WORLD, &phy_rank ); // get MPI rank for current MPI task

  if (myrank == 0) {
    start = MPI_Wtime();
  }
  
  sr_normal( myrank, numtasks );
  sr_anysource( myrank, numtasks );

  MPI_Pcontrol( 0 ); // kill a primary node
  MPI_Barrier( MPI_COMM_WORLD );

  sr_normal( myrank, numtasks );
  sr_anysource( myrank, numtasks );

  MPI_Pcontrol( 7 ); // kill a replica node
  MPI_Barrier( MPI_COMM_WORLD );

  sr_normal( myrank, numtasks );
  sr_anysource( myrank, numtasks );

  if (myrank == 0) {
    end = MPI_Wtime();
    printf( "Rank %d: exec time = %.3lf\n", phy_rank, end-start );
  }
  
  MPI_Finalize();
}
