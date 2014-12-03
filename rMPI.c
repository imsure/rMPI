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

#define NDEBUG
#undef NDEBUG

#ifdef NDEBUG
#define Debug(M, ...)
#else
#define Debug(M, ...) fprintf( stderr, "DEBUG: %s:%d: " M "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__ )
#endif

/* Tag specific */
#define RED_TAG_MASK 0x1 << 11 // tag mask for redundant messages
#define TAG_Barrier 0xff // for MPI_Barrier only
#define TAG_META 0xfe // for leader(primary) and replica to exchange meta-data

/* Global variables */
MPI_Comm primary_comm; // communicator for primary ranks
MPI_Comm replica_comm; // communicator for primary ranks
int num_ranks;
int num_phy_ranks;
int user_rank; // rank seen by users
int phy_rank; // physical rank
int *rank_states; // array for rank states, 1 is alive, 0 is dead.

/* Return whether or not the current rank is a primary rank
   or a replica rank. */
static int is_primary( int physical_rank )
{
  if ( physical_rank < num_ranks ) return 1; // primary
  else return 0; // replica
}

/* Get the replica rank of a given 'usr_rank' in the primary group. */
static int get_replica_rank( int usr_rank )
{
  return usr_rank + num_ranks;
}

/**
 * Return whether or not a given rank 'phy_rank' is alive or not.
 * 1: alive
 * 0: dead
 */
static int is_alive( int phy_rank )
{
  return rank_states[ phy_rank ];
}

static int mask_red_tag( int tag )
{
  return tag | RED_TAG_MASK;
}

/**
 * Mirror protocol --- MPI_Send
 */
static int Mirror_Send(void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm)
{
  int _wrap_py_return_val = 0;
  int dest_replica = get_replica_rank( dest );

  /* Mirror Protocol */
  if ( !is_primary(phy_rank) && is_alive(user_rank) ) {
    // only do this when the primary is alive because if not, the message
    // from replica is not redundant.
    tag = mask_red_tag(tag); 
  }
  
  if ( is_alive(dest) ) { // send to primary dest
    _wrap_py_return_val = PMPI_Send(buf, count, datatype, dest, tag, MPI_COMM_WORLD);
    //Debug( "rank %d sending to rank %d", phy_rank, dest );
  }

  if ( is_alive(dest_replica) ) { // send to replica of dest --- redundant message
    // we are setting the high-order tag bit to distinguish active message and redundant message
    _wrap_py_return_val = PMPI_Send(buf, count, datatype, dest_replica, tag, MPI_COMM_WORLD);
    //Debug( "rank %d sending to rank %d", phy_rank, dest_replica );
  }
}

/**
 * Mirror protocol --- MPI_Recv
 */
