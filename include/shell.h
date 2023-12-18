#ifndef _SHELL_H_
#define _SHELL_H_

#include <stdbool.h>
#include "job.h"
#include "history.h"
#include <signal.h>

#endif

// Represents the state of the shell
typedef struct msh {
  /** TODO: IMPLEMENT **/
  int MAX_JOBS;
  int MAX_LINE;
  int MAX_HISTORY;
  job_t *jobs;
  history_t *history;
}msh_t;

// make global var
extern msh_t* shell;
// extern pid_t fgpid;
extern volatile sig_atomic_t fgpid;
extern int jobscount;

/*
* alloc_shell: allocates and initializes the state of the shell
*
* max_jobs: The maximum number of jobs that can be in existence at any point in time.
*
* max_line: The maximum number of characters that can be entered for any specific command line.
*
* max_history: The maximum number of saved history commands for the shell.
*
* Returns: a msh_t pointer that is allocated and initialized
*/
msh_t *alloc_shell(int max_jobs, int max_line, int max_history);

/**
* parse_tok: Continuously retrieves separate commands from the provided command line until all commands are parsed
*
* line:  the command line to parse, which may include multiple commands. If line is NULL then parse_tok continues parsing the previous command line.
*
* job_type: Specifies whether the parsed command is a background or foreground job. If no job is returned then assign the value at the address to -1
*
* Returns: NULL no other commands can be parsed; otherwise, it returns a parsed command from the command line.
*
* Please note this function does modify the ``line`` parameter.
*/
char *parse_tok(char *line, int *job_type);

/*
* compute_num_args - computes the number of arguments in the provided command line string
*
* line: a string that represents text from a shell's command line
*
* Assumptions: You can assume the line does not contain more than a single command (i.e., we do not have "&" or ";" in the line string).
*
* Returns: the total number of arguments for the command including the command name
*/
int compute_num_args(const char *line);

/**
* separate_args: Separates the arguments of command and places them in an allocated array returned by this function
*
* line: the command line to separate. This function assumes only a single command that takes in zero or more arguments.
*
* argc: Stores the number of arguments produced at the memory location of the argc pointer.
*
* is_builtin: true if the command is a built-in command; otherwise false.
*
* Returns: NULL is line contains no arguments; otherwise, a newly allocated array of strings that represents the arguments of the command (similar to argv). Make sure the array includes a NULL value in its last location.
* Note: The user is responsible for freeing the memory return by this function!
*/
char **separate_args(char *line, int *argc, bool *is_builtin);

/*
* evaluate - executes the provided command line string
*
* shell - the current shell state value
*
* line - the command line string to evaluate
*
* Returns: non-zero if the command executed wants the shell program to close. Otherwise, a 0 is returned.
*/
int evaluate(msh_t *shell, char *line);

/*
* waitfg - wait for the foreground jobs to terminate.
*
*/
void waitfg();

/*
* exit_shell - Closes down the shell by deallocating the shell state.
*
* shell - the current shell state value
*
*/
void exit_shell(msh_t *shell);

/*
* builtin_cmd -  takes in your argv array to handle processing any built-in commands.
*       It retunrs char * because the !N may want to return the chosen comamnd line history.
*       return NULL to indicate nothing additional should happen.
*
*/
char *builtin_cmd(char **argv, int argc);
