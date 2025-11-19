#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void) {
  int pipe_file_descriptors[2];
  pid_t process_id_ps;
  pid_t process_id_grep;
  int execution_status;

  if (pipe(pipe_file_descriptors) == -1) {
    perror("pipe");
    return EXIT_FAILURE;
  }

  process_id_ps = fork();
  if (process_id_ps == -1) {
    perror("fork");
    return EXIT_FAILURE;
  }

  if (process_id_ps == 0) {
    if (close(pipe_file_descriptors[0]) == -1) {
      perror("close");
      exit(EXIT_FAILURE);
    }

    if (dup2(pipe_file_descriptors[1], STDOUT_FILENO) == -1) {
      perror("dup2");
      exit(EXIT_FAILURE);
    }

    if (close(pipe_file_descriptors[1]) == -1) {
      perror("close");
      exit(EXIT_FAILURE);
    }

    execlp("ps", "ps", "eaux", NULL);
    perror("execlp");
    exit(EXIT_FAILURE);
  }

  process_id_grep = fork();
  if (process_id_grep == -1) {
    perror("fork");
    return EXIT_FAILURE;
  }

  if (process_id_grep == 0) {
    int null_file_descriptor;

    if (close(pipe_file_descriptors[1]) == -1) {
      perror("close");
      exit(EXIT_FAILURE);
    }

    if (dup2(pipe_file_descriptors[0], STDIN_FILENO) == -1) {
      perror("dup2");
      exit(EXIT_FAILURE);
    }

    if (close(pipe_file_descriptors[0]) == -1) {
      perror("close");
      exit(EXIT_FAILURE);
    }

    null_file_descriptor = open("/dev/null", O_WRONLY);
    if (null_file_descriptor == -1) {
      perror("open");
      exit(EXIT_FAILURE);
    }

    if (dup2(null_file_descriptor, STDOUT_FILENO) == -1) {
      perror("dup2");
      exit(EXIT_FAILURE);
    }

    if (close(null_file_descriptor) == -1) {
      perror("close");
      exit(EXIT_FAILURE);
    }

    execlp("grep", "grep", "^root ", NULL);
    perror("execlp");
    exit(EXIT_FAILURE);
  }

  if (close(pipe_file_descriptors[0]) == -1) {
    perror("close");
  }
  if (close(pipe_file_descriptors[1]) == -1) {
    perror("close");
  }

  if (waitpid(process_id_ps, NULL, 0) == -1) {
    perror("waitpid");
    return EXIT_FAILURE;
  }

  if (waitpid(process_id_grep, &execution_status, 0) == -1) {
    perror("waitpid");
    return EXIT_FAILURE;
  }

  if (WIFEXITED(execution_status)) {
    if (WEXITSTATUS(execution_status) == 0) {
      const char* message = "root est connecté\n";
      if (write(STDOUT_FILENO, message, strlen(message)) == -1) {
        perror("write");
        return EXIT_FAILURE;
      }
    }
  }

  return EXIT_SUCCESS;
}
