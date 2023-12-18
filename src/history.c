#include "history.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *HISTORY_FILE_PATH = "../data/.msh_history";  // store the history after the shell exits

history_t *alloc_history(int max_history){
  // ASK: what to use as buffer if not max_line ??

  // allocate space for each variable
  history_t *new_history = (history_t *)malloc(sizeof(history_t)); // malloc state
  new_history->lines = (char **)malloc(sizeof(char *) * max_history); // malloc array

  new_history->max_history = max_history;
  new_history->next = 0;

  // load history from file to populate new_history->lines array
  FILE *file = fopen(HISTORY_FILE_PATH, "a+");
  if (file == NULL) {
    perror("Failed to open history file");
    // exit(EXIT_FAILURE);
    }

  char *line = NULL;
  long int len = 0;
  int nRead = getline(&line, &len, file);
  while (nRead != -1 && new_history->next < max_history){
    line[strcspn(line, "\n")] = '\0';
    new_history->lines[new_history->next] = strdup(line);
    new_history->next++;
    nRead = getline(&line, &len, file);
  }

  fclose(file);

  return new_history;
  }

void add_line_history(history_t *history, const char *cmd_line){
  // printf("add line command: %s \n", cmd_line);  // debug
  int built_in_n_history = 0;
  if (cmd_line[0] == '!' && cmd_line[1] != '\0' &&
    strspn(cmd_line + 1, "0123456789") == strlen(cmd_line + 1)){
    built_in_n_history = 1;
    }

  if (cmd_line == NULL || strlen(cmd_line) == 0 || strcmp(cmd_line, "exit") == 0 || built_in_n_history == 1){
    return;
  }

  // if history is fulled, shift left by one
  if (history->next == history->max_history) {
    free(history->lines[0]);
    for (int i=0; i < history->next - 1; i++){  // next-1 here because we shift left
      history->lines[i] = history->lines[i+1];
    }

    history->next--; // decrement next counter at the the end
  }
  // allocate new cmd at the last element
  history->lines[history->next] = strdup(cmd_line);  // use strdup because we haven't allo memo at the destination
  history->next++;
}

void print_history(history_t *history) {
    for(int i = 1; i <= history->next; i++) {
        printf("%5d\t%s\n",i,history->lines[i-1]);
    }
}

// Function to find and return a command line from history by index
char *find_line_history(history_t *history, int index) {
    // Adjust the index to zero-based for accessing the lines array
    int zero_based_index = index - 1;

    if (zero_based_index < 0 || zero_based_index >= history->next) {
      // Index out of range, return NULL
      return NULL;
    }
    // Return the command line at the specified index
    // printf("\n %s %d \n",history->lines[zero_based_index],  zero_based_index);
    return history->lines[zero_based_index];
}

void free_history(history_t *history){
  FILE *file = fopen(HISTORY_FILE_PATH, "w");
  if (file != NULL) {
    int line_count = 0;
    for (int i=0; i < history->next; i++) {
      fprintf(file, "%s\n", history->lines[i]); // write
      history->lines[i] = NULL; // Free memory for the string allocated to this line
    }
    free(history->lines);  // Free lines array
    free(history); // Free history_t object
    fclose(file);
    }
}
