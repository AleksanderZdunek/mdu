/** @file
    A stack of jobs for threads to do. Like a job queue but easier to implement.
    A job is described by a file path string for a file that needs to be
    handled.
    Implemented as a mutex guarded wrapper around string_stack.

    @author Aleksander Zdunek
    @date 2026-10-14
*/

#include <errno.h>
#include <assert.h>
#include "job_stack.h"

void job_stack_init(job_stack_t* js)
{
    pthread_mutex_init(&js->mtx, NULL);
    js->stack = string_stack_init();
}

bool job_stack_push(job_stack_t* js, const char* path)
{
    int err = pthread_mutex_lock(&js->mtx);
    assert(!err);

    bool ret = string_stack_push(&js->stack, path);

    err = pthread_mutex_unlock(&js->mtx);
    assert(!err);

    return ret;
}

const char* job_stack_pop(job_stack_t* js)
{
    int err = pthread_mutex_lock(&js->mtx);
    assert(!err);

    const char* ret = string_stack_pop(&js->stack);

    err = pthread_mutex_unlock(&js->mtx);
    assert(!err);

    return ret;
}

void job_stack_destroy(job_stack_t* js)
{
    int err = pthread_mutex_destroy(&js->mtx);
    assert(!err);
    string_stack_clean(&js->stack);
}
