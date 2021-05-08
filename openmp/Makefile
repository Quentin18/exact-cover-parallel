CC = gcc
CFLAGS = -Wall -Wextra -O3
LIBS = -fopenmp
EXEC = exact_cover_omp.out exact_cover_omp_bfs.out

all: $(EXEC)

exact_cover_omp.out: exact_cover_omp.c
	$(CC) $^ -o $@ $(CFLAGS) $(LIBS)

exact_cover_omp_bfs.out: exact_cover_omp_bfs.c
	$(CC) $^ -o $@ $(CFLAGS) $(LIBS)

clean:
	rm -f $(EXEC)
