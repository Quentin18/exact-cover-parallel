CC = mpicc
CFLAGS = -Wall -Wextra -O3
LIBS = -fopenmp
EXEC = exact_cover_hybrid.out

all: $(EXEC)

exact_cover_hybrid.out: exact_cover_hybrid.c
	$(CC) $^ -o $@ $(CFLAGS) $(LIBS)

clean:
	rm -f $(EXEC)
