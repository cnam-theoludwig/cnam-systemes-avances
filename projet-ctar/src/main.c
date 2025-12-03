#include <stdio.h>
#include <stdlib.h>

#include "../include/cli_utils.h"
#include "../include/ctar.h"

int main(int argc, char** argv) {
  struct cli_params cli_params;
  cli_main(argc, &argv, &cli_params);

  is_verbose = cli_params.is_verbose;
  int result_code = 0;

  if (cli_params.list_archive_file != NULL) {
    result_code = ctar_list(cli_params.list_archive_file);
  } else if (cli_params.extract_archive_file != NULL) {
    result_code = ctar_extract(cli_params.extract_archive_file, cli_params.directory_to_process);
  } else if (cli_params.create_archive_file != NULL) {
    if (cli_params.directory_to_process == NULL) {
      dprintf(STDERR_FILENO, "Error: --create requires --directory option\n");
      cli_print_usage(argv[0]);
      result_code = -1;
    } else {
      result_code = ctar_create(cli_params.create_archive_file, cli_params.directory_to_process);
    }
  } else {
    cli_print_usage(argv[0]);
  }

  cli_free_params(&cli_params);
  return result_code;
}
