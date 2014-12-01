/**
 * Redundant MPI.
 */

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef _EXTERN_C_
#ifdef __cplusplus
#define _EXTERN_C_ extern "C"
#else /* __cplusplus */
#define _EXTERN_C_
#endif /* __cplusplus */
#endif /* _EXTERN_C_ */

#ifdef MPICH_HAS_C2F
_EXTERN_C_ void *MPIR_ToPointer(int);
#endif // MPICH_HAS_C2F

#ifdef PIC
/* For shared libraries, declare these weak and figure out which one was linked
   based on which init wrapper was called.  See mpi_init wrappers.  */
#pragma weak pmpi_init
#pragma weak PMPI_INIT
#pragma weak pmpi_init_
#pragma weak pmpi_init__
#endif /* PIC */

#define DEBUG 1
#define TAG_Barrier 32767 // for MPI_Barrier only

/* Global variables */
MPI_Comm primary_comm; // communicator for primary ranks
MPI_Comm replica_comm; // communicator for primary ranks
int num_ranks;
int num_phy_ranks;
int user_rank; // rank seen by users
int phy_rank; // physical rank

/* Return whether or not the current rank is a primary rank
   or a replica rank. */
static int is_primary( int physical_rank )
{
  if ( physical_rank < num_ranks ) return 1; // primary
  else return 0; // replica
}

/* Get the replica rank of a given 'rank' in the primary group.  */
static int get_replica_rank( int rank )
{
  return rank + num_ranks;
}

/* Return whether or not the current rank is a primary rank
   or a replica rank. */
static int is_primary( int physical_rank )
{
  if ( physical_rank < num_ranks ) return 1; // primary
  else return 0; // replica
}

_EXTERN_C_ void pmpi_init(MPI_Fint *ierr);
_EXTERN_C_ void PMPI_INIT(MPI_Fint *ierr);
_EXTERN_C_ void pmpi_init_(MPI_Fint *ierr);
_EXTERN_C_ void pmpi_init__(MPI_Fint *ierr);

/* ================== C Wrappers for MPI_Init ================== */
_EXTERN_C_ int PMPI_Init(int *argc, char ***argv);
_EXTERN_C_ int MPI_Init(int *argc, char ***argv) { 
  int _wrap_py_return_val = 0;
  int i;
  MPI_Group world_group, primary_group, replica_group;
  int *primary_list, *replica_list;
    
  _wrap_py_return_val = PMPI_Init(argc, argv);

  // Get the number of physical ranks.
  PMPI_Comm_size( MPI_COMM_WORLD, &num_phy_ranks );
  if ( num_phy_ranks % 2 != 0 ) {
    fprintf( stderr, "Specified number of ranks must be even!\n " );
    MPI_Abort( MPI_COMM_WORLD, -1 );
  }
  num_ranks = num_phy_ranks >> 1; // divide by two
  MPI_Comm_rank( MPI_COMM_WORLD, &user_rank ); // get the rank seen by users
  PMPI_Comm_rank( MPI_COMM_WORLD, &phy_rank ); // get the physical rank

  // Get the group of MPI_COMM_WORLD
  MPI_Comm_group( MPI_COMM_WORLD, &world_group );
  // Build the rank list for the primary group.
  primary_list = (int *) malloc( num_ranks * sizeof(int) );
  for ( i = 0; i < num_ranks; ++i ) {
    primary_list[ i ] = i;
  }

  // Build the rank list for the replica group.
  replica_list = (int *) malloc( num_ranks * sizeof(int) );
  for ( i = num_ranks; i < num_phy_ranks; ++i ) {
    replica_list[ i - num_ranks ] = i;
  }

  // Create primary group and communicator
  MPI_Group_incl( world_group, num_ranks, primary_list, &primary_group );
  MPI_Comm_create( MPI_COMM_WORLD, primary_group, &primary_comm );

  // Create replica group and communicator
  MPI_Group_incl( world_group, num_ranks, replica_list, &replica_group );
  MPI_Comm_create( MPI_COMM_WORLD, replica_group, &replica_comm );

#if DEBUG
  {
    int primary_size, replica_size, primary_rank, replica_rank;
    if ( phy_rank < num_ranks ) {
      PMPI_Comm_size( primary_comm, &primary_size );
      PMPI_Comm_size( primary_comm, &primary_size );
      printf( "DEBUG: %s myrank=%d physical_rank=%d primary_size=%d\n",
	      __FUNCTION__, user_rank, phy_rank, primary_size );
    } else {
      PMPI_Comm_size( replica_comm, &replica_size );
      printf( "DEBUG: %s myrank=%d physical_rank=%d replica_size=%d\n",
	      __FUNCTION__, user_rank, phy_rank, replica_size );
    }
  }
#endif

  return _wrap_py_return_val;
}

