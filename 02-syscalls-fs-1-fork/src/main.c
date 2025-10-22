#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main() {
  pid_t pid;
  int status;

  pid = fork();
  if (pid < 0) {
    perror("Fork failed");
    exit(EXIT_FAILURE);
  } else if (pid == 0) {
    fprintf(stdout, "Child process : my PID is (PID: %d), Parent PID is : %d\n", getpid(), getppid());
    int exit_code = getpid() % 10;
    fprintf(stdout, "Child process : Exiting with code %d\n", exit_code);
    exit(exit_code);
  } else {
    fprintf(stdout, "Parent process (PID: %d), created Child PID: %d\n", getpid(), pid);
    wait(&status);

    if (WIFEXITED(status)) {
      int exit_status = WEXITSTATUS(status);
      fprintf(stdout, "Parent process (PID: %d), Child PID: %d exited with status: %d\n", getpid(), pid, exit_status);
    } else {
      fprintf(stdout, "Child process did not terminate normally.\n");
    }
  }

  return EXIT_SUCCESS;
}
