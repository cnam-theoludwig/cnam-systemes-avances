#include "./cli_utils.h"

int main(int argc, char** argv) {
  struct cli_params cli_params;
  cli_main(argc, &argv, &cli_params);

  int file_input = open(cli_params.input_path, O_RDONLY);
  int file_output = open(cli_params.output_path, O_CREAT | O_WRONLY | O_TRUNC, 0644);

  if (file_input == -1) {
    perror("Error open(file_input)");

    free(cli_params.input_path);
    free(cli_params.output_path);
    return EXIT_FAILURE;
  }
  if (file_output == -1) {
    perror("Error open(file_output)");

    close(file_input);
    free(cli_params.input_path);
    free(cli_params.output_path);
    return EXIT_FAILURE;
  }

  size_t bytes_read = 0;
  size_t bytes_write = 0;
  char buffer[BUFFER_SIZE];

  while ((bytes_read = read(file_input, buffer, BUFFER_SIZE)) > 0) {
    bytes_write = write(file_output, &buffer, bytes_read);
    if (bytes_write != bytes_read) {
      break;
    }
  }

  close(file_input);
  free(cli_params.input_path);

  close(file_output);
  free(cli_params.output_path);
  return EXIT_SUCCESS;
}
