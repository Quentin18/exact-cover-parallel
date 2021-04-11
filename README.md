# Exact Cover Parallel

Parallélisation d'un solver du problème de couverture exacte.

## Versions

### Séquentielle

```
./exact_cover.out --in ../instances/bell13.ec
```

### Parallèle avec OpenMP

```
./exact_cover_omp.out --in ../instances/bell13.ec
```

### Parallèle avec MPI

```
mpirun -n 4 ./exact_cover_mpi_static.out --in ../instances/bell13.ec
```
```
mpirun -n 4 ./exact_cover_mpi_dynamic.out --in ../instances/bell13.ec
```

### Parallèle hybride MPI + OpenMP

```
mpirun -x OMP_NUM_THREADS=2 -n 4 ./exact_cover_hybrid.out --in ../instances/bell13.ec
```

## Auteur

Quentin Deschamps
