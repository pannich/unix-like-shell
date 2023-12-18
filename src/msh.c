#include <stdio.h>
#include <unistd.h>
#define _GNU_SOURCE
#include <stdlib.h>
#include "shell.h"
#include "history.h"
#include <string.h>

int main(int argc, char *argv[]){
  int j;
  int max_jobs = 0;
  int max_history = 0;
  int max_line = 0;

  int opt;

  while((opt = getopt(argc, argv, "s:j:l:")) != -1) {
    // getopt returns ':', indicating a missing argument.

    switch(opt)  {
      int number;

      case 's':
          if (sscanf(optarg, "%d", &number) != 1 || number <= 0) { //scane for int in the argument passed by the options sjl
          printf("usage: msh [-s NUMBER] [-j NUMBER] [-l NUMBER]\n");
          return 1;
          }
          max_history = number;
          break;
      case 'j':
          if (sscanf(optarg, "%d", &number) != 1 || number <= 0) { //scane for int in the argument passed by the options sjl
          printf("usage: msh [-s NUMBER] [-j NUMBER] [-l NUMBER]\n");
          return 1;
          }
          max_jobs = number;
          break;
      case 'l':
          if (sscanf(optarg, "%d", &number) != 1 || number <= 0) { //scane for int in the argument passed by the options sjl
          printf("usage: msh [-s NUMBER] [-j NUMBER] [-l NUMBER]\n");
          return 1;
          }
          max_line= number;
          break;
      case '?':
        printf("usage: msh [-s NUMBER] [-j NUMBER] [-l NUMBER]\n");
      return 1;
      }
    }

  if (optind < argc) {
        printf("usage: msh [-s NUMBER] [-j NUMBER] [-l NUMBER]\n");
        return 1;
    }

  shell = alloc_shell(max_jobs, max_line, max_history); //make global var
  initialize_jobs(shell->jobs, shell->MAX_JOBS);

  char *line = NULL;
  long int len = 0;

  printf("msh> ");
  long nRead = getline(&line, &len, stdin);

  // add null terminator
  if (nRead > 0 && line[nRead - 1] == '\n') {
      // there's new line char. Replace newline character with '\0'
      line[nRead - 1] = '\0';
  } else {
      // if there's no new line character
      line[nRead] = '\0'; // Append '\0' at the end
  }

  while ( nRead != -1) {

      if (evaluate(shell, line) == 1){
        free(line);
        break;
      };

      free(line);
      //Make sure to reset line back to null for the next line
      line = NULL;

      printf("msh> ");
      nRead = getline(&line, &len, stdin);
      if (nRead > 0 && line[nRead - 1] == '\n') {
      // there's new line char. Replace newline character with '\0'
      line[nRead - 1] = '\0';
      } else {
      // if there's no new line character
      line[nRead] = '\0'; // Append '\0' at the end
      }
  }
  exit_shell(shell);
  return 0;
  }
