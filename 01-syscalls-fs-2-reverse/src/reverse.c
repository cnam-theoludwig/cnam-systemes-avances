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

#define USAGE_SYNTAX "[OPTIONS] -i INPUT"
#define USAGE_PARAMS \
  "OPTIONS:\n\
  -i, --input  INPUT_FILE  : input file\n\
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
 * Print the contents of a file in reverse order to STDOUT
 *
 * \param path Path to the input file
 * \param verbose If true, prints extra info
 * \return EXIT_SUCCESS or EXIT_FAILURE
 */
int print_file_reverse(const char* path, bool verbose) {
  int fd = open(path, O_RDONLY);
  if (fd == -1) {
    perror("open");
    return EXIT_FAILURE;
  }

  struct stat st;
  if (fstat(fd, &st) == -1) {
    perror("fstat");
    close(fd);
    return EXIT_FAILURE;
  }

  off_t filesize = st.st_size;
  if (filesize == 0) {
    if (verbose) dprintf(STDOUT, "[INFO] File is empty.\n");
    close(fd);
    return EXIT_SUCCESS;
  }

  if (verbose) {
    dprintf(STDOUT, "[INFO] Reading file '%s' (%ld bytes)\n",
            path, (long)filesize);
  }

  size_t sz = (size_t)filesize;
  char* buf = malloc(sz);
  if (buf == NULL) {
    perror("malloc");
    close(fd);
    return EXIT_FAILURE;
  }

  size_t total = 0;
  while (total < sz) {
    ssize_t rr = read(fd, buf + total, sz - total);
    if (rr == -1) {
      perror("read");
      free(buf);
      close(fd);
      return EXIT_FAILURE;
    }
    if (rr == 0) break;
    total += (size_t)rr;
  }
  close(fd);

  for (size_t i = 0, j = total - 1; i < j; i++, j--) {
    char tmp = buf[i];
    buf[i] = buf[j];
    buf[j] = tmp;
  }

  size_t wrote = 0;
  while (wrote < total) {
    ssize_t ww = write(STDOUT, buf + wrote, total - wrote);
    if (ww == -1) {
      perror("write");
      free(buf);
      return EXIT_FAILURE;
    }
    wrote += (size_t)ww;
  }

  free(buf);
  return EXIT_SUCCESS;
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
    {0, 0, 0, 0}};

/**
 * Binary options string
 * (linked to optionn declaration)
 *
 * \see man 3 getopt_long or getopt
 */
const char* binary_optstr = "hvi:";

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
      case 'v':
        is_verbose_mode = true;
        break;
      case 'h':
        print_usage(argv[0]);

        free_if_needed(bin_input_param);
        return EXIT_SUCCESS;
      default:
        break;
    }
  }

  /**
   * Checking binary requirements
   * (could defined in a separate function)
   */
  if (bin_input_param == NULL) {
    dprintf(STDERR, "Bad usage! See HELP [--help|-h]\n");

    free_if_needed(bin_input_param);
    return EXIT_FAILURE;
  }

  if (is_verbose_mode) {
    dprintf(STDOUT, "** PARAMS **\n%-8s: %s\n%-8s: %s\n",
            "input", bin_input_param,
            "verbose", is_verbose_mode ? "true" : "false");
  }

  int return_code = print_file_reverse(bin_input_param, is_verbose_mode);

  free_if_needed(bin_input_param);
  return return_code;
}
