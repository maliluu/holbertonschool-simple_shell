#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_LINE 256

int main() {
  char command[MAX_LINE];
  int running = 1;

  while (running) {
    printf("#cisfun$ ");  // Display prompt

    // Read command line
    if (fgets(command, MAX_LINE, stdin) == NULL) {
      if (feof(stdin)) {
        printf("\n");  // Handle end of file (Ctrl+D)
      } else {
        perror("fgets");  // Handle other errors
      }
      break;
    }

    // Remove trailing newline
    command[strcspn(command, "\n")] = '\0';

    // Check for exit command
    if (strcmp(command, "exit") == 0) {
      running = 0;
      continue;
    }

    // Fork a child process
    pid_t pid = fork();
    if (pid < 0) {
      perror("fork");
      continue;
    } else if (pid == 0) {
      // Child process
      // Replace current process with the desired program (execve)
      if (execve(command, NULL, environ) == -1) {
        fprintf(stderr, "%s: No such file or directory\n", command);
      }
      exit(EXIT_FAILURE);  // Should never reach here if execve succeeds
    } else {
      // Parent process
      // Wait for child process to finish
      int status;
      waitpid(pid, &status, 0);
    }
  }

  return 0;
}
