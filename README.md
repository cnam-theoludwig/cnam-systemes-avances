# cnam-systemes-avances

## À propos

Code réalisé dans le cadre de la formation [Ingénieur en Informatique et Systèmes d'Information (SI), CNAM](https://www.itii-alsace.fr/formations/informatique-et-systemes-dinformation-le-cnam/), pour le module Systèmes Avancés.

- [TP N°1.1 : Appels système & fichiers : Copie de fichiers](./01-syscalls-fs-1-copy)
- [TP N°1.2 : Appels système & fichiers : Print « reverse »](./01-syscalls-fs-2-reverse)
- [TP N°1.3 : Appels système & fichiers : ls « like »](./01-syscalls-fs-3-ls)
- [TP N°2.1 : Appels système & processus : « Fork yourself »](./02-syscalls-process-1-fork)
- [TP N°2.2 : Appels système & processus : « Redirection de flux standard »](./02-syscalls-process-2-redirect)
- [TP N°2.3 : Appels système & processus : Redirection de flux via « pipe »](./02-syscalls-process-3-redirect-pipe)
- [Projet `ctar`](./projet-ctar)

### Membres du groupe

- [Djelal BOUDJI](https://github.com/djelalb)
- [Théo LUDWIG](https://gitlab.com/theoludwig)

## Prérequis

- [GNU coreutils](https://www.gnu.org/software/coreutils/)
- [GNU binutils](https://www.gnu.org/software/binutils/)
- [GNU gcc](https://gcc.gnu.org/)
- [GNU make](https://www.gnu.org/software/make/)
- [GNU gcov](https://gcc.gnu.org/onlinedocs/gcc/Gcov.html)
- [ClangFormat](https://clang.llvm.org/docs/ClangFormat.html)
- [Doxygen](https://www.doxygen.nl/)
- [LCOV Code Coverage](https://github.com/linux-test-project/lcov)
- [Docker](https://www.docker.com/)

## Utilisation

```sh
# Cloner le dépôt
git clone git@github.com:cnam-theoludwig/cnam-systemes-avances.git

# Se déplacer dans le répertoire du TP (e.g. 01-syscalls-fs-1-copy)
cd cnam-systemes-avances/01-syscalls-fs-1-copy

# Compiler et exécuter le programme
make run

# Afficher l'aide du programme
make run ARGS="--help"
```
