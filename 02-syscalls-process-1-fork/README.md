# 02-syscalls-process-1-fork

## Sujet : « Fork yourself »

Écrivez un programme générant un processus fils avec la primitive système fork.

- Le processus fils doit afficher son numéro (PID) ainsi que celui de son père à l'aide méthodes getpid et getppid, puis sort exit avec un code de retour égal au dernier chiffre du PID.
- Le processus père, quant à lui, affiche le PID du fils, puis attend sa terminaison via wait et affiche le code de retour son fils.
