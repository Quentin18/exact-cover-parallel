CC = mpicc
CFLAGS = -Wall -Wextra -O3
LIBS = -fopenmp
EXEC = exact_cover_hybrid_cp.out exact_cover_mpi_cp.out

all: $(EXEC)

exact_cover_hybrid_cp.out: exact_cover_hybrid_cp.c
	$(CC) $^ -o $@ $(CFLAGS) $(LIBS)

exact_cover_mpi_cp.out: exact_cover_mpi_cp.c
	$(CC) $^ -o $@ $(CFLAGS)

clean:
	rm -f $(EXEC)
