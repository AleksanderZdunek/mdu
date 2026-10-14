/** @file
    Calculate device usage of file tree in 512-byte blocks,
    using multithreading.

    Each thread picks file paths from a job stack, adds the file block count to
    a sum total, and adds any discoverd sub-tree file paths to the job stack.

    @author Aleksander Zdunek
    @date 2026-10-14
*/
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <assert.h>
#include <stdbool.h>
#include <stdatomic.h>
#include "block_count_multithreaded.h"
#include "job_stack.h"

//Certain debug prints intentionally left behind as informative error messages
#define DEBUG_PRINT(str) fprintf(stderr, "%s:%d:%s(): %s\n", __FILE__, __LINE__, __func__, str)

//Condition variable for thread synchronisation
struct condition
{
    pthread_mutex_t mtx;
    pthread_cond_t cond;
    size_t nrof_active_threads;
    atomic_bool error; //Atomic allows the error condition to be checked without locking the mutex
};

typedef struct shared_context
{
    job_stack_t job_stack;
    struct condition condition; //Allow threads to conditionally wait for more jobs or quit.
} shared_context_t;

typedef struct thread_context
{
    pthread_t thread_id;
    int64_t block_sum;
    shared_context_t* shared_context;
} thread_context_t;

//----------------------------- Order of contents ------------------------------
int64_t block_count_multithreaded(const char* path, const size_t nrof_threads);
static void shared_context_init(shared_context_t* ctx);
static void shared_context_clean(shared_context_t* ctx);
static size_t create_threads(thread_context_t* threads, const size_t nrof_threads, shared_context_t* shared_context);
static size_t join_threads(thread_context_t* threads, const size_t nrof_threads);
static void* thread_worker(void* arg);
static void raise_error_flag(struct condition* c);
static bool wait_continue(struct condition* c);
static void signal_wait_condition(struct condition* c);
static char* pathcat(const char* const path, const char* const name);
static bool push_more_jobs(const char* path, job_stack_t* job_stack, struct condition* condition);

//----------------------------- External functions -----------------------------

int64_t block_count_multithreaded(const char* path, const size_t nrof_threads)
{
    shared_context_t shared_context;
    shared_context_init(&shared_context);
    shared_context.condition.nrof_active_threads = nrof_threads;

    //Duplicate path to allow it to be free()'d after removing from the job stack.
    const char* initial_job = strdup(path);
    if(!initial_job)
    {
        DEBUG_PRINT("Error creating initial job");
        shared_context_clean(&shared_context);
        return -1;
    }
    if(!job_stack_push(&shared_context.job_stack, initial_job))
    {
        DEBUG_PRINT("Error pushing initial job");
        shared_context_clean(&shared_context);
        return -1;
    }

    //Create threads
    thread_context_t threads[nrof_threads];
    memset(threads, 0, sizeof(threads));
    size_t nrof_threads_created = create_threads(threads, nrof_threads, &shared_context);
    if(nrof_threads != nrof_threads_created) raise_error_flag(&shared_context.condition);

    //Wait for threads
    int64_t block_count = join_threads(threads, nrof_threads_created);
    if(shared_context.condition.error) block_count = -1;

    //Cleanup
    shared_context_clean(&shared_context);

    return block_count;
}


//----------------------------- Internal functions -----------------------------

/**
    Initialise data shared between all threads

    @param[out] ctx Pointer to shared_context_t struct to initialise
*/
static void shared_context_init(shared_context_t* ctx)
{
    job_stack_init(&ctx->job_stack);

    pthread_mutex_init(&ctx->condition.mtx, NULL);
    pthread_cond_init(&ctx->condition.cond, NULL);
    ctx->condition.nrof_active_threads = 0;
    ctx->condition.error = false;
}

/**
    Release resources held by shared context

    @param ctx Pointer to initialised shared_context_t
*/
static void shared_context_clean(shared_context_t* ctx)
{
    job_stack_destroy(&ctx->job_stack);

    int err = pthread_mutex_destroy(&ctx->condition.mtx);
    assert(!err);
    err = pthread_cond_destroy(&ctx->condition.cond);
    assert(!err);
}

/**
    Create threads

    @param threads Array of thread_context_t objectes
    @param nrof_threads Number of threads
    @param shared_context Sets the shared_context field of each thread_context_t
        to this pointer.

    @return Number of threads successfully created. May be fewer than number of
        threads requested if an error occured. threads[i].thread_id is
        probably invalid for threads[i] indices i equal to or greater than
        return value.

*/
static size_t create_threads(thread_context_t threads[], const size_t nrof_threads, shared_context_t* shared_context)
{
    size_t i = 0;
    for(; i < nrof_threads; ++i)
    {
        threads[i].shared_context = shared_context;
        int err = pthread_create(&threads[i].thread_id, NULL, thread_worker, &threads[i]);
        if(err)
        {
            fprintf(stderr, "Error creating thread index %ld: %s\n", i, strerror(err));
            break;
        }
    }
    return i;
}