static int Mirror_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status)
{
  int _wrap_py_return_val = 0;
  int source_replica;
  int matching_src = -1; // hold the source which sent the message to the leader first

  /* Mirror Protocol */
  if ( source == MPI_ANY_SOURCE ) {
    MPI_Status tmp_status; // in case 'status' passed by caller is NULL
    Debug( "Processing MPI_ANY_SOURCE ......" );
    
    if ( is_primary(phy_rank) ) { // leader is also the primary rank
      int replica_rank = get_replica_rank( user_rank );
      // post a recv for any source. it must be from primary rank because redundant message's tag must be masked.
      _wrap_py_return_val = PMPI_Recv(buf, count, datatype, source, tag, comm, &tmp_status);
      if ( status != NULL ) {
	*status = tmp_status; // setting status if it is not null
      }
      matching_src = tmp_status.MPI_SOURCE; // get the source rank first arrived.
      Debug( "Leader %d <=== Matching sender(MPI_ANY_SOURCE) %d", phy_rank, matching_src );

      if ( is_alive(replica_rank) ) {
	// send metadata to replica of the leader
	_wrap_py_return_val = PMPI_Send( &matching_src, 1, MPI_INT, replica_rank, TAG_META, MPI_COMM_WORLD );
	Debug( "Leader rank %d ===> replica %d, matching sender of MPI_ANY_SOURCE: %d",
	       phy_rank, replica_rank, matching_src );
      }

      if ( is_primary(matching_src) ) {
	// receive from the replica of 'matching_src' if the replica is alive
	if ( is_alive(get_replica_rank(matching_src)) ) { 
	  _wrap_py_return_val = PMPI_Recv(buf, count, datatype, get_replica_rank(matching_src),
					  mask_red_tag(tag), comm, status);
	}
      }
    } else { // non-leader: replica
      // first check if the leader is alive
      if ( is_alive(user_rank) ) {
	// post a recv to get the matching sender from the leader
	_wrap_py_return_val = PMPI_Recv( &matching_src, 1, MPI_INT, user_rank, TAG_META, MPI_COMM_WORLD, status );
	Debug( "replica %d <=== leader rank %d, matching sender of MPI_ANY_SOURCE: %d",
	       phy_rank, user_rank, matching_src );
	// recv data from the matching sender
	_wrap_py_return_val = PMPI_Recv(buf, count, datatype, matching_src, tag, comm, status);

	if ( is_primary(matching_src) ) {
	  // receive from the replica of 'matching_src' if the replica is alive
	  if ( is_alive(get_replica_rank(matching_src)) ) {
	    _wrap_py_return_val = PMPI_Recv(buf, count, datatype, get_replica_rank(matching_src),
					    mask_red_tag(tag), comm, status);
	  }
	}
      } else { // leader already died, replica will do the leader's work except no meta-data needs to be sent.
	// post a recv for any source. it must be from primary rank because redundant message's tag must be masked.
	_wrap_py_return_val = PMPI_Recv(buf, count, datatype, source, tag, comm, &tmp_status);
	matching_src = tmp_status.MPI_SOURCE; // get the source rank first arrived.
	if ( status != NULL ) {
	  *status = tmp_status; // setting status if it is not null
	}

	if ( is_primary(matching_src) ) {
	  // receive from the replica of 'matching_src' if the replica is alive
	  if ( is_alive(get_replica_rank(matching_src)) ) {
	    _wrap_py_return_val = PMPI_Recv(buf, count, datatype, get_replica_rank(matching_src),
					    mask_red_tag(tag), comm, status);
	    Debug( "Matching sender %d <=== replica rank %d", matching_src, phy_rank );
	  }
	}
      }
    }
  } else { // normal case: source != MPI_ANY_SOURCE
    source_replica = get_replica_rank(source);
    if ( is_alive(source) ) { // post a recv for primary source 
      _wrap_py_return_val = PMPI_Recv(buf, count, datatype, source, tag, comm, status);
      Debug( "rank %d <=== rank %d\ttag: 0x%x", phy_rank, source, tag );
      tag = mask_red_tag( tag ); // only mask the tag when the primary rank is alive
    }
    if ( is_alive(source_replica) ) { // post a recv for replica of source
      _wrap_py_return_val = PMPI_Recv(buf, count, datatype, source_replica, tag, comm, status);
      Debug( "rank %d <=== rank %d\ttag: 0x%x", phy_rank, source_replica, mask_red_tag(tag) );
    }
  }
}

/**
 * Parallel protocol --- MPI_Send
 */
static int Parallel_Send(void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm)
{
  int _wrap_py_return_val = 0;
  int dest_replica = get_replica_rank( dest );

  if ( is_primary(phy_rank) ) {
    if ( is_alive(get_replica_rank(phy_rank)) ) { // replica partner is alive
      if ( is_alive(dest) ) {
	// primary ===> primary
	_wrap_py_return_val = PMPI_Send(buf, count, datatype, dest, tag, MPI_COMM_WORLD);
	Debug( "Parallel Protocol: rank %d ===> rank %d", phy_rank, dest );
      } else { // dest died, send a dest's replica instead
	// primary ===> replica
	_wrap_py_return_val = PMPI_Send(buf, count, datatype, dest_replica, tag, MPI_COMM_WORLD);
	Debug( "Parallel Protocol: rank %d ===> rank %d", phy_rank, dest_replica );
      }

      /* TODO: sync with replica */
      
    } else { // if replica died, degrade to mirror protocol
      	_wrap_py_return_val = Mirror_Send(buf, count, datatype, dest, tag, MPI_COMM_WORLD);
    }
  } else { // replica of the primary sender
    if ( is_alive(user_rank) ) { // primary partner is alive
      if ( is_alive(dest_replica) ) {
	// replica ===> replica
	_wrap_py_return_val = PMPI_Send(buf, count, datatype, dest_replica, tag, MPI_COMM_WORLD);
	Debug( "Parallel Protocol: rank %d ===> rank %d", phy_rank, dest_replica );
      } else { // replica of dest died, send to dest instead
	// replica ===> primary
	_wrap_py_return_val = PMPI_Send(buf, count, datatype, dest, tag, MPI_COMM_WORLD);
	Debug( "Parallel Protocol: rank %d ===> rank %d", phy_rank, dest );
      }
    } else { // if primary died, degrade to mirror protocol
      	_wrap_py_return_val = Mirror_Send(buf, count, datatype, dest, tag, MPI_COMM_WORLD);
    }
  }
    return _wrap_py_return_val;
}

