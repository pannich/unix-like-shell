#ifndef _HISTORY_H_
#define _HISTORY_H_

extern const char *HISTORY_FILE_PATH;

//Represents the state of the history of the shell
typedef struct history {
    char **lines;
    int max_history;
    int next;
}history_t;

/*
* alloc_history: creates a new history_t state
*
* max_history: The maximum number of saved history commands for the shell.
*
* Returns: a new history_t state
*/
history_t *alloc_history(int max_history);

/*
* add_line_history: add new history line
*
* history: history object.
*
* cmd_line: input command line
*
* Returns: a new history_t state
*/
void add_line_history(history_t *history, const char *cmd_line);

/*
* print_history: print history up to max history line
*
* Returns: a list of history
*/
void print_history(history_t *history);

/*
* print_history: print line in the history at index
*
* Returns: line in the history at index
*/
char *find_line_history(history_t *history, int index);

/*
* free_history: free memory used in history
*/
void free_history(history_t *history);

#endif
