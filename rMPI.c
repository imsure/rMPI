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

_EXTERN_C_ void pmpi_init(MPI_Fint *ierr);
_EXTERN_C_ void PMPI_INIT(MPI_Fint *ierr);
_EXTERN_C_ void pmpi_init_(MPI_Fint *ierr);
_EXTERN_C_ void pmpi_init__(MPI_Fint *ierr);

/* ================== C Wrappers for MPI_Init ================== */
_EXTERN_C_ int PMPI_Init(int *argc, char ***argv);
_EXTERN_C_ int MPI_Init(int *argc, char ***argv) { 
  int _wrap_py_return_val = 0;
  int n;
    
  _wrap_py_return_val = PMPI_Init(argc, argv);

  PMPI_Comm_size( MPI_COMM_WORLD, &n );
  if ( n % 2 != 0 ) {
    fprintf( stderr, "Specified number of ranks must be even!\n " );
    MPI_Abort( MPI_COMM_WORLD, -1 );
  }

  return _wrap_py_return_val;
}

/* ================== C Wrappers for MPI_Send ================== */
_EXTERN_C_ int PMPI_Send(void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm);
_EXTERN_C_ int MPI_Send(void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm) { 
  int _wrap_py_return_val = 0;

  _wrap_py_return_val = PMPI_Send(buf, count, datatype, dest, tag, comm);
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
  int _wrap_py_return_val = 0;

  _wrap_py_return_val = PMPI_Barrier(comm);
  return _wrap_py_return_val;
}

/* ================== C Wrappers for MPI_Comm_rank ================== */
_EXTERN_C_ int PMPI_Comm_rank(MPI_Comm comm, int *rank);
_EXTERN_C_ int MPI_Comm_rank(MPI_Comm comm, int *rank) { 
  int _wrap_py_return_val = 0;
  int numranks;

  _wrap_py_return_val = PMPI_Comm_rank(comm, rank);
  MPI_Comm_size( comm, &numranks );

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

