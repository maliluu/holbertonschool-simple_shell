#include "main.h"

int main() {
  while (1) {
    prompt();

    char *input = show_input();

    if (compareExit(input, "exit") == 0) {
      free(input);
      exit(EXIT_SUCCESS);
    }

    if (compareEnv(input, "env") == 0) {
      while (*environ != NULL) {
        place(*environ);
        place("\n");
        environ++;
      }
      free(input);
      continue;
    }

    char **args = identify_string(input);

    char *command = findfile(args[0]);

    if (command == NULL) {
      place("Error: Command not found\n");
      free(args);
      free(input);
      continue;
    }

    pid_t child_pid = fork();

    if (child_pid < 0) {
      perror("Error:");
      free(args);
      free(input);
      continue;
    }

    if (child_pid == 0) {
      execve(command, args, environ);
      perror("Error:");
      exit(EXIT_FAILURE);
    } else {
      wait(NULL);
    }

    free(args);
    free(input);
    if (command != NULL) {
      free(command);
    }
  }

  return (EXIT_SUCCESS);
}
