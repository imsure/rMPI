/**
 * MPI Environment Management test.
 */

#include "mpi.h"
#include <stdio.h>
#include <unistd.h>

void no_killing( int myrank, int numranks )
{
  int msg, counter = 0;
  MPI_Status status;

  if ( myrank == 2 ) {
    msg = 200;
    MPI_Send( &msg, 1, MPI_INT, 3, 5, MPI_COMM_WORLD );
  } else if ( myrank == 1 ) {
    msg = 100;
    MPI_Send( &msg, 1, MPI_INT, 3, 5, MPI_COMM_WORLD );
  } else if ( myrank == 0 ) {
    msg = 0;
    sleep( 1 );
    MPI_Send( &msg, 1, MPI_INT, 3, 5, MPI_COMM_WORLD );
  } else {
    MPI_Recv( &msg, 1, MPI_INT, MPI_ANY_SOURCE, 5, MPI_COMM_WORLD, &status );
    printf( "Recv %d: Rank %d receives %d from rank %d.\n", counter++, myrank, msg, status.MPI_SOURCE );
  }
}

void killing_replica( int myrank, int numranks )
{
  int msg, counter = 0;
  MPI_Status  status;
  
  if ( myrank == 0 )
    printf( "killing replica 4,5,6,7 ......\n" );

  MPI_Pcontrol( 4 );
  MPI_Pcontrol( 5 );
  MPI_Pcontrol( 6 );
  MPI_Pcontrol( 7 );
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
}

void killing_primary( int myrank, int numranks )
{
  int msg, counter = 0;
  MPI_Status status;
  
  if ( myrank == 0 )
    printf( "killing primary 0,1,2,3 ......\n" );

  MPI_Pcontrol( 0 );
  MPI_Pcontrol( 1 );
  MPI_Pcontrol( 2 );
  MPI_Pcontrol( 3 );
  MPI_Barrier( MPI_COMM_WORLD );

  if ( myrank == 2 ) {
    msg = 200;
    MPI_Send( &msg, 1, MPI_INT, 3, 5, MPI_COMM_WORLD );
  } else if ( myrank == 1 ) {
    msg = 100;
    MPI_Send( &msg, 1, MPI_INT, 3, 5, MPI_COMM_WORLD );
  } else if ( myrank == 0 ) {
    msg = 0;
    sleep( 1 );
    MPI_Send( &msg, 1, MPI_INT, 3, 5, MPI_COMM_WORLD );
  } else {
    MPI_Recv( &msg, 1, MPI_INT, MPI_ANY_SOURCE, 5, MPI_COMM_WORLD, &status );
    printf( "Recv %d: Rank %d receives %d from rank %d.\n", counter++, myrank, msg, status.MPI_SOURCE );
  }
}

void killing_mix( int myrank, int numranks )
{
  int msg, counter = 0;
  MPI_Status status;
  
  if ( myrank == 0 )
    printf( "killing primary 0, replica 6 ......\n" );

  MPI_Pcontrol( 0 );
  MPI_Pcontrol( 6 );
  MPI_Pcontrol( 7 );
  MPI_Barrier( MPI_COMM_WORLD );

  if ( myrank == 2 ) {
    msg = 200;
    MPI_Send( &msg, 1, MPI_INT, 3, 5, MPI_COMM_WORLD );
  } else if ( myrank == 1 ) {
    msg = 100;
    MPI_Send( &msg, 1, MPI_INT, 3, 5, MPI_COMM_WORLD );
  } else if ( myrank == 0 ) {
    msg = 0;
    sleep( 1 );
    MPI_Send( &msg, 1, MPI_INT, 3, 5, MPI_COMM_WORLD );
  } else {
    MPI_Recv( &msg, 1, MPI_INT, MPI_ANY_SOURCE, 5, MPI_COMM_WORLD, &status );
    printf( "Recv %d: Rank %d receives %d from rank %d.\n", counter++, myrank, msg, status.MPI_SOURCE );
  }
}

int main( int argc, char *argv[] )
{
  int numtasks, myrank, msg, counter = 0;

  MPI_Init( &argc, &argv ); // must be called and should be called only once
  MPI_Comm_size( MPI_COMM_WORLD, &numtasks ); // get number of MPI tasks
  MPI_Comm_rank( MPI_COMM_WORLD, &myrank ); // get MPI rank for current MPI task

  no_killing( myrank, numtasks );
  //killing_replica( myrank, numtasks );
  //killing_primary( myrank, numtasks );
  //killing_mix( myrank, numtasks );

  MPI_Finalize();
}
