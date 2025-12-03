#ifndef __CTAR__
#define __CTAR__

#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../include/ctar_helper.h"
#include "../include/typedef.h"

extern bool is_verbose;

/**
 * @brief List all files in a tar archive.
 *
 * @param archive_path Path to the tar archive file.
 * @return 0 on success, -1 on error (errno set).
 */
int ctar_list(const char* archive_path);

/**
 * @brief Extract all files from a tar archive.
 *
 * @param archive_path Path to the tar archive file.
 * @param target_directory Directory to extract files into (NULL for current directory).
 * @return 0 on success, -1 on error (errno set).
 */
int ctar_extract(const char* archive_path, const char* target_directory);

/**
 * @brief Create a tar archive from a directory.
 *
 * @param archive_path Path where the archive will be created.
 * @param directory_path Directory to archive.
 * @return 0 on success, -1 on error (errno set).
 */
int ctar_create(const char* archive_path, const char* directory_path);

/**
 * @brief Create a directory and any necessary parent directories.
 *
 * @param directory_path Path of the directory to create.
 * @return 0 on success, -1 on error (errno set).
 */
int ctar_directory(const char* directory_path);

#endif
