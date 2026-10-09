/** @file
    A stack of jobs for threads to do. Like a job queue but easier to implement.
    A job is described by a file path string for a file that needs to be
    handled.
    Implemented as a mutex guarded wrapper around string_stack.

    @author Aleksander Zdunek
    @date 2026-10-09
*/
#ifndef JOB_STACK_H
#define JOB_STACK_H
#include <pthread.h>
#include <stdbool.h>
#include "string_stack.h"

typedef struct job_stack
{
    pthread_mutex_t mtx;
    string_stack_t stack;
} job_stack_t;

/**
    TODO: Document
*/
void job_stack_init(job_stack_t* js);

/**
    TODO: Document

    @return true on success
            false on failure to grow the capacity of the stack to accomodate the
            push.
*/
bool job_stack_push(job_stack_t* js, const char* path);

/**
    TODO: Document
*/
const char* job_stack_pop(job_stack_t* js);

/**
    TODO: Document
*/
void job_stack_destroy(job_stack_t* js);

#endif //JOB_STACK_H