/**
 * Parallel protocol --- MPI_Recv
 */
static int Parallel_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status)
{
  int _wrap_py_return_val = 0;
  int matching_src = -1; // hold the source which sent the message to the leader first
  int source_replica = get_replica_rank( source );

  if ( is_primary(phy_rank) ) { // primary rank
    if ( is_alive(get_replica_rank(phy_rank)) ) { // replica partner is alive
      if ( is_alive(source) ) {
	// primary <=== primary
	_wrap_py_return_val = PMPI_Recv( buf, count, datatype, source, tag, comm, status );
	Debug( "Parallel Protocol: rank %d <=== rank %d", phy_rank, source );
      } else {
	// primary <=== replica
	_wrap_py_return_val = PMPI_Recv( buf, count, datatype, source_replica, tag, comm, status );
	Debug( "Parallel Protocol: rank %d <=== rank %d", phy_rank, source_replica );
      }
    } else { // degrade to mirror protocol
      _wrap_py_return_val = Mirror_Recv( buf, count, datatype, source, tag, comm, status );
    }
  } else { // replica partner
    if ( is_alive(user_rank) ) { // primary partner is alive
      if ( is_alive(source_replica) ) {
	// replica <=== replica
	_wrap_py_return_val = PMPI_Recv( buf, count, datatype, source_replica, tag, comm, status );
	Debug( "Parallel Protocol: rank %d <=== rank %d", phy_rank, source_replica );
      } else {
	// primary <=== replica
	_wrap_py_return_val = PMPI_Recv( buf, count, datatype, source, tag, comm, status );
	Debug( "Parallel Protocol: rank %d <=== rank %d", phy_rank, source );
      }
    } else { // degrade to mirror protocol
      _wrap_py_return_val = Mirror_Recv( buf, count, datatype, source, tag, comm, status );
    }
  }
  return _wrap_py_return_val;
}

_EXTERN_C_ void pmpi_init(MPI_Fint *ierr);
_EXTERN_C_ void PMPI_INIT(MPI_Fint *ierr);
_EXTERN_C_ void pmpi_init_(MPI_Fint *ierr);
_EXTERN_C_ void pmpi_init__(MPI_Fint *ierr);

/* ================== C Wrappers for MPI_Init ================== */
_EXTERN_C_ int PMPI_Init(int *argc, char ***argv);
_EXTERN_C_ int MPI_Init(int *argc, char ***argv) { 
  int _wrap_py_return_val = 0, i;
    
  _wrap_py_return_val = PMPI_Init(argc, argv);

  // Get the number of physical ranks.
  PMPI_Comm_size( MPI_COMM_WORLD, &num_phy_ranks );
  if ( num_phy_ranks % 2 != 0 ) {
    fprintf( stderr, "Specified number of ranks must be even!\n " );
    MPI_Abort( MPI_COMM_WORLD, -1 );
  }
  num_ranks = num_phy_ranks >> 1; // divide by two
  PMPI_Comm_rank( MPI_COMM_WORLD, &phy_rank ); // get the physical rank
  user_rank = phy_rank < num_ranks ? phy_rank : phy_rank-num_ranks;

  //Debug( "num_user_rank=%d num_physical_rank=%d", num_ranks, num_phy_ranks );
  Debug( "user_rank=%d physical_rank=%d", user_rank, phy_rank );

  // allocate space for ranks states and initialize
  rank_states = (int *) malloc( num_phy_ranks * sizeof(int) );
  for ( i = 0; i < num_phy_ranks; ++i ) {
    rank_states[ i ] = 1; // all alive initially
  }

  return _wrap_py_return_val;
}

