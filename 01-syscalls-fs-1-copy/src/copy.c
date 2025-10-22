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

#define USAGE_SYNTAX "[OPTIONS] -i INPUT -o OUTPUT"
#define USAGE_PARAMS \
  "OPTIONS:\n\
  -i, --input  INPUT_FILE  : input file\n\
  -o, --output OUTPUT_FILE : output file\n\
***\n\
  -v, --verbose : enable *verbose* mode\n\
  -h, --help    : display this help\n\
"

/**
 * Procedure which displays binary usage
 * by printing on stdout all available options
 *
 * \return void
 */
void print_usage(char* bin_name) {
  dprintf(STDOUT, "USAGE: %s %s\n\n%s\n", bin_name, USAGE_SYNTAX, USAGE_PARAMS);
}

/**
 * Procedure checks if variable must be free
 * (check: ptr != NULL)
 *
 * \param void* to_free pointer to an allocated mem
 * \see man 3 free
 * \return void
 */
void free_if_needed(void* to_free) {
  if (to_free != NULL) {
    free(to_free);
  }
}

/**
 *
 * \see man 3 strndup
 * \see man 3 perror
 * \return
 */
char* dup_optarg_str() {
  char* str = NULL;

  if (optarg != NULL) {
    str = strndup(optarg, MAX_PATH_LENGTH);

    // Checking if ERRNO is set
    if (str == NULL) {
      perror(strerror(errno));
    }
  }

  return str;
}

/**
 * Binary options declaration
 * (must end with {0,0,0,0})
 *
 * \see man 3 getopt_long or getopt
 * \see struct option definition
 */
static struct option binary_opts[] = {
    {"help", no_argument, 0, 'h'},
    {"verbose", no_argument, 0, 'v'},
    {"input", required_argument, 0, 'i'},
    {"output", required_argument, 0, 'o'},
    {0, 0, 0, 0}};

/**
 * Binary options string
 * (linked to optionn declaration)
 *
 * \see man 3 getopt_long or getopt
 */
const char* binary_optstr = "hvi:o:";

/**
 * Binary main loop
 *
 * \return 1 if it exit successfully
 */
int main(int argc, char** argv) {
  /**
   * Binary variables
   * (could be defined in a structure)
   */
  bool is_verbose_mode = false;
  char* bin_input_param = NULL;
  char* bin_output_param = NULL;

  // Parsing options
  int opt = -1;
  int opt_idx = -1;

  while ((opt = getopt_long(argc, argv, binary_optstr, binary_opts, &opt_idx)) != -1) {
    switch (opt) {
      case 'i':
        if (optarg) {
          free_if_needed(bin_input_param);
          bin_input_param = dup_optarg_str();
        }
        break;
      case 'o':
        if (optarg) {
          free_if_needed(bin_output_param);
          bin_output_param = dup_optarg_str();
        }
        break;
      case 'v':
        is_verbose_mode = true;
        break;
      case 'h':
        print_usage(argv[0]);

        free_if_needed(bin_input_param);
        free_if_needed(bin_output_param);

        return EXIT_SUCCESS;
      default:
        break;
    }
  }

  /**
   * Checking binary requirements
   * (could defined in a separate function)
   */
  if (bin_input_param == NULL || bin_output_param == NULL) {
    dprintf(STDERR, "Bad usage! See HELP [--help|-h]\n");

    free_if_needed(bin_input_param);
    free_if_needed(bin_output_param);
    return EXIT_FAILURE;
  }

  // Printing params
  dprintf(1, "** PARAMS **\n%-8s: %s\n%-8s: %s\n%-8s: %d\n",
          "input", bin_input_param,
          "output", bin_output_param,
          "verbose", is_verbose_mode);

  int file1 = open(bin_input_param, O_RDONLY);
  int file2 = open(bin_output_param, O_WRONLY);

  if (file1 == -1 || file2 == -1) {
    dprintf(STDERR, "Error: could not open the files.\n");
    perror("Error (open)");
    free_if_needed(bin_input_param);
    free_if_needed(bin_output_param);
    return EXIT_FAILURE;
  }

  size_t bytes_read = 0;
  size_t bytes_write = 0;
  char buffer[BUFFER_SIZE];

  while ((bytes_read = read(file1, buffer, BUFFER_SIZE)) > 0) {
    bytes_write = write(file2, &buffer, bytes_read);
    if (bytes_write != bytes_read) {
      break;
    }
  }

  close(file1);
  close(file2);

  free_if_needed(bin_input_param);
  free_if_needed(bin_output_param);

  return EXIT_SUCCESS;
}
