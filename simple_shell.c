#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_LINE 256

extern char** environ;

int main() {
  char command[MAX_LINE];
  int running = 1;
  pid_t pid;
  int save_in = -1;

  while (running) {
    if (!isatty(fileno(stdin))) {
      save_in = dup(STDIN_FILENO);
      if (save_in == -1) {
        perror("dup");
        break;
      }
      dup2(save_in, STDOUT_FILENO);
      dup2(fileno(stderr), STDOUT_FILENO);
    }

    printf("#cisfun$ ");

    if (save_in != -1) {
      dup2(save_in, STDIN_FILENO);
      close(save_in);
      save_in = -1;
      dup2(STDERR_FILENO, STDERR_FILENO);
    }

    if (fgets(command, MAX_LINE, stdin) == NULL) {
      if (feof(stdin)) {
        printf("\n"); 
      } else {
        perror("fgets"); 
      }
      break;
    }

    command[strcspn(command, "\n")] = '\0';

    if (strcmp(command, "exit") == 0) {
      running = 0;
      continue;
    }

    pid = fork();
    if (pid < 0) {
      perror("fork");
      continue;
    } else if (pid == 0) {
      char *args[] = {NULL};
      if (execve(command, args, environ) == -1) {
        fprintf(stderr, "%s: No such file or directory\n", command);
      }
      exit(EXIT_FAILURE);
    } else {
      int status;
      waitpid(pid, &status, 0);
    }
  }

  return 0;
}
