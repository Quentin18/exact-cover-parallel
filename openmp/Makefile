CC = gcc
CFLAGS = -Wall -Wextra -O3
LIBS = -fopenmp
SRC = exact_cover.c
EXEC = exact_cover

all: $(EXEC)

$(EXEC): $(SRC)
	$(CC) $^ -o $@ $(CFLAGS) $(LIBS)

clean:
	rm -f $(EXEC)
