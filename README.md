# Exact Cover Parallel

Parallélisation d'un solver du problème de couverture exacte.

## Répertoires

- `sequential` : programme séquentiel
- `openmp` : parallélisation avec OpenMP
- `mpi` : parallélisation avec MPI
- `hybrid` : parallélisation avec MPI et OpenMP
- `checkpointing` : parallélisation avec *checkpointing*
- `instances` : instances du problème de couverture exacte
- `benchmark` : scripts en Python pour lancer des benchmarks et visualiser les
résultats, fichiers de configuration (json) et fichiers de résultats (csv, png)

## Compilation

- Compiler un seul programme : aller dans un répertoire et entrer `make`
- Compiler tous les programmes :  entrer `make` à la racine du répertoire
- Supprimer un exécutable : entrer `make clean` dans un répertoire
- Supprimer tous les exécutables : entrer `make clean` à la racine du répertoire

## Versions

Voici les différentes versions du solver du problème de couverture exacte.

### Séquentielle

- Répertoire : `sequential`
- Programme : `exact_cover.c`

```
./exact_cover.out --in ../instances/bell13.ec
```

### Parallèle avec OpenMP

- Répertoire : `openmp`
- Programmes :
  - `exact_cover_omp_bfs.c`
  - `exact_cover_omp_tasks.c`

```
./exact_cover_omp_bfs.out --in ../instances/bell13.ec
```

### Parallèle avec MPI

- Répertoire : `mpi`
- Programmes :
  - `exact_cover_mpi_bfs.c`
  - `exact_cover_mpi_dynamic.c`
  - `exact_cover_mpi_static.c`

En local (pour tester) :
```
mpirun -n 4 ./exact_cover_mpi_bfs.out --in ../instances/bell13.ec
```
Sur Grid5000 (avec un processus par coeur) :
```
mpirun --map-by ppr:1:core --hostfile $OAR_NODEFILE ./exact_cover_mpi_bfs.out --in ../instances/bell13.ec
```

### Parallèle hybride MPI + OpenMP

- Répertoire : `hybrid`
- Programmes :
  - `exact_cover_hybrid_bfs.c`
  - `exact_cover_hybrid_tasks.c`

En local (pour tester) :
```
mpirun -x OMP_NUM_THREADS=2 -n 4 ./exact_cover_hybrid_bfs.out --in ../instances/bell13.ec
```
Sur Grid5000 (avec un processus par noeud) :
```
mpirun --map-by ppr:1:node --hostfile $OAR_NODEFILE ./exact_cover_hybrid_bfs.out --in ../instances/bell13.ec
```

### Parallèle avec checkpointing

- Répertoire : `checkpointing`
- Programmes :
  - `exact_cover_hybrid_cp.c`
  - `exact_cover_mpi_cp.c`

## Auteur

[Quentin Deschamps](mailto:quentindeschamps18@gmail.com)
