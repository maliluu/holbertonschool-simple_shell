#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_LINE 256

int main() {
    extern char** environ;
  char command[MAX_LINE];
  int running = 1;

  while (running) {
    printf("#cisfun$ ");

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

    pid_t pid;
    pid = fork();
    if (pid < 0) {
      perror("fork");
      continue;
    } else if (pid == 0) {
      if (execve(command, NULL, environ) == -1) {
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
