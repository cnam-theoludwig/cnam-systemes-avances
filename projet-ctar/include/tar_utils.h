#ifndef __TAR_UTILS_H__
#define __TAR_UTILS_H__

#include <stdbool.h>
#include <stddef.h>

/*
 * Liste le contenu d'une archive tar (ustar).
 * path : chemin de l'archive
 * verbose : si true, affiche plus d'informations
 *
 * Retour : 0 si OK, -1 en cas d'erreur (errno est positionné)
 */
int tar_list(const char *path, bool verbose);

/*
 * Extrait l'intégralité d'une archive tar (ustar) vers le répertoire courant.
 * Crée les répertoires parents si nécessaire.
 *
 * Retour : 0 si OK, -1 en cas d'erreur (errno est positionné)
 */
int tar_extract(const char *path, bool verbose);

#endif /* __TAR_UTILS_H__ */