/**
    Wait for threads.
    Return the total number of blocks allocated for the file tree calculated
    by all threads.

    @param threads Array of thread_context_t objectes
    @param nrof_threads Number of threads to join

    @return Block count if all threads were successfull. -1 on error.
*/
static size_t join_threads(thread_context_t* threads, const size_t nrof_threads)
{
    int block_count = 0;
    for(size_t i = 0; i < nrof_threads; ++i)
    {
        void* thread_retval; //Unused. Left for reference.
        int err = pthread_join(threads[i].thread_id, &thread_retval);
        if(err)
        {
            fprintf(stderr, "Error joining thread index %ld: %s\n", i, strerror(err));
            block_count = -1;
            //Need to join the rest of the threads even if one threw an error
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
    Sum the number 512-byte blocks allocated to files in the file system.

    Passed as start_routine to phtreads_create(). Intended to work concurrently
    with an arbitrary number of threads running the same routine.

    Pops a file path from a job stack and adds the allocated block count for
    that file to a thread-local sum. If the file is a directory, adds the paths
    for the files directly one level under the directory to the job stack.
    Continues doing this until the job stack is empty, then waits for a signal
    from other threads that either a new file path is available on the job stack
    or no more file paths will be added to the job stack. In the latter case
    returns and the block sums for all threads should be summed for a total
    allocated block count of the whole file tree.

    File paths are absolute or relative to current working directory.
    Symbolic links are not followed, meaning the allocated blocks for the
    symlink itself are counted, but not the file the symlink points to.
    Hard links may be counted duplicate times.

    @param arg Pointer to a thread_context_t instance

    @return The thread sum of blocks, cast to void*. This pointer shall NOT be
        dereferenced.
        The same value is stored in thread_context_t::block_sum and is probably
        better read from the thread_context_t instance.
*/
static void* thread_worker(void* arg)
{
    thread_context_t* thread_context = (thread_context_t*)arg;

    do {
        const char* path = NULL;
        while(
            (!thread_context->shared_context->condition.error) &&
            (path = job_stack_pop(&thread_context->shared_context->job_stack))
        )
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
                if(!push_more_jobs(path,
                    &thread_context->shared_context->job_stack,
                    &thread_context->shared_context->condition)
                ) raise_error_flag(&thread_context->shared_context->condition);
            }

            free((char*)path);
        }
    } while(wait_continue(&thread_context->shared_context->condition));

    return (void*)thread_context->block_sum;
}

/**
    Set condition variable error flag and broadcast change to waiting threads.

    @param c Pointer to condition struct that holds a pthread_cond_t, associated
    pthread_mutex_t, and error boolean.
*/
static void raise_error_flag(struct condition* c)
{
    int err = pthread_mutex_lock(&c->mtx);
    assert(!err);

    c->error = true;
    pthread_cond_broadcast(&c->cond);

    err = pthread_mutex_unlock(&c->mtx);
    assert(!err);
}

/**
    Blocks until thread receives a signal that there are more jobs on the job
    stack, or all threads are idle implying that no more job will be added to
    the job stack.

    Decrements the active threads counter condition variable when called.
    Increaments the counter when returning true.
    Wakes up all other waiting threads when called by the last thread to go
    idle.

    @param c Pointer to condition struct that holds a pthread_cond_t, associated
        pthread_mutex_t, and counter of active threads.

    @return true if should continue
            false if no more jobs
*/
static bool wait_continue(struct condition* c)
{
    int err = pthread_mutex_lock(&c->mtx);
    assert(!err);

    assert(c->nrof_active_threads > 0);
    c->nrof_active_threads -= 1;
    if(c->nrof_active_threads == 0 || c->error)
    {
        pthread_cond_broadcast(&c->cond);
        err = pthread_mutex_unlock(&c->mtx);
        assert(!err);
        return false;
    }

    pthread_cond_wait(&c->cond, &c->mtx);

    //Check the state of the condition variable again after waking up.
    bool ret = false;
    if(c->nrof_active_threads > 0 && !c->error)
    {
        c->nrof_active_threads += 1;
        ret = true;
    }

    err = pthread_mutex_unlock(&c->mtx);
    assert(!err);

    return ret;
}

/**
    Signal one waiting thread that a condition is updated

    @param c Pointer to structure holding a phtread_cond_t and associated
        pthread_mutex_t.
*/
static void signal_wait_condition(struct condition* c)
{
    int err = pthread_mutex_lock(&c->mtx);
    assert(!err);
    pthread_cond_signal(&c->cond);
    err = pthread_mutex_unlock(&c->mtx);
    assert(!err);
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
static bool push_more_jobs(const char* path, job_stack_t* job_stack, struct condition* condition)
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
        //Signal one waiting thread that there is a new job on the stack
        signal_wait_condition(condition);
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
