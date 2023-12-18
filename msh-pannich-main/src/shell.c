#include "shell.h"
#include "signal_handlers.h"
#include "history.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

// TODO: fix sig stomic t in kill command

msh_t* shell = NULL;
// pid_t fgpid = -1; // TODO: change this to volatile
volatile sig_atomic_t fgpid = -1;
int jobscount = 0;

msh_t *alloc_shell(int max_jobs, int max_line, int max_history){
  // setting default values
  if (max_jobs <= 0) {
    max_jobs = 16;
  }
  if (max_line <= 0) {
    max_line = 1024;
  }
  if (max_history <= 0) {
    max_history = 10;
  }

  // assign memory location to shell pointer
  shell = malloc(sizeof(msh_t));

  // assign values
  shell->MAX_JOBS = max_jobs;
  shell->MAX_LINE = max_line;
  shell->MAX_HISTORY = max_history;
  // Allocate memory for jobs array
  shell->jobs = malloc(max_jobs * sizeof(job_t));
  initialize_signal_handlers();
  shell->history = alloc_history(shell->MAX_HISTORY);
  return shell;
}

char *parse_tok(char *line, int *job_type){
  // return an array of command seperate by ; or &
  static char *internal_pointer = NULL;
  int command_len;
  char* curr;

  if (line != NULL) {
    // set to new command if new command passed
    internal_pointer = line;   // make the internal_pointer stores line's memory address
  }

  if (internal_pointer == NULL || *internal_pointer == '\0') {
        *job_type =  -1;
        return NULL;
    }

  // find the next & or ; and store in separator
  char *separator = strpbrk(internal_pointer, "&;");

  // if not found separator; this is last command and is foreground
  if (separator == NULL) {
    *job_type =  1; // foreground
    curr = internal_pointer;  // initialize curr as char pointer, and assign to address of internal_pointer
    command_len = strlen(internal_pointer);
    internal_pointer = NULL; // reset internal_pointer
  } else {

  // find number of char between two pointers
  command_len = separator - internal_pointer;

  if (*separator == '&'){
    *job_type = 0;
  } else if (*separator == ';') {
    *job_type = 1;
  }

  // return pointer to the first char of the command in the original string. It correctly output command because it terminates when see \0 char.
  curr = internal_pointer;

  // update the pointer to the address of the next char of the next job
  internal_pointer = separator + 1;

  // replace separator with null char
  *separator = '\0';
  }

  // create a command string

  bool non_space_found = false;
  char* currcopy = strdup(curr);

  while (*currcopy != '\0' && *currcopy != ';' && *currcopy != '&') {
    if (*currcopy != ' ') {
        non_space_found = true;
        break;  // Exit the loop if a non-space character is found
        }
    currcopy++;
  }
  if (!non_space_found) {
    return NULL;  // Return NULL if only white spaces are found
  }

  char* command = malloc(command_len + 1);
  if (command != NULL) {
        // if memory allocation not fail
        // copy command_len characters from the beginning curr into allocated space in command
        strncpy(command, curr, command_len);
        // Add string termination
        command[command_len] = '\0';
    }

  return command;
}

int compute_num_args(const char *line){
  char* linecopy = strdup(line);

  int argc=0;
  char *p= strtok(linecopy," ");
  while (p!=NULL)
  {
    argc++;
    p = strtok(NULL," ");
  }
  return argc;
}

char **separate_args(char *line, int *argc, bool *is_builtin) {
  // separate args by space into argv array
  if (line == NULL || *line == '\0') {
      *argc = 0;
      *is_builtin = false;
      return NULL;
    }
  *argc = compute_num_args(line); // minus one to ignore the filename argument?

  // assign memory space for argv array
  char **argv = malloc(sizeof(char *) * (*argc + 1));

  // loop through line and put token into array
  char* linecopy = strdup(line);
  char* token = strtok(linecopy, " ");

  int i = 0;
  while (i < *argc)
  {
    argv[i] = malloc(strlen(token) + 1);  // YOU DONT NEED THIS LINE. allocate space to store token string
    if (argv[i] == NULL) {
      fprintf(stderr, "Memory allocation failed.\n");
      exit(EXIT_FAILURE);
    }
    strcpy(argv[i], token);  // copy token it into the memory (this step can be simplified if use argv[i]=stdup(token))
    // free(argv[i]); ASK : should I free argv[i] ? no need bc i return argv ?
    token = strtok(NULL," ");
    i++;
  }

  argv[*argc] = NULL;

  free(linecopy);

  return argv;
}


