CC = mpicc
CFLAGS = -Wall -Wextra -O3
LIBS = -fopenmp
EXEC = exact_cover_hybrid_bfs.out exact_cover_hybrid_tasks.out

all: $(EXEC)

exact_cover_hybrid_bfs.out: exact_cover_hybrid_bfs.c
	$(CC) $^ -o $@ $(CFLAGS) $(LIBS)

exact_cover_hybrid_tasks.out: exact_cover_hybrid_tasks.c
	$(CC) $^ -o $@ $(CFLAGS) $(LIBS)

clean:
	rm -f $(EXEC)
