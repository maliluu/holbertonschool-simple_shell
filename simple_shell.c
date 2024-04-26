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

  // Flag to indicate if a command is piped
  int is_piped = 0;

  while (running) {
    if (!is_piped) {
      // Print prompt only if not piped
      printf("#cisfun$ ");
    }

    if (fgets(command, MAX_LINE, stdin) == NULL) {
      if (feof(stdin)) {
        printf("\n"); 
      } else {
        perror("fgets"); 
      }
      break;
    }

    // Reset piped flag for next command
    is_piped = 0;

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
      // Check for pipe using `isatty(fileno(stdin))`
      if (!isatty(fileno(stdin))) {
        is_piped = 1;  // Set piped flag if not a terminal
      }
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