int evaluate(msh_t *shell, char *line){
  int type; // initialize type here and it'll be updated to number after we call parse_tok
  char *job;
  char **argv;
  int argc;
  bool is_builtin;
  char* linecopy = strdup(line);
  sigset_t mask_all, mask_one, prev_one;
  sigfillset(&mask_all);
  sigemptyset(&mask_one);
  sigaddset(&mask_one, SIGCHLD);


  if (strlen(linecopy) > (*shell).MAX_LINE){
    printf("error: reached the maximum line limit\n");
    return 0;
  }

  job = parse_tok(linecopy, &type);

  while (job != NULL) {

    // parsing cmd line
    argv = separate_args(job, &argc, &is_builtin);
    // printf(">>>>%s %s \n", argv[0], argv[1]);
    /* check built in*/
    char *builtin_return = builtin_cmd(argv, argc);
    if (builtin_return == NULL){
      is_builtin = true;
      if (strcmp(argv[0], "history") != 0){
        add_line_history(shell->history,line);
      }
    } else if (strcmp(builtin_return,"notbuiltin") == 0) {
      is_builtin = false;
      add_line_history(shell->history,line);
    } else {
      // history !N command
      is_builtin = true;
      evaluate(shell, builtin_return); // send this back to evaluate.
    }

    if (!is_builtin){
      pid_t child_pid; // new process
      job_state_t new_state;

      if (strcmp(argv[0], "exit") == 0){
        return 1;
      }

      sigprocmask(SIG_BLOCK, &mask_one, &prev_one); /* Block SIGCHLD */

      /* Fork Child process */
      child_pid = fork();
      if (child_pid == 0) {
        if (new_state == FOREGROUND){
          setpgid(0, 0);
          }
        sigprocmask(SIG_SETMASK, &prev_one, NULL); // unblock SIGCHLD to execute command
        if (execve(argv[0], argv, NULL) == -1 ) { // inside the child we execute command
          perror("execve"); // Error handling.
          printf("This line will not be printed if execve succeeds\n");
          exit(1);
        }
      }

      // printf("%d \n", new_state);
      // printf("%d \n", fgpid);

      /* Parent */
      if (type == 1){
        new_state = FOREGROUND;
        fgpid = child_pid;
      } else if (type == 0){
        new_state = BACKGROUND;
      }

      sigprocmask(SIG_BLOCK, &mask_all, NULL); // Block all signals before add job
      if (!add_job(shell->jobs, shell->MAX_JOBS, child_pid, new_state, argv[0])) {
        printf("error: reached the maximum jobs limit\n");
      }
      jobscount++;
      sigprocmask(SIG_SETMASK, &prev_one, NULL); /* Unblock SGCHLD*/

      /*Foreground job handling*/
      if (fgpid != -1){
        waitfg();
      }
      free(job);         // free job here before assing new malloc on the new job
    }

    free(argv);       // free malloc from separate_args
    job = parse_tok(NULL,&type);  // parse next job in the command line
  }

  return 0;
}

void waitfg(){
  while(1){
    /*wait for the hanler to pass the fpid var to indicate the 'fg' child has terminated*/
    if (fgpid == -1){ // child has terminated
      break;
    }
    // printf("waiting %d\n", fgpid);
    sleep(1); // ASK : no need if use Sigsuspend?
  }
}

void exit_shell(msh_t *shell){
  // at exit shell fn, need to loop thru bg tasks to "wait" all bg to finish
  while (jobscount > 0){
    // printf("waiting to exit\n");
    sleep(1);
  }

  free_history(shell->history);
  free_jobs(shell->jobs, shell->MAX_JOBS); // doubled free in the job deleted already
  free(shell);
}

