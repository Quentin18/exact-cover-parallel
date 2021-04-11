CC = mpicc
CFLAGS = -Wall -Wextra -O3
EXEC = exact_cover_mpi_dynamic.out exact_cover_mpi_static.out

all: $(EXEC)

exact_cover_mpi_dynamic.out: exact_cover_mpi_dynamic.c
	$(CC) $^ -o $@ $(CFLAGS)

exact_cover_mpi_static.out: exact_cover_mpi_static.c
	$(CC) $^ -o $@ $(CFLAGS)

clean:
	rm -f $(EXEC)
