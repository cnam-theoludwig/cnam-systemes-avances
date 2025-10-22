#include "./cli_utils.h"

int main(int argc, char** argv) {
  struct cli_params cli_params;
  cli_main(argc, &argv, &cli_params);

  printf("%s\n", cli_params.is_compress ? "Compression enabled" : "No compression");

  return EXIT_SUCCESS;
}
