all:
	cd sequential && make
	cd openmp && make
	cd mpi && make
	cd hybrid && make
	cd checkpointing && make

clean:
	cd sequential && make clean
	cd openmp && make clean
	cd mpi && make clean
	cd hybrid && make clean
	cd checkpointing && make clean
