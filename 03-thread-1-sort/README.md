# 03-thread-1-sort

## Sujet : Threaded sort

Écrivez un programme qui initialise un grand tableau d'entiers avec des valeurs aléatoires, recherche ensuite le
minimum et le maximum du tableau et affiche le résultat.

Le tableau sera déclaré en variable globale :

```c
#define SIZE (int)1e8
int tab[SIZE];
```

Le programme affichera également le temps de la recherche du min et du max (temps d'initialisation non compris).
On utilisera la fonction gettimeofday.

- Proposez une version threadée de ce programme. Le résultat sera écrit dans une ou plusieurs variables globales (une seule variable si une structure est utilisée). Testez ce programme avec 2, 4, 8 threads et en augmentant la taille du tableau. Que constatez-vous ?
- Ajoutez ensuite un mutex pour protéger l'accès aux variables globales résultats.

Remarque : on utilise gettimeofday et non clock car gettimeofday rend le temps machine réel alors que clock donne
la somme des temps d'exécution des threads (principal et fils).
Remarque 2 : pour la phase de compilation ne pas oublier d'utiliser l'option `-lpthread` avec gcc.