/* ================== C Wrappers for MPI_Send ================== */
_EXTERN_C_ int PMPI_Send(void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm);
_EXTERN_C_ int MPI_Send(void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm) { 
  int _wrap_py_return_val = 0;

  /* Mirror Protocol */
  _wrap_py_return_val = PMPI_Send(buf, count, datatype, dest, tag, MPI_COMM_WORLD);
  _wrap_py_return_val = PMPI_Send(buf, count, datatype, get_replica_rank(dest), tag, MPI_COMM_WORLD);
  
  return _wrap_py_return_val;
}

/* ================== C Wrappers for MPI_Recv ================== */
_EXTERN_C_ int PMPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status);
_EXTERN_C_ int MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status) { 
  int _wrap_py_return_val = 0;

  _wrap_py_return_val = PMPI_Recv(buf, count, datatype, source, tag, comm, status);
  return _wrap_py_return_val;
}

/* ================== C Wrappers for MPI_Barrier ================== */
_EXTERN_C_ int PMPI_Barrier(MPI_Comm comm);
_EXTERN_C_ int MPI_Barrier(MPI_Comm comm) { 
  int numranks, myrank, i, msg_arrived, msg_continue = 0;

  // We need to dissolve MPI_Barrier calls into MPI_Sends and MPI_Recvs calls.
  //int _wrap_py_return_val = PMPI_Barrier(comm);

  MPI_Comm_size( comm, &numranks );
  MPI_Comm_rank( comm, &myrank );

  /* We implement a simple dissemination barrier where rank 0 is the coordinator.
     Rank 0 receives messages from all other ranks and then notify them so they
     can continue. */
  if ( myrank == 0 ) {
    for ( i = 1; i < numranks; ++i ) {
      MPI_Recv( &msg_arrived, 1, MPI_INT, MPI_ANY_SOURCE, TAG_Barrier, comm, MPI_STATUS_IGNORE );
      printf( "Rank %d arrived.\n", i );
    }
    for ( i = 1; i < numranks*2; ++i ) {
      MPI_Send( &msg_continue, 1, MPI_INT, i, TAG_Barrier, comm );
    }
  } else {
    MPI_Send( &msg_arrived, 1, MPI_INT, 0, TAG_Barrier, comm );
    MPI_Recv( &msg_continue, 1, MPI_INT, 0, TAG_Barrier, comm, MPI_STATUS_IGNORE );
    printf( "Rank %d is ready to continue.\n", myrank );
  }

  return MPI_SUCCESS;
}

/* ================== C Wrappers for MPI_Comm_rank ================== */
_EXTERN_C_ int PMPI_Comm_rank(MPI_Comm comm, int *rank);
_EXTERN_C_ int MPI_Comm_rank(MPI_Comm comm, int *rank) { 
  int _wrap_py_return_val = 0;
  int numranks, cmp_res;

  // Subtitute MPI_COMM_WORLD with communicator for primary ranks
  // such that users only see those primary ranks.
  /* MPI_Comm_compare( MPI_COMM_WORLD, comm, &cmp_res ); */
  /* if ( cmp_res == MPI_IDENT ) { */
  /*   _wrap_py_return_val = PMPI_Comm_rank( primary_comm, rank ); */
  /* } else { */
  /*   _wrap_py_return_val = PMPI_Comm_rank(comm, rank); */
  /* } */

  MPI_Comm_size( comm, &numranks );
  _wrap_py_return_val = PMPI_Comm_rank(comm, rank);
  if ( *rank >= numranks ) {
    *rank = *rank - numranks;
  }
  
  return _wrap_py_return_val;
}

/* ================== C Wrappers for MPI_Comm_size ================== */
_EXTERN_C_ int PMPI_Comm_size(MPI_Comm comm, int *size);
_EXTERN_C_ int MPI_Comm_size(MPI_Comm comm, int *size) { 
  int _wrap_py_return_val = 0;

  _wrap_py_return_val = PMPI_Comm_size(comm, size);
  *size = *size >> 1; // return size of primary ranks, remainings are replicas
  
  return _wrap_py_return_val;
}

/* ================== C Wrappers for MPI_Finalize ================== */
_EXTERN_C_ int PMPI_Finalize();
_EXTERN_C_ int MPI_Finalize() { 
  int _wrap_py_return_val = 0;

  _wrap_py_return_val = PMPI_Finalize();
  return _wrap_py_return_val;
}

