#include "signal_handlers.h"
#include "shell.h"
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <sys/wait.h>

/*
* sigchld_handler - The kernel sends a SIGCHLD to the shell whenever
*     a child job terminates (becomes a zombie), or stops because it
*     received a SIGSTOP or SIGTSTP signal. The handler reaps all
*     available zombie children, but doesn't wait for any other // terminating or sigstop makes child stop
*     currently running children to terminate.
* Citation: Bryant and O’Hallaron, Computer Systems: A Programmer’s Perspective, Third Edition
*/
void sigchld_handler(int sig)
{
  int olderrno = errno;
  pid_t pid;
  int child_status;
  sigset_t mask_all, prev_all;
  while ((pid = waitpid(-1, &child_status, WNOHANG | WUNTRACED)) > 0){    // child sent signnal defined by pid
    // printf("---pid receive %d ", pid);
    sigprocmask(SIG_SETMASK, &mask_all, &prev_all);
    if (pid == -1){
      perror("waitpid");
      exit(EXIT_FAILURE);
    } else if (pid == fgpid) {
      // foreground job finished
      // printf("---pid is fgpid deleted fg job\n");
      for (int i=0; i < shell->MAX_JOBS; ++i){
          if (fgpid == shell->jobs[i].pid){
            if (shell->jobs[i].state != SUSPENDED){
              delete_job(shell->jobs, pid, shell->MAX_JOBS);
              fgpid = -1; // reset our variable to -1
            }
          }
        }
    } else if (pid != fgpid) {
      // background job
      // printf("--background handler job\n");
      if (WIFEXITED(child_status)){ //child terminate normally
        // printf("bg exit normally\n");
        // background job finished. delete the bg job.
        delete_job(shell->jobs, pid, shell->MAX_JOBS);
      } else if (WIFSTOPPED(child_status)){ // when ctr+z move child to suspend.
        // printf("suspend bg job\n");
        for (int i=0; i < shell->MAX_JOBS; ++i){
          if (pid == shell->jobs[i].pid){
              shell->jobs[i].state = SUSPENDED;
              }
        }
      } else if (WIFSIGNALED(child_status) > 0){ // when ctr+C terminate the fg child.
        if (WTERMSIG(child_status) == SIGINT){
          // delete job if force terminate
          // printf("terminate bg job\n");
          delete_job(shell->jobs, pid, shell->MAX_JOBS);
          fgpid = -1;
        }
      }
      sigprocmask(SIG_SETMASK, &prev_all, NULL);
    }
    jobscount--;
  }
  errno = olderrno;
}

/*
* sigint_handler - The kernel sends a SIGINT to the shell whenver the
*    user types ctrl-c at the keyboard.  Catch it and send it along
*    to the foreground job.
* Citation: Bryant and O’Hallaron, Computer Systems: A Programmer’s Perspective, Third Edition
*/
void sigint_handler(int sig)
{
  // Sio_puts("\n Caught signal %d\n", sig);
  kill(-fgpid, SIGINT); // Terminate the process
}

/*
* sigtstp_handler - The kernel sends a SIGTSTP to the shell whenever
*     the user types ctrl-z at the keyboard. Catch it and suspend the
*     foreground job by sending it a SIGTSTP.
* Citation: Bryant and O’Hallaron, Computer Systems: A Programmer’s Perspective, Third Edition
*/
void sigtstp_handler(int sig)
{
  if (fgpid != -1) {
    // Sio_puts("\n Suspend signal caught %d\n", sig);
    kill(-fgpid, SIGTSTP);
    for (int i=0; i < shell->MAX_JOBS; ++i){
      if (fgpid == shell->jobs[i].pid){
        shell->jobs[i].state = SUSPENDED;
      }
    }
    fgpid = -1;
    jobscount--;
  }
}

/*
* setup_handler - wrapper for the sigaction function
*
* Citation: Bryant and O’Hallaron, Computer Systems: A Programmer’s Perspective, Third Edition
*/
typedef void handler_t(int);
handler_t *setup_handler(int signum, handler_t *handler)
{
    struct sigaction action, old_action;

    action.sa_handler = handler;
    sigemptyset(&action.sa_mask); /* block sigs of type being handled */
    action.sa_flags = SA_RESTART; /* restart syscalls if possible */

    if (sigaction(signum, &action, &old_action) < 0) {
        perror("Signal error");
        exit(1);
    }
    return (old_action.sa_handler);
}


void initialize_signal_handlers() {

    // sigint handler: Catches SIGINT (ctrl-c) signals.
    setup_handler(SIGINT,  sigint_handler);   /* ctrl-c */
    // sigtstp handler: Catches SIGTSTP (ctrl-z) signals.
    setup_handler(SIGTSTP, sigtstp_handler);  /* ctrl-z */
    // sigchld handler: Catches SIGCHILD signals.
    setup_handler(SIGCHLD, sigchld_handler);  /* Terminated or stopped child */
}



