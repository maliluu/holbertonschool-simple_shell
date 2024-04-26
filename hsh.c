#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_LINE 256

extern char** environ;

int main() {
  char command[MAX_LINE], *args[MAX_LINE/2 + 1]; // Allow for multiple arguments
  int running = 1, background = 0;
  pid_t pid;
  int pipefd[2];  // File descriptors for the pipe

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

    command[strcspn(command, "\n")] = '\0';  // Remove trailing newline

    // Check for background execution (&)
    if (command[strlen(command) - 1] == '&') {
      background = 1;
      command[strlen(command) - 1] = '\0';  // Remove '&' from command
    }

    // Handle piped commands (|)
    char *pipe_delim = strchr(command, '|');
    if (pipe_delim) {
      *pipe_delim = '\0';  // Separate commands by null character
      int in = dup(STDIN_FILENO);  // Save original stdin
      pipe(pipefd);  // Create a pipe

      pid = fork();
      if (pid < 0) {
        perror("fork");
        continue;
      } else if (pid == 0) {
        // Child 1 (executes first command)
        dup2(pipefd[1], STDOUT_FILENO);  // Redirect stdout to pipe write end
        close(pipefd[0]);  // Close pipe read end
        close(in);  // Restore original stdin
        execve(command, args, environ);
        fprintf(stderr, "%s: No such file or directory\n", command);
        exit(EXIT_FAILURE);
      } else {
        // Parent process
        close(pipefd[1]);  // Close pipe write end
        dup2(pipefd[0], STDIN_FILENO);  // Redirect parent's stdin to pipe read end
        close(in);  // Restore original stdin

        pid = fork();
        if (pid < 0) {
          perror("fork");
          close(pipefd[0]);
          continue;
        } else if (pid == 0) {
          // Child 2 (executes second command)
          dup2(STDOUT_FILENO, fileno(stderr));  // Redirect stderr to stdout
          execve(pipe_delim + 1, &args[strlen(command) / 2 + 1], environ);
          fprintf(stderr, "%s: No such file or directory\n", pipe_delim + 1);
          exit(EXIT_FAILURE);
        } else {
          // Parent process (wait for both children)
          close(pipefd[0]);  // Close pipe read end (no longer needed)
          int status;
          waitpid(pid, &status, 0);
          waitpid(pid, &status, 0);  // Wait for both child processes
        }
      }
    } else {
      // No pipes, standard execution
      pid = fork();
      if (pid < 0) {
        perror("fork");
        continue;
      } else if (pid == 0) {
        // Child process
        execve(command, args, environ);
        fprintf(stderr, "%s: No such file or directory\n", command);
        exit(EXIT_FAILURE);
      } else {
        // Parent process
        if (!background) {
          waitpid(pid, &status, 0);  // Wait for child process
        }
      }
    }

    background = 0;  // Reset background flag for next iteration

    // Parse arguments for next command
    args[0] = strtok(command, " ");
    int i = 1;
    while ((args[i] = strtok(NULL, " ")) != NULL) {
      i++;
    }
    args[i] = NULL;  // Terminate
