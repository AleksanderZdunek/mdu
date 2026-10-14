/** @file
    A stack of jobs for threads to do. Like a job queue but easier to implement.
    A job is described by a file path string for a file that needs to be
    handled.
    Implemented as a mutex guarded wrapper around string_stack.

    @author Aleksander Zdunek
    @date 2026-10-14
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
    Initialize job stack.
    An initialized job stack shall be freed with job_stack_destroy().

    @param[out] js Pointer to uninitialized job_stack_t structure
*/
void job_stack_init(job_stack_t* js);

/**
    Push a file path onto the stack.

    @note The file path string should be dynamically allocated and freeable with
        free(), or else the file path should be removed from the stack before
        calling job_stack_destroy() on the stack.

    @param js Pointer to job stack
    @param path File path string.

    @return true on success
            false on failure to grow the capacity of the stack to accomodate the
            push.
*/
bool job_stack_push(job_stack_t* js, const char* path);

/**
    Remove the top element from the stack.

    @param @param js Pointer to job stack

    @return Pointer to the removed element
        NULL if stack is empty (or the top element is NULL)
*/
const char* job_stack_pop(job_stack_t* js);

/**
    Free initialized job stack.

    @warning Will call free() on each element left on the stack. Ensure that
        each element is a dynamically allocated valid string pointer.

    @param js Pointer to job stack
*/
void job_stack_destroy(job_stack_t* js);

#endif //JOB_STACK_H
