#include "./cli_utils.h"
#include "../include/tar_utils.h"

int main(int argc, char** argv) {
  struct cli_params cli_params;
  cli_main(argc, &argv, &cli_params);

  if (cli_params.list_archive_file != NULL) {
    int rc = tar_list(cli_params.list_archive_file, cli_params.is_verbose);
    if (rc != 0) {
      dprintf(STDERR_FILENO, "Failed to list archive '%s'\n", cli_params.list_archive_file);
      free_if_needed(cli_params.list_archive_file);
      free_if_needed(cli_params.extract_archive_file);
      free_if_needed(cli_params.create_archive_file);
      free_if_needed(cli_params.directory_to_process);
      return EXIT_FAILURE;
    }
    free_if_needed(cli_params.list_archive_file);
    free_if_needed(cli_params.extract_archive_file);
    free_if_needed(cli_params.create_archive_file);
    free_if_needed(cli_params.directory_to_process);
    return EXIT_SUCCESS;
  }

  if (cli_params.extract_archive_file != NULL) {
    int rc = tar_extract(cli_params.extract_archive_file, cli_params.is_verbose);
    if (rc != 0) {
      dprintf(STDERR_FILENO, "Failed to extract archive '%s'\n", cli_params.extract_archive_file);
      free_if_needed(cli_params.list_archive_file);
      free_if_needed(cli_params.extract_archive_file);
      free_if_needed(cli_params.create_archive_file);
      free_if_needed(cli_params.directory_to_process);
      return EXIT_FAILURE;
    }
    free_if_needed(cli_params.list_archive_file);
    free_if_needed(cli_params.extract_archive_file);
    free_if_needed(cli_params.create_archive_file);
    free_if_needed(cli_params.directory_to_process);
    return EXIT_SUCCESS;
  }

  printf("%s\n", cli_params.is_compress ? "Compression enabled" : "No compression");

  free_if_needed(cli_params.list_archive_file);
  free_if_needed(cli_params.extract_archive_file);
  free_if_needed(cli_params.create_archive_file);
  free_if_needed(cli_params.directory_to_process);

  return EXIT_SUCCESS;
}
