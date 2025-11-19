#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <mot>\n", argv[0]);
    return EXIT_FAILURE;
  }

  printf("Message printed : %s\n", argv[1]);
  return EXIT_SUCCESS;
}
