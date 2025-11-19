#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <programme_a_exec> [args...]\n", argv[0]);
    return EXIT_FAILURE;
  }

  pid_t pid = fork();
  if (pid < 0) {
    perror("fork");
    exit(EXIT_FAILURE);
  }

  if (pid == 0) {
    printf("Child: PID = %d (this goes to stdout before close)\n", getpid());
    fflush(stdout);

    if (close(1) < 0 && errno != EBADF) {
      perror("close(1)");
      _exit(EXIT_FAILURE);
    }

    char template[] = "/tmp/proc-exerciseXXXXXX";
    int fd = mkstemp(template);
    if (fd < 0) {
      perror("mkstemp");
      _exit(EXIT_FAILURE);
    }
    dprintf(2, "Child: opened temp file '%s' with fd = %d\n", template, fd);

    if (dup2(fd, 1) < 0) {
      perror("dup2");
      close(fd);
      _exit(EXIT_FAILURE);
    }
    if (fd != 1) close(fd);

    dprintf(2, "Child: stdout is now redirected to file '%s' (fd 1)\n", template);

    execvp(argv[1], &argv[1]);
    perror("execvp");
    _exit(EXIT_FAILURE);
  } else {
    printf("Parent: PID = %d, created child PID = %d\n", getpid(), pid);
    int status;
    wait(&status);
    if (WIFEXITED(status)) {
      printf("Parent: child exited with status %d\n", WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
      printf("Parent: child killed by signal %d\n", WTERMSIG(status));
    } else {
      printf("Parent: child ended abnormally\n");
    }
    printf("That's All Folks !\n");
  }

  return EXIT_SUCCESS;
}
