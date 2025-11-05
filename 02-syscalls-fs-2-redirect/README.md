# 01-syscalls-fs-3-ls

## Sujet : « Redirection de flux standard »

#### Réalisez l’exercice suivant en réalisant les étapes dans l’ordre spécifié :
1. Écrivez un programme affiche qui affiche un message sur la sortie standard, dans ce message doit figurer le
premier mot passé sur la ligne de commande après le nom du programme) (argv[1]) .
On s’assurera que le bon nombre de paramètres est passé sur la ligne de commande.
2. Faites évoluer le programme de sorte à ce qu’il crée un fils qui :
    - Affiche son PID.
    - Ferme le descripteur 1 (-STDOUT) (close(1)).
    - Ouvre en création et écriture le fichier temporaire /tmp/proc-exercise (cf mkstemp)
    - Affiche le numéro du descripteur du fichier ouvert. (Possible avec la méthode dup2)
    - Exécute le programme affiché par son père lors de l’étape N°1 (argv[1]) (cf execXXX).
    - Le père quant à lui :
    - Affiche son PID.
    - Attend la fin du fils.
    - Affiche un message quelconque avant de terminer (ie : « That’s All Folks ! »).
3. Examinez le contenu de /tmp/proc-exercise, recommencer en fermant cette fois le descripteur 2.
(Laissant le descripteur 1 ouvert)
