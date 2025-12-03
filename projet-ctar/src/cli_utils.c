#include "../include/cli_utils.h"

/**
 * @brief Binary options declaration (must end with {0, 0, 0, 0}).
 *
 * @see man 3 getopt_long or getopt
 * @see struct option definition
 */
static struct option binary_options[] = {
    {"help", no_argument, 0, 'h'},
    {"verbose", no_argument, 0, 'v'},
    {"list", required_argument, 0, 'l'},
    {"extract", required_argument, 0, 'e'},
    {"create", required_argument, 0, 'c'},
    {"directory", required_argument, 0, 'd'},
    {"compress", no_argument, 0, 'z'},
    {0, 0, 0, 0}};

void cli_print_usage(char* bin_name) {
  dprintf(STDOUT, "USAGE: %s %s\n\n%s", bin_name, USAGE_SYNTAX, USAGE_PARAMS);
}

void free_if_needed(void* to_free) {
  if (to_free != NULL) {
    free(to_free);
  }
}

char* dup_optarg_str() {
  char* string = NULL;
  if (optarg != NULL) {
    string = strndup(optarg, MAX_PATH_LENGTH);
    if (string == NULL) {
      perror(strerror(errno));
    }
  }
  return string;
}

/**
 * Binary options string (linked to option declaration).
 *
 * @see man 3 getopt_long or getopt
 */
const char* binary_optstr = "hvl:e:c:d:z";

void cli_free_params(struct cli_params* params) {
  if (params == NULL) {
    return;
  }
  free_if_needed(params->list_archive_file);
  free_if_needed(params->extract_archive_file);
  free_if_needed(params->create_archive_file);
  free_if_needed(params->directory_to_process);

  params->list_archive_file = NULL;
  params->extract_archive_file = NULL;
  params->create_archive_file = NULL;
  params->directory_to_process = NULL;
}

void cli_main(int argc, char*** argv, struct cli_params* cli_params) {
  cli_params->list_archive_file = NULL;
  cli_params->extract_archive_file = NULL;
  cli_params->create_archive_file = NULL;
  cli_params->directory_to_process = NULL;
  cli_params->is_compress = false;
  cli_params->is_verbose = false;

  int option = -1;
  int option_index = -1;

  while ((option = getopt_long(argc, *argv, binary_optstr, binary_options, &option_index)) != -1) {
    switch (option) {
      case 'l':
        cli_params->list_archive_file = dup_optarg_str();
        break;
      case 'e':
        cli_params->extract_archive_file = dup_optarg_str();
        break;
      case 'c':
        cli_params->create_archive_file = dup_optarg_str();
        break;
      case 'd':
        cli_params->directory_to_process = dup_optarg_str();
        break;
      case 'z':
        cli_params->is_compress = true;
        break;
      case 'v':
        cli_params->is_verbose = true;
        break;
      case 'h':
        cli_print_usage((*argv)[0]);
        exit(EXIT_SUCCESS);
      default:
        cli_print_usage((*argv)[0]);
        exit(EXIT_FAILURE);
    }
  }

  if (cli_params->is_verbose) {
    dprintf(STDOUT, "*** CLI PARAMS ***\n");
    dprintf(STDOUT, "%-10s: %s\n",
            "LIST",
            cli_params->list_archive_file != NULL
                ? cli_params->list_archive_file
                : "NULL");
    dprintf(STDOUT, "%-10s: %s\n",
            "EXTRACT",
            cli_params->extract_archive_file != NULL
                ? cli_params->extract_archive_file
                : "NULL");
    dprintf(STDOUT, "%-10s: %s\n",
            "CREATE",
            cli_params->create_archive_file != NULL
                ? cli_params->create_archive_file
                : "NULL");
    dprintf(STDOUT, "%-10s: %s\n",
            "DIRECTORY",
            cli_params->directory_to_process != NULL
                ? cli_params->directory_to_process
                : "NULL");
    dprintf(STDOUT, "%-10s: %s\n",
            "COMPRESS",
            cli_params->is_compress ? "true" : "false");
    dprintf(STDOUT, "%-10s: %s\n",
            "VERBOSE",
            cli_params->is_verbose ? "true" : "false");
    dprintf(STDOUT, "*** CLI PARAMS ***\n\n");
  }
}
