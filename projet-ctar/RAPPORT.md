# Rapport de Projet CTAR

- [Djelal BOUDJI](https://github.com/djelalb)
- [Théo LUDWIG](https://gitlab.com/theoludwig)

## Utilisation du programme

Pour les instructions détaillées de compilation et d'exécution, se référer à la section [Utilisation du fichier README.md](./README.md#utilisation).

---

## 1. Contexte, objectifs et respect du cahier des charges

Le projet consistait à développer, en C sous Linux, une version simplifiée de la commande `tar` nommée **ctar**.
Nous avons respecté les exigences techniques et fonctionnelles définies dans le cahier des charges (les points pas mentionnés n'ont pas été implémentés).

### Fonctionnalités métiers (FM)

- FM01 - Listing des éléments d'une archive (`ctar --list archive.tar`)
- FM02 - Extraction d'une archive (`ctar --extract ./archive.tar --directory ./test-tar/archive`)
- FM03 - Création d'une archive (`ctar --create ./archive.tar --directory ./test-tar`)

### Fonctionnalités métiers optionnelles (FMO)

- FMO01 – La prise en charge de la compression d’une archive tar (via la libraire zlib). `ctar --create ./archive.tar.gz --directory ./test-tar --compress`
- FMO02 – La prise en charge de la décompression d’une archive tar.gz (gzip)

### Contraintes techniques obligatoires (CT)
- CT01 - Compilation via Makefile (`make` avec `all`, `clean`, `lint`, `doc`, `coverage`)
- CT02 - Structures constantes définies dans `typedef.h` (`struct header_posix_ustar`, `USTAR_MAGIC`, etc.)
- CT03 - Séparation des prototypes et implémentations dans des fichiers `.h` (dossier `includes`) et `.c` (dossier `src`)
- CT04 - Documentation du code avec commentaires explicatifs des fonctions et structures dans les fichiers `.h` (`/** ... */`)
- CT05 - Parsing des arguments via `getopt_long()` (dans `src/cli_utils.c`)
- CT06 - Gestion des erreurs via `errno`

### Contraintes techniques optionnelles (CTO)
- CTO1 - Documentation Doxygen (`make doc` génère la documentation dans `docs/`)
- CTO2 - Couverture de code avec `gcov` (`make coverage` génère les rapports dans `gcov/`)
- CTO3 - Page de manuel Linux (`ctar.1` fournie et visible via `man --local-file ./ctar.1`)

---

## 2. Choix de conception : Architecture du projet

- **Organisation générale du projet**
  Le code est séparé en deux dossiers principaux :
  - `include/` contenant les fichiers d’en-tête (`.h`)
  - `src/` contenant les fichiers d’implémentation (`.c`)
  Cette séparation permet de distinguer clairement les interfaces des implémentations.

- **Rôle des principaux fichiers**
  - **`typedef.h`** centralise toutes les structures et constantes liées au format TAR ustar (structure de l’en-tête, taille des blocs, types de fichiers). Cela garantit une meilleure conformité au standard et évite les incohérences.
  - **`ctar.h / ctar.c`** regroupent les fonctions métier principales (`list`, `extract`, `create`) appelées directement par le `main`.
  - **`ctar_helper.h / ctar_helper.c`** contiennent les fonctions utilitaires et de bas niveau : lecture et écriture binaire, parsing des champs octaux, calcul de checksum, gestion des blocs, copie de fichiers et parcours récursif des répertoires.
  - **`cli_utils.h / cli_utils.c`** sont dédiés à l’interface en ligne de commande, notamment le parsing des arguments via `getopt_long` et la gestion des paramètres utilisateurs.
  - **`main.c`** reste volontairement minimal : il orchestre les appels aux fonctions métiers en fonction des options passées par l’utilisateur.

- **Séparation des responsabilités**
  Chaque module a un rôle précis (CLI, logique métier, bas niveau), ce qui facilite la compréhension du code, le débogage et la maintenance.

- **Robustesse et sécurité**
  Les opérations sensibles (I/O, parsing binaire, erreurs système) sont centralisées dans les helpers.
  Une vérification des chemins est effectuée lors de l’extraction afin d’éviter les problèmes de type *path traversal* (`../`, chemins absolus).

- **Mode verbose et approche incrémentale**
  Un mode `verbose` global permet d’obtenir des messages de debug sans perturber l’exécution normale.
  Le développement a suivi une approche incrémentale recommandée : parsing des arguments, lecture de l’archive, extraction, puis création d’archives.

Nous utilisons `-fsanitize=address -fsanitize=undefined` les flags de compilation gcc pour détecter les fuites mémoires et les erreurs d'accès mémoire.
Ces choix de conception visent à produire un code clair, structuré et robuste, tout en restant adapté au cadre du projet.

---

## 3. Implémentation

Le code est organisé de façon claire et modulaire dont voici une explication succincte :

* **`main.c`** : point d'entrée. Récupère les paramètres CLI via `cli_main()` et appelle la fonction métier appropriée (`ctar_list`, `ctar_extract` ou `ctar_create`). Gère le mode verbose et les erreurs d'arguments simples.

* **Headers** (dans `include/`) :

  * `typedef.h` : définit la structure `header_posix_ustar`, constantes (`TAR_BLOCK_SIZE`, `USTAR_MAGIC`, types) et tailles de buffers. C'est la référence du format utilisée par tout le code.
  * `ctar.h` : prototypes publics des fonctions métiers et l'exposition du flag `is_verbose`.
  * `cli_utils.h` : parsing des options (getopt_long), structure `cli_params` et utilitaires pour gérer l'allocation/destruction des paramètres.
  * `ctar_helper.h` : ensemble des utilitaires bas niveau pour lire/écrire les blocs, parser les champs, écrire les en-têtes, copier les fichiers, créer les répertoires, vérifier la sécurité des chemins, etc.

* **`ctar.c`** : implémentation des trois fonctions métier. On y trouve :

  * lecture itérative des en-têtes via `ctar_helper_read_and_parse_header()` ;
  * pour la liste : affichage de `parsed.full_name` et saut des blocs de données ;
  * pour l'extraction : création des répertoires nécessaires, vérification de sécurité des chemins (anti path-traversal), écriture des fichiers avec `ctar_helper_copy_exact()` et gestion du padding ;
  * pour la création : parcours récursif du répertoire (`ctar_helper_add_directory_recursive`) et écriture d'un marqueur de fin (2 blocs nuls).

---

## 4. Difficultés rencontrées et résolutions

1. **Parsing des champs ASCII-octal (taille, mode, checksum)**

   * *Problème :* champs qui contiennent des NUL ou des espaces, erreurs de conversion.
   * *Résolution :* centraliser le parsing dans `ctar_helper_parse_octal` et `ctar_helper_parse_octal_mode` avec vérification d'erreurs et retour `EINVAL` si le contenu est invalide.

2. **Calcul et vérification de la checksum d'en-tête**

   * *Problème :* la checksum doit être calculée en traitant le champ checksum comme des espaces et en sommant les octets sur 512 octets.
   * *Résolution :* implémenter `ctar_helper_compute_checksum()` et stocker à la fois la valeur déclarée et la valeur calculée dans la structure parsée. En mode verbose, afficher un warning si elles diffèrent mais continuer selon la politique choisie.

3. **Saut des blocs de données et compatibilité `lseek`**

   * *Problème :* `lseek` peut échouer si l'archive est lue depuis un flux non seekable (ex : fifo). Il faut alors lire et jeter les octets.
   * *Résolution :* tenter `lseek` et, en cas d'échec, effectuer des lectures répétées via `ctar_helper_safe_read` pour consommer exactement la quantité attendue.

4. **Sécurité lors de l'extraction (path traversal)**

   * *Problème :* entrées malicieuses dans l'archive peuvent écrire en dehors du répertoire cible.
   * *Résolution :* implémenter `ctar_helper_is_path_safe()` qui rejette chemins absolus et composants `..`; sauter ces entrées et logguer en verbose.

5. **Gestion récursive des répertoires pour la création**

   * *Problème :* construire les chemins relatifs corrects dans l'archive et respecter la limite de longueur (name + prefix).
   * *Résolution :* `ctar_helper_add_directory_recursive` calcule `archive_path` relatif au `base_path`, utilise `ctar_helper_join_path` pour scinder prefix/name si nécessaire et écrit un header via `ctar_helper_write_header()` avant d'ajouter les données.

---

## 5. Répartition du temps

| Phase         | Djelal (h) | Théo (h) |
| ------------- | ---------: | -------: |
| Conception    |          1 |        2 |
| Codage        |          2 |        4 |
| Tests         |          1 |        1 |
| Documentation |          1 |        1 |
| **Total**     |      **5** |    **8** |

> Total déclaré par le binôme : **13 h**

---

*Fin du rapport*
