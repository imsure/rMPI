CC=mpicc
rMPI=rMPI.c

app: app.c $(rMPI)
	$(CC) -o app app.c $(rMPI)

clean:
	rm app