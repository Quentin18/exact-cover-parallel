# Benchmark

Ce répertoire contient des scripts en Python afin de lancer des benchmarks pour tester les différentes solutions de parallélisation sur une instance du problème. Des graphes peuvent être générés à partir de fichiers csv en utilisant `matplotlib`.

## Commandes

- Copie du répertoire vers Grid5000
```
scp -r exact-cover-parallel qdeschamps@access.grid5000.fr:nancy/exact-cover-parallel
```

- Déplacement vers Grid5000
```
ssh nancy.g5k
```

- Réservation des noeuds en mode interactif (Cluster Gros, 10 noeuds)
```
oarsub -p "cluster='gros'" -l nodes=10,walltime=1 -I
```

- Lancement d'un benchmark sur une instance en mode interactif
```
python3 benchmark_json.py [config.json]
```

- Lancement d'un benchmark sur une instance en tant que job
```
oarsub -p "cluster='gros'" -l nodes=10,walltime=00:00:30 "python3 benchmark_json.py [config.json]" --notify "mail:quentin.deschamps@etu.sorbonne-universite.fr" -O "STDOUT.txt" 
```

- Voir les jobs en liste d'attente pour mon compte
```
oarstat -u
```

- Récupération des résultats du benchmark (en dehors de Grid5000)
```
scp qdeschamps@access.grid5000.fr:nancy/exact-cover-parallel/instance.csv ./instance.csv
```

- Génération d'un graphe à partir de résultats sous forme de fichier csv
```
python3 csvgraph.py [instance.csv]
```

## Liste des benchmarks

Tous les benchmarks ont été lancés sur le cluster *gros* de Nancy.

- [x] `bell12_procs.json`
- [x] `bell12_threads.json`
- [x] `bell13_procs.json`
- [x] `bell13_threads.json`
- [x] `bell14_procs.json`
- [x] `bell14_threads.json`
- [x] `matching8_procs.json`
- [x] `matching8_threads.json`
- [x] `matching9_procs.json`
- [x] `matching9_threads.json`
- [x] `matching10_procs.json`
- [x] `matching10_threads.json`
- [x] `pento_plus_tetra_2x4x10_procs.json`
- [x] `pento_plus_tetra_2x4x10_threads.json`
- [x] `pento_plus_tetra_8x8_secondary` : 1372.6 secondes avec 3x18 coeurs
- [x] `pento_plus_tetra_8x10.json` : 13h32 avec 10x18 coeurs

## Résultats

Instance | Solutions
--- | ---
bell12 | 4 213 597
bell13 | 27 644 437
bell14 | 190 899 322
matching8 | 2 027 025
matching9 | 34 459 425
matching10 | 654 729 075
pentomino_6_10 | 9 356
pento_plus_tetra_2x4x10 | 895 536
pento_plus_tetra_8x10 | 13 544 006 752
pento_plus_tetra_8x8_secondary | 1 207 328
rmols10 | **TODO**
