# Exact Cover Parallel

Parallélisation d'un solver du problème de couverture exacte.

## Répertoires

- `sequential` : programme séquentiel
- `openmp` : parallélisation avec OpenMP
- `mpi` : parallélisation avec MPI
- `hybrid` : parallélisation avec MPI et OpenMP
- `instances` : instances du problème de couverture exacte
- `benchmark` : scripts en Python pour lancer des benchmarks et visualiser les résultats, fichiers de configuration (json) et fichiers de résultats (csv, png)

## Compilation

- Compiler un seul programme : aller dans un répertoire et entrer `make`
- Compiler tous les programmes :  entrer `make` à la racine du répertoire
- Supprimer un exécutable : entrer `make clean` dans un répertoire
- Supprimer tous les exécutables : entrer `make clean` à la racine du répertoire

## Versions

Voici 4 versions du solver du problème de couverture exacte.

### Séquentielle

Répertoire : `sequential`

```
./exact_cover.out --in ../instances/bell13.ec
```

### Parallèle avec OpenMP

Répertoire : `openmp`

```
./exact_cover_omp.out --in ../instances/bell13.ec
```

### Parallèle avec MPI

Répertoire : `mpi`

```
mpirun -n 4 ./exact_cover_mpi_static.out --in ../instances/bell13.ec
```
```
mpirun -n 4 ./exact_cover_mpi_dynamic.out --in ../instances/bell13.ec
```

### Parallèle hybride MPI + OpenMP

Répertoire : `hybrid`

```
mpirun -x OMP_NUM_THREADS=2 -n 4 ./exact_cover_hybrid.out --in ../instances/bell13.ec
```

## Auteur

[Quentin Deschamps](mailto:quentindeschamps18@gmail.com)
