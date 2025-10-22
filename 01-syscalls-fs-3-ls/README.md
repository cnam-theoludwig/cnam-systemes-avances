# 01-syscalls-fs-3-ls

## Sujet : ls « like »

Implémentez une nouvelle version de la commande « ls » listant les méta-données des fichiers au sein d'une arboressance. A l'aide des primitives système, des structures usuelles et du skeleton (cf archive `01_skeleton.tar.gz`).

Pour simplifier le code, on ne passera ni option, ni expression régulière (RegExp)

- Vous chercherez à vous appuyer les structures suivantes : dirent / stat / passwd / group / tm ;
- Efforcez-vous à gérer au maximum les erreurs via les valeurs errno.
- L'affichage sur la sortie standard est libre, néanmoins vous devez faire apparaitre les informations suivantes :

Nom / Permissions / Propriétaire / Groupe / Taille / Date de dernière màj
Exemple :

```sh
$ ls /root/my_folder
File1 - rwxrwxrw- root : root - 4096 - 010916 @ 16h24
File2 - rwxrwx--- root : root - 0785 - 020916 @ 10h22
Myfil3 - rwx------ toto : users - 4096 - 010916 @ 16h24
[…]
```

## Utilisation

```sh
mdkir folder
echo "File1 content" > folder/File1.txt
make run ARGS="/root/my_folder"
```
