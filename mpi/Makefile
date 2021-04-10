CC = mpicc
CFLAGS = -Wall -Wextra -O3
SRC = exact_cover.c
EXEC = exact_cover

all: $(EXEC)

$(EXEC): $(SRC)
	$(CC) $^ -o $@ $(CFLAGS)

clean:
	rm -f $(EXEC)
