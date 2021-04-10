all:
	cd sequencial && make
	cd mpi && make

clean:
	cd sequencial && make clean
	cd mpi && make clean
