#include "job.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void initialize_jobs(job_t *jobs, int max_jobs) {
  for (int i = 0; i < max_jobs; ++i) {
      jobs[i].pid = -1; // Initialize all pid to negative number
      jobs[i].state = UNDEFINED;
  }
}

bool add_job(job_t *jobs, int max_jobs, pid_t pid, job_state_t state, const char *cmd_line){
  // add new job to jobs array
  for (int i=0; i < max_jobs; i++){
    if (jobs[i].pid == -1){   // ASK: Check null in array? use neg num. -1 ** (FIX)
      jobs[i].cmd_line = strdup(cmd_line); // ASK: this dynamically allocate cmd_line. We do this bc it is a string?
      jobs[i].state = state;
      jobs[i].pid = pid;
      jobs[i].jid = i+1;
      return true;
    }
  }
  // all jobs array already occupied
  return false;
}

bool delete_job(job_t *jobs, pid_t pid, int max_jobs){
  for (int i=0; i< max_jobs; ++i){
    if (jobs[i].pid == pid){
      // reset all fields
      free(jobs[i].cmd_line); // (ASK) Free this because cmdline uses malloc ?
      jobs[i].pid = -1;  // reset pid to empty_value
      jobs[i].state = UNDEFINED;
      jobs[i].jid = 0;
      return true;
    }
  }
  // when job with the given pid id is not found in job array
  return false;
}

void free_jobs(job_t *jobs, int max_jobs){
  // for (int i=0; i < max_jobs; ++i){
  //   free(jobs[i].cmd_line);    // already freed cell in delete jobs
  // }
  free(jobs);
}
