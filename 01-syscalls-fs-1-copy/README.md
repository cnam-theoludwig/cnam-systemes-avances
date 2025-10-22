# 01-syscalls-fs-1-copy

## Sujet : Copie de fichiers

Écrivez un programme qui recopie un fichier `f1` vers un fichier `f2` à créer, à l'aide des primitives système et du skeleton (cf archive `01_skeleton.tar.gz`) fourni.

- Vous ne chercherez pas à créer le nouveau fichier avec les permissions du fichier original.
- Les noms des fichiers sont à passer en paramètre sur la ligne de commande selon le code existant.

## Utilisation

```sh
echo "This is content for my_file.txt" > my_file.txt
make run ARGS="-i my_file.txt -o copied_file.txt -v"
```
