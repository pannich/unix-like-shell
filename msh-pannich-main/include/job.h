#ifndef _JOB_H_
#define _JOB_H_

#include <stdbool.h>
#include <sys/types.h>

typedef enum job_state{FOREGROUND, BACKGROUND, SUSPENDED, UNDEFINED} job_state_t;

// Represents a job in a shell.
typedef struct job {
    char *cmd_line;     // The command line for this specific job.
    job_state_t state;  // The current state for this job
    pid_t pid;          // The process id for this job
    int jid;            // The job number for this job
}job_t;

#endif

void initialize_jobs(job_t *jobs, int max_jobs);

/**
*
* add_job: function that adds a new job to the array
*
* job_t: array of jobs that are running in the shell.
*
* max_jobs: maxmimum about of jobs that can be added to the job_t array
*
* pid: the process id for this job
*
* job_state_t: current state for this job
*
* cmd_line: command line for this specific job
*
* Returns: True if job can be added to the job_t array. False if there's no more jobs left to allocate (i.e. max_jobs has been reached)
*/
bool add_job(job_t *jobs, int max_jobs, pid_t pid, job_state_t state, const char *cmd_line);

/**
*
* delete_job: delete current job from job_t array based on pid_t
*
* job_t: array of jobs that are running in the shell.
*
* max_jobs: maxmimum about of jobs that can be added to the job_t array
*
* pid: the process id for this job
*
* job_state_t: current state for this job
*
* cmd_line: command line for this specific job
*
* Returns: True if job can successfully terminated. False if cannot find job in job_t array or cannot terminate job.
*/
bool delete_job(job_t *jobs, pid_t pid, int max_jobs);

/*
* free_jobs: Closes down the shell by deallocating the job array.
*
* jobs: the job array
*
* max_jobs: size of array
*
*/
void free_jobs(job_t *jobs, int max_jobs);
