# 01-syscalls-fs-2-reverse

## Sujet : Print « reverse »

A l'aide de la fonction `lseek` et du skeleton (cf archive `01_skeleton.tar.gz`), écrivez un programme qui affiche un fichier texte à l'envers (affichage du dernier caractère, de l'avant-dernier, etc.).

- Vous ne chercherez pas à prendre en charge les fichiers autres que textuels.
- Le nom de fichier d'entré est à passer en paramètre sur la ligne de commande selon le code existant.
- L'affichage du résultat du fichier se fera sur la sortie standard `STDOUT(1)`.

## Utilisation

```sh
echo "This is content for my_file.txt" > my_file.txt
make run ARGS="-i my_file.txt -v"
```
