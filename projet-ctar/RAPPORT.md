# Rapport de Projet CTAR

- [Djelal BOUDJI](https://github.com/djelalb)
- [Théo LUDWIG](https://gitlab.com/theoludwig)

## Utilisation du programme

Pour les instructions détaillées de compilation et d'exécution, se référer à la section [Utilisation du fichier README.md](./README.md#utilisation).

## Choix de conception: Architecture du projet

- **`typedef.h`** : Centralise toutes les définitions de structures et constantes liées au format TAR ustar
- **`ctar.h/ctar.c`** : Implémente uniquement les 4 fonctions principales utilisées par le `main`
- **`ctar_helper.h/ctar_helper.c`** : Contient toutes les fonctions utilitaires et de bas niveau
- **`cli_utils.h/cli_utils.c`** : Gère l'interface en ligne de commande et le parsing des arguments
- **`main.c`** : Point d'entrée simple qui orchestre les appels de fonctions

Le dossier `include/` contient les fichiers `.h` et le dossier `src/` contient les fichiers `.c`.

## Temps passé sur le projet

| Phase             | Temps estimé  | Détail                                                                       |
| ----------------- | ------------- | ---------------------------------------------------------------------------- |
| **Conception**    | 2 heures      | Étude du format TAR, conception de l'architecture, définition des structures |
| **Codage**        | 6 heures      | Implémentation des fonctions et débogage                                     |
| **Tests**         | 2 heures      | Tests fonctionnels, validation avec des archives existantes                  |
| **Documentation** | 2 heures      | Commentaires du code, page de manuel, rapport                                |
| **Total**         | **12 heures** |                                                                              |

## Respect du cahier des charges

Nous avons respecté les exigences techniques et fonctionnelles définies dans le cahier des charges (les points pas mentionnés n'ont pas été implémentés).

### Fonctionnalités métiers (FM)

- FM01 - Listing des éléments d'une archive (`ctar --list archive.tar`)
- FM02 - Extraction d'une archive (`ctar --extract ./archive.tar --directory ./test-tar/archive`)
- FM03 - Création d'une archive (`ctar --create ./archive.tar --directory ./test-tar`)

### Fonctionnalités métiers optionnelles (FMO)

- FMO01 - La prise en charge de la compression d'une archive tar (via la libraire zlib). `ctar --create ./archive.tar.gz --directory ./test-tar --compress`
- FMO02 - La prise en charge de la décompression d'une archive tar.gz (gzip).

<!--
- FMO03 - La réalisation d'une interface en mode console « tui » via la librairie ncurses.
-->

### Contraintes techniques obligatoires (CT)

- CT01 - Compilation via Makefile (`make` avec `all`, `clean`, `lint`, `doc`, `coverage`)
- CT02 - Structures constantes définies dans `typedef.h` (`struct header_posix_ustar`, `USTAR_MAGIC`, etc.)
- CT03 - Séparation des prototypes et implémentations dans des fichiers `.h` (dossier `includes`) et `.c` (dossier `src`)
- CT04 - Documentation du code avec commentaires explicatifs des fonctions et structures dans les fichiers `.h` (`/** ... */`)
- CT05 - Parsing des arguments via `getopt_long()` (dans `src/cli_utils.c`)
- CT06 - Gestion des erreurs via `errno`

### 5.3 Contraintes techniques optionnelles (CTO)

- CTO1 - Documentation Doxygen (`make doc` génère la documentation dans `docs/`)
- CTO2 - Couverture de code avec `gcov` (`make coverage` génère les rapports dans `coverage/`)
- CTO3 - Page de manuel Linux (`ctar.1` fournie et visible via `man --local-file ./ctar.1`)
