CC=mpicc
rMPI=../rMPI.c

all: rb-kill-both rb-kill-primary rb-kill-replica rb-all-alive

rb-all-alive: rb-all-alive.c $(rMPI)
	$(CC) -o rb-all-alive rb-all-alive.c $(rMPI)

rb-kill-primary: rb-kill-primary.c $(rMPI)
	$(CC) -o rb-kill-primary rb-kill-primary.c $(rMPI)

rb-kill-replica: rb-kill-replica.c $(rMPI)
	$(CC) -o rb-kill-replica rb-kill-replica.c $(rMPI)

rb-kill-both: rb-kill-both.c $(rMPI)
	$(CC) -o rb-kill-both rb-kill-both.c $(rMPI)

mytest: mytest.c $(rMPI)
	$(CC) -o mytest mytest.c $(rMPI)

clean:
	rm rb-kill-both rb-kill-primary rb-kill-replica rb-all-alive mytest