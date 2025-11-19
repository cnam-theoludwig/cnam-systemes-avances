# 02-syscalls-process-3-redirect-pipe

## Sujet : Redirection de flux via « pipe »

Écrivez un programme C équivalent au script shell suivant :

```sh
ps eaux | grep "^root " > /dev/null && echo "root est connecté"
```

Votre programme devra ainsi prendre en charge les actions suivantes :

- Réaliser les exécutions de ps et grep en utilisant la primitive execlp dans l'ordre des priorités usuelles.
- Mettre en place les redirections des entrées/sorties nécessaires grâce à la primitive dup (ou dup2).
- Réaliser l'affichage final avec la primitive write.

Vous chercherez à simplifier le code pour ne prendre en charge que cette séquence de commandes.
