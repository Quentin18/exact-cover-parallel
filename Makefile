all:
	cd sequencial && make
	cd openmp && make
	cd mpi && make

clean:
	cd sequencial && make clean
	cd openmp && make clean
	cd mpi && make clean
