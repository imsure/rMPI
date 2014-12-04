CC=mpicc
rMPI=../rMPI.c

test_normal: normal_test.c $(rMPI)
	$(CC) -o test_normal normal_test.c $(rMPI)

test_anysource: anysource_test.c $(rMPI)
	$(CC) -o test_anysource anysource_test.c $(rMPI)

test_anysource2: anysource2.c $(rMPI)
	$(CC) -o test_anysource2 anysource2.c $(rMPI)

test_rb: rb-grid-mpi.c $(rMPI)
	$(CC) -o test_rb rb-grid-mpi.c $(rMPI)

clean:
	rm test_normal test_anysource test_rb test_anysource2