char *builtin_cmd(char **argv, int argc){
  pid_t pid;
  if (strcmp(argv[0], "jobs") == 0) {
      // Implementation for 'jobs' command
      // list the background or suspened jobs
      int idx = 0; //delete
      for (int i=0; i < shell->MAX_JOBS; ++i){
        if (shell->jobs[i].state == BACKGROUND){
          printf("[%d] %d RUNNING %s\n", shell->jobs[i].jid, shell->jobs[i].pid, shell->jobs[i].cmd_line);
        } else if (shell->jobs[i].state == SUSPENDED) {
          printf("[%d] %d Stopped %s\n", shell->jobs[i].jid, shell->jobs[i].pid, shell->jobs[i].cmd_line);
        }
      }
    return NULL;
  }

  else if (strcmp(argv[0], "history") == 0) {
    add_line_history(shell->history, argv[0]);
    print_history(shell->history);
    return NULL;
  }

  else if (argv[0][0] == '!' && argv[0][1] != '\0' &&
      strspn(argv[0] + 1, "0123456789") == strlen(argv[0] + 1)) {
        int N = atoi(argv[0] + 1);
        return find_line_history(shell->history, N);
      }

  else if (((strcmp(argv[0], "bg" ) == 0 || strcmp(argv[0], "fg" ) == 0)
    && argv[1] != NULL)
    && ((strspn(argv[1] + 1, "0123456789%") == strlen(argv[1] + 1)) ||
    (strspn(argv[1] + 1, "0123456789") == strlen(argv[1] + 1)))) {
    int idx;
    /* bg PID or bg %jid */
    if (argv[1][0] == '%'){
      // Handle JOB_ID case
      idx = atoi(argv[1] + 1) - 1; // convert to jobs array idx
      pid = shell->jobs[idx].pid;
    } else {
      // Handle PID case and save idx
      pid = atoi(argv[1]); //send this to SIGCONT
      for (int i=0; i<shell->MAX_JOBS; ++i){
        if (shell->jobs[i].pid == pid){
          idx = i;
        }
      }
    }

    // change state
    if (strcmp(argv[0], "fg") == 0) {
      // printf("pid : %d\n", pid);
      if (shell->jobs[idx].pid != -1){
        shell->jobs[idx].state = FOREGROUND;
        fgpid = pid;
        // printf("fgpid : %d\n", fgpid);
        kill(-pid, SIGCONT); // Send SIGCONT signal to the PID to resume the process
        waitfg(); // ASK? : not stopping when do fg %1 after ctrl z
        jobscount++;
      }
    } else {
      // fg job continues in the bg
      if (shell->jobs[idx].pid != -1){
      shell->jobs[idx].state = BACKGROUND;
      kill(-pid, SIGCONT); // Send SIGCONT signal to the PID to resume the process
      jobscount++;
      }
    }
    return NULL;
  }

  else if ((argc == 3) &&
    (strcmp(argv[0], "kill") == 0)
    && (strspn(argv[2], "0123456789") == strlen(argv[2]))
    ) {

    int sigNum = atoi(argv[1]);
    pid = atoi(argv[2]);
    sigset_t mask_all, prev_all;

    switch (sigNum) {
        case SIGINT:
        case SIGKILL:
        case SIGCONT:
        case SIGSTOP:
            // Send the specified signal to the process
            if (kill(-pid, sigNum) == -1) {
                perror("kill");
                // Handle error if needed
            } else {
                // printf("2. pid %d \t", pid);
                // Handle specific actions for SIGCONT
                if (sigNum == SIGCONT) {
                  sigprocmask(SIG_SETMASK, &mask_all, &prev_all);
                  for (int i=0; i < shell->MAX_JOBS; ++i){
                    if (pid == shell->jobs[i].pid){
                      shell->jobs[i].state = FOREGROUND;
                      fgpid = pid;
                      sigprocmask(SIG_SETMASK, &prev_all, NULL);
                      waitfg();
                    }
                  }
                }
            return NULL;
            }
            break;
        default:
            // Invalid signal number error message to the user
            printf("error: invalid signal number\n");
            break;
        }
  } else {
    // other commands that are not built-ins
    return "notbuiltin";
  }
}
