/** @file
    Calculate device usage of file tree in 512-byte blocks,
    using multithreading.

    @author Aleksander Zdunek
    @date 2026-10-10
*/
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <assert.h>
#include <stdbool.h>
#include "block_count_multithreaded.h"
#include "job_stack.h"
#include "debug.h"

typedef struct shared_context
{
    job_stack_t job_stack;
} shared_context_t;

typedef struct thread_context
{
    pthread_t thread_id;
    int64_t block_sum;
    shared_context_t* shared_context;
} thread_context_t;

static void* thread_worker(void* arg);
static char* pathcat(const char* const path, const char* const name);
static bool push_more_jobs(const char* path, job_stack_t* job_stack);

int64_t block_count_multithreaded(const char* path, const size_t nrof_threads)
{
    shared_context_t shared_context;
    job_stack_init(&shared_context.job_stack);

    //Duplicate path to allow it to be free()'d after removing from the job stack.
    const char* initial_job = strdup(path);
    if(!initial_job)
    {
        DEBUG_PRINT("Error creating initial job");
        job_stack_destroy(&shared_context.job_stack);
        return -1;
    }
    if(!job_stack_push(&shared_context.job_stack, initial_job))
    {
        DEBUG_PRINT("Error pushing initial job");
        job_stack_destroy(&shared_context.job_stack);
        return -1;
    }

    thread_context_t threads[nrof_threads];
    memset(threads, 0, sizeof(threads));

    //Create threads
    int64_t block_count = 0; //return value
    for(size_t i = 0; i < nrof_threads; ++i)
    {
        threads[i].shared_context = &shared_context;
        int err = pthread_create(&threads[i].thread_id, NULL, thread_worker, &threads[i]);
        if(err)
        {
            DEBUG_PRINT(strerror(err));
            //TODO: how to handle error with running threads?
            //TODO: pthread_cancel()?
            block_count = -1;
            break;
        }
    }

    //Wait for threads
    for(size_t i = 0; i < nrof_threads; ++i)
    {
        void* thread_retval;
        int err = pthread_join(threads[i].thread_id, &thread_retval);
        if(err)
        {
            DEBUG_PRINT(strerror(err));
            //TODO: Better error handling
            continue;
        }
        if(block_count >= 0)
        {
            block_count += threads[i].block_sum;
        }
    }

    return block_count;
}

/**
    TODO: document

    @param arg Pointer to a thread_context_t instance

    @return TODO:
*/
static void* thread_worker(void* arg)
{
    thread_context_t* thread_context = (thread_context_t*)arg;

    //TODO: Make sure threads don't quit before all work is done.
    //      Just because the job stack is empty doesn't mean more jobs won't be
    //      pushed by another thread.

    const char* path = NULL;
    while((path = job_stack_pop(&thread_context->shared_context->job_stack)))
    {
        //stat() the file path
        struct stat statbuf;
        if(lstat(path, &statbuf))
        {
            fprintf(stderr, "mdu: cannot access '%s': %s\n", path, strerror(errno));
            free((char*)path);
            continue;
        }
        thread_context->block_sum += statbuf.st_blocks;

        //Push files in directory on to job stack
        if(S_ISDIR(statbuf.st_mode))
        {
            if(!push_more_jobs(path, &thread_context->shared_context->job_stack))
            {
                //TODO: handle critical error
                DEBUG_STR("TODO: handle error");
                //TODO: rememeber to free path?
            }
        }

        free((char*)path);
    }

    return (void*)thread_context->block_sum;
}

/**
    Concatenate a file name to a path with a '/'.

    @param path Leading part of the concatenated string
    @param name Trailing part of the concatenated string

    @return Concatenated file path. Must be deallocated with free().
*/
static char* pathcat(const char* const path, const char* const name)
{
    size_t buf_size = strlen(path) + strlen(name) + 2;
    char* buf = malloc(buf_size);
    if(!buf)
    {
        DEBUG_PRINT("malloc() error");
        return NULL;
    }
    int res = snprintf(buf, buf_size, "%s/%s", path, name);
    assert((size_t)res == buf_size - 1);
    return buf;
}

/**
    Push files in a directory on to the job stack.

    @param path to directory
    @param job_stack Ponter to job stack

    @return true if no errors or if there were file access error which should
        be ignored.
        false on critical error.
        On error an error message is printed to stderr.
*/
static bool push_more_jobs(const char* path, job_stack_t* job_stack)
{
    DIR* dir = opendir(path);
    if(!dir)
    {
        fprintf(stderr, "mdu: cannot open directory '%s': %s\n", path, strerror(errno));
        return true;
    }

    bool critical_error = false;
    struct dirent* ent;
    while( (errno = 0, ent = readdir(dir)) )
    {
        //Ignore current and parent directories
        if(!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..")) continue;

        const char* ent_path = pathcat(path, ent->d_name);
        if(!ent_path)
        {
            critical_error = true;
            break;
        }

        if(!job_stack_push(job_stack, ent_path))
        {
            DEBUG_PRINT("Error pusing job on to stack");
            critical_error = true;
            free((char*)ent_path);
            break;
        }
    }
    if(errno)
    {
        fprintf(stderr, "mdu: cannot read directory '%s': %s\n", path, strerror(errno));
        if(errno != EACCES) critical_error = true;
    }

    if(closedir(dir))
    {
        fprintf(stderr, "mdu: cannot close directory '%s': %s\n", path, strerror(errno));
        critical_error = true;
    }
    return !critical_error;
}