/* ================== C Wrappers for MPI_Send ================== */
_EXTERN_C_ int PMPI_Send(void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm);
_EXTERN_C_ int MPI_Send(void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm) { 
  int _wrap_py_return_val = 0;

  /* check aliveness */
  if ( !is_alive(phy_rank) ) {
    Debug( "rank %d has died. Return immediately", phy_rank );
    return 0;
  }

  if ( 0 ) {
    _wrap_py_return_val = Mirror_Send( buf, count, datatype, dest, tag, comm );
  } else {
    _wrap_py_return_val = Parallel_Send( buf, count, datatype, dest, tag, comm );
  }
  
  return _wrap_py_return_val;
}

/* ================== C Wrappers for MPI_Recv ================== */
_EXTERN_C_ int PMPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status);
_EXTERN_C_ int MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status) { 
  int _wrap_py_return_val;
  
  /* check aliveness */
  if ( !is_alive(phy_rank) ) {
    Debug( "rank %d has died. Return immediately", phy_rank );
    return 0;
  }

  if ( 0 ) {
    _wrap_py_return_val = Mirror_Recv( buf, count, datatype, source, tag, comm, status );
  } else {
    _wrap_py_return_val = Parallel_Recv( buf, count, datatype, source, tag, comm, status );
  }

  return _wrap_py_return_val;
}

/* ================== C Wrappers for MPI_Barrier ================== */
_EXTERN_C_ int PMPI_Barrier(MPI_Comm comm);
_EXTERN_C_ int MPI_Barrier(MPI_Comm comm) { 
  int i, msg_arrived = 1, msg_continue = 1;
  MPI_Status status;

  // We need to dissolve MPI_Barrier calls into MPI_Sends and MPI_Recvs calls.
  //int _wrap_py_return_val = PMPI_Barrier(comm);

  Debug( "Rank %d Entering barrier ..................", phy_rank );
  
  /* We implement a simple dissemination barrier where rank 0 is the coordinator.
     Rank 0 receives messages from all other ranks and then notify them so they
     can continue. */
  if ( user_rank == 0 ) { // user rank 0 --- two physical ranks 0 and N
    for ( i = 1; i < num_ranks; ++i ) {
      MPI_Recv( &msg_arrived, 1, MPI_INT, i, TAG_Barrier, comm, &status );
    }
    for ( i = 1; i < num_ranks; ++i ) {
      MPI_Send( &msg_continue, 1, MPI_INT, i, TAG_Barrier, comm );
    }
  } else {
    MPI_Send( &msg_arrived, 1, MPI_INT, 0, TAG_Barrier, comm );
    MPI_Recv( &msg_continue, 1, MPI_INT, 0, TAG_Barrier, comm, &status );
  }

  Debug( "Rank %d Leaving barrier ..................", phy_rank );
  
  return MPI_SUCCESS;
}

/* ================== C Wrappers for MPI_Comm_rank ================== */
_EXTERN_C_ int PMPI_Comm_rank(MPI_Comm comm, int *rank);
_EXTERN_C_ int MPI_Comm_rank(MPI_Comm comm, int *rank) { 
  int _wrap_py_return_val = 0;
  int numranks, cmp_res;

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

/* ================== C Wrappers for MPI_Pcontrol ================== */
_EXTERN_C_ int PMPI_Pcontrol(const int level, ...);
_EXTERN_C_ int MPI_Pcontrol(const int level, ...) {
  int ret;

  ret = PMPI_Pcontrol(level); // just a dummy call
  // make sure level is in the proper range
  if ( level < 0 || level >= num_phy_ranks ) {
    fprintf( stderr, "Cannot kill rank %d because it is a invalid rank number!\n",
	     level );
    return ret;
  }

  // update rank states
  rank_states[ level ] = 0;
  if ( phy_rank == level ) {
    Debug( "Rank %d is killed.", level );
    MPI_Finalize();
    exit( 0 );
  }

  return ret;
}

/* ================== C Wrappers for MPI_Finalize ================== */
_EXTERN_C_ int PMPI_Finalize();
_EXTERN_C_ int MPI_Finalize() { 
  int _wrap_py_return_val = 0;

  free( rank_states );
  _wrap_py_return_val = PMPI_Finalize();
  Debug( "Rank %d is finishing...", phy_rank );

  return _wrap_py_return_val;
}

