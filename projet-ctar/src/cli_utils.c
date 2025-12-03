#include "./cli_utils.h"

/**
 * @brief Binary options declaration (must end with {0, 0, 0, 0}).
 */
static struct option binary_options[] = {
    {"help",      no_argument,       0, 'h'},
    {"verbose",   no_argument,       0, 'v'},
    {"list",      required_argument, 0, 'l'},
    {"extract",   required_argument, 0, 'e'},
    {"create",    required_argument, 0, 'c'},
    {"directory", required_argument, 0, 'd'},
    {"compress",  no_argument,       0, 'z'},
    {0, 0, 0, 0}
};

void cli_print_usage(char* bin_name) {
  dprintf(STDOUT, "USAGE: %s %s\n\n%s", bin_name, USAGE_SYNTAX, USAGE_PARAMS);
}

void free_if_needed(void* to_free) {
  if (to_free != NULL) {
    free(to_free);
  }
}

char* dup_optarg_str() {
  char* str = NULL;
  if (optarg != NULL) {
    str = strndup(optarg, MAX_PATH_LENGTH);
    if (str == NULL) {
      perror("strndup");
    }
  }
  return str;
}

/* Helper to free all allocated strings inside cli_params */
static void free_cli_params_allocated(struct cli_params* p) {
  if (p == NULL) return;
  free_if_needed(p->list_archive_file);
  free_if_needed(p->extract_archive_file);
  free_if_needed(p->create_archive_file);
  free_if_needed(p->directory_to_process);
}

/**
 * Binary options string (linked to option declaration).
 * l,e,c,d require an argument; h,v,z do not.
 */
const char* binary_optstr = "hvzl:e:c:d:";

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
        if (optarg != NULL) {
          cli_params->list_archive_file = dup_optarg_str();
        } else {
          dprintf(STDERR, "Option -l/--list requires an argument\n");
          cli_print_usage((*argv)[0]);
          free_cli_params_allocated(cli_params);
          exit(EXIT_FAILURE);
        }
        break;
      case 'e':
        if (optarg != NULL) {
          cli_params->extract_archive_file = dup_optarg_str();
        } else {
          dprintf(STDERR, "Option -e/--extract requires an argument\n");
          cli_print_usage((*argv)[0]);
          free_cli_params_allocated(cli_params);
          exit(EXIT_FAILURE);
        }
        break;
      case 'c':
        if (optarg != NULL) {
          cli_params->create_archive_file = dup_optarg_str();
        } else {
          dprintf(STDERR, "Option -c/--create requires an argument\n");
          cli_print_usage((*argv)[0]);
          free_cli_params_allocated(cli_params);
          exit(EXIT_FAILURE);
        }
        break;
      case 'd':
        if (optarg != NULL) {
          cli_params->directory_to_process = dup_optarg_str();
        } else {
          dprintf(STDERR, "Option -d/--directory requires an argument\n");
          cli_print_usage((*argv)[0]);
          free_cli_params_allocated(cli_params);
          exit(EXIT_FAILURE);
        }
        break;
      case 'z':
        cli_params->is_compress = true;
        break;
      case 'v':
        cli_params->is_verbose = true;
        break;
      case 'h':
        cli_print_usage((*argv)[0]);
        free_cli_params_allocated(cli_params);
        exit(EXIT_SUCCESS);
      case '?':
      default:
        break;
    }
  }

  if (cli_params->is_verbose) {
    dprintf(STDOUT, "** PARAMS **\n%-8s: %s\n%-8s: %s\n%-8s: %s\n",
            "LIST", cli_params->list_archive_file != NULL ? cli_params->list_archive_file : "NULL",
            "EXTRACT", cli_params->extract_archive_file != NULL ? cli_params->extract_archive_file : "NULL",
            "CREATE", cli_params->create_archive_file != NULL ? cli_params->create_archive_file : "NULL");
    dprintf(STDOUT, "%-8s: %s\n%-8s: %s\n%-8s: %s\n",
            "DIRECTORY", cli_params->directory_to_process != NULL ? cli_params->directory_to_process : "NULL",
            "COMPRESS", cli_params->is_compress ? "true" : "false",
            "VERBOSE", cli_params->is_verbose ? "true" : "false");
  }
}
