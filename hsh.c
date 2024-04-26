#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_LINE 256

extern char** environ;

int main() {
  char command[MAX_LINE], *args[MAX_LINE/2 + 1];
  int running = 1, background = 0;
  pid_t pid;
  int pipefd[2];

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

    if (command[strlen(command) - 1] == '&') {
      background = 1;
      command[strlen(command) - 1] = '\0';
    }

    char *pipe_delim = strchr(command, '|');
    if (pipe_delim) {
      *pipe_delim = '\0'; 
      int in = dup(STDIN_FILENO);
      pipe(pipefd); 

      pid = fork();
      if (pid < 0) {
        perror("fork");
        continue;
      } else if (pid == 0) {
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[0]);
        close(in);
        execve(command, args, environ);
        fprintf(stderr, "%s: No such file or directory\n", command);
        exit(EXIT_FAILURE);
      } else {
        close(pipefd[1]);
        dup2(pipefd[0], STDIN_FILENO); 
        close(in); 

        pid = fork();
        if (pid < 0) {
          perror("fork");
          close(pipefd[0]);
          continue;
        } else if (pid == 0) {
          dup2(STDOUT_FILENO, fileno(stderr));
          execve(pipe_delim + 1, &args[strlen(command) / 2 + 1], environ);
          fprintf(stderr, "%s: No such file or directory\n", pipe_delim + 1);
          exit(EXIT_FAILURE);
        } else {
          close(pipefd[0]); 
          int status;
          waitpid(pid, &status, 0);
          waitpid(pid, &status, 0); 
        }
      }
    } else {
      pid = fork();
      if (pid < 0) {
        perror("fork");
        continue;
      } else if (pid == 0) {
        execve(command, args, environ);
        fprintf(stderr, "%s: No such file or directory\n", command);
        exit(EXIT_FAILURE);
      } else {
        if (!background) {
          waitpid(pid, &status, 0);
        }
      }
    }

    background = 0;

    args[0] = strtok(command, " ");
    int i = 1;
    while ((args[i] = strtok(NULL, " ")) != NULL) {
      i++;
    }
    args[i] = NULL; 
