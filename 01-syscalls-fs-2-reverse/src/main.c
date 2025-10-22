#include "./cli_utils.h"

/**
 * @brief Reverse the characters in a string.
 *
 * NOTE: Mutates the string.
 *
 * @param string
 */
void string_reverse(char* string) {
  size_t string_length = strlen(string);
  size_t index_start = 0;
  size_t index_end = string_length - 1;
  while (index_start < index_end) {
    char temporary = string[index_start];
    string[index_start] = string[index_end];
    string[index_end] = temporary;
    index_start++;
    index_end--;
  }
}

int main(int argc, char** argv) {
  struct cli_params cli_params;
  cli_main(argc, &argv, &cli_params);

  int file_input = open(cli_params.input_path, O_RDONLY);
  if (file_input == -1) {
    perror("Error open(file_input)");
    free(cli_params.input_path);
    return EXIT_FAILURE;
  }

  off_t file_size = lseek(file_input, 0, SEEK_END);
  lseek(file_input, 0, SEEK_SET);

  if (file_size <= 0) {
    fprintf(stderr, "Error: empty or invalid file.\n");
    close(file_input);
    free(cli_params.input_path);
    return EXIT_FAILURE;
  }

  char* buffer = malloc(file_size + 1);
  if (buffer == NULL) {
    perror("Error malloc(buffer)");
    close(file_input);
    free(cli_params.input_path);
    return EXIT_FAILURE;
  }

  ssize_t bytes_read = read(file_input, buffer, file_size);
  if (bytes_read == -1) {
    perror("Error read(file_input)");
    free(buffer);
    close(file_input);
    free(cli_params.input_path);
    return EXIT_FAILURE;
  }
  buffer[bytes_read] = '\0';

  string_reverse(buffer);
  printf("%s", buffer);

  free(buffer);
  close(file_input);
  free(cli_params.input_path);

  return EXIT_SUCCESS;
}
