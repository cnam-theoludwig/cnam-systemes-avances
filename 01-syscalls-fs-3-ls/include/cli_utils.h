#ifndef __CLI_UTILS__
#define __CLI_UTILS__

#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define STDOUT 1
#define STDERR 2

#define MAX_PATH_LENGTH 4096
#define BUFFER_SIZE 4096

#define USAGE_SYNTAX "-i INPUT_PATH -o OUTPUT_PATH"
#define USAGE_PARAMS \
  "OPTIONS:\n\
  -i, --input  INPUT_PATH  : input path\n\
  -o, --output OUTPUT_PATH : output path\n\
***\n\
  -v, --verbose : enable *verbose* mode\n\
  -h, --help    : display this help\n\
"

struct cli_params {
  char* input_path;
  char* output_path;
  bool is_verbose;
};

/**
 * @brief Procedure which displays binary usage by printing on stdout all available options.
 *
 * @param bin_name
 */
void cli_print_usage(char* bin_name);

/**
 * @brief Procedure checks if variable must be free (check: ptr != NULL).
 *
 * @param to_free pointer to an allocated memory.
 * @see man 3 free
 */
void free_if_needed(void* to_free);

/**
 * @see man 3 strndup
 * @see man 3 perror
 *
 * @return char*
 */
char* dup_optarg_str();

/**
 * @brief CLI main procedure.
 *
 * @param argc
 * @param argv
 */
void cli_main(int argc, char*** argv, struct cli_params* params);

#endif
