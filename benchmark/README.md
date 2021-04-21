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

- [x] `bell12_procs.json`
- [x] `bell12_threads.json`
- [x] `bell13_procs.json`
- [x] `bell13_threads.json`
- [x] `bell14_procs.json`
- [x] `bell14_threads.json`
