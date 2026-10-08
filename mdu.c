/** @file
    Calculate disk usage of files

    @author Aleksander Zdunek
    @date 2026-10-07
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <dirent.h>
#include <assert.h>
#include <pthread.h>

#define DEBUG_EXPR(expr) fprintf(stderr, "%s:%d:%s(): %s: 0x%llX\n", __FILE__, __LINE__, __func__, #expr, (unsigned long long)(expr))

struct cfg
{
    int nrof_threads;
    size_t nrof_file_args;
    const char *const *file_args;
};
struct cfg options(int argc, char* argv[]);
int64_t file_block_count(const char* const path);
int64_t dir_block_count(const char* const path);

typedef struct thread_context
{
    pthread_t thread_id;
    const char* file_path;
} thread_context_t;
void* thread_worker(void* arg);

int main(int argc, char* argv[])
{
    const struct cfg cfg = options(argc, argv);

    DEBUG_EXPR(cfg.nrof_threads);
    // for(size_t i = 0; i < cfg.nrof_file_args; ++i)
    // {
    //     puts(cfg.file_args[i]);
    // }

    for(size_t i = 0; i < cfg.nrof_file_args; ++i)
    {
        thread_context_t ctx = {.file_path = cfg.file_args[i]};

        int err = pthread_create(&ctx.thread_id, NULL, thread_worker, &ctx);
        if(err)
        {
            fprintf(stderr, "Error creating thread for file %s: %s\n", ctx.file_path, strerror(err));
            exit(EXIT_FAILURE);
        }

        int64_t thread_res;
        if( (err = pthread_join(ctx.thread_id, (void**)&thread_res)) )
        {
            fprintf(stderr, "Error joining thread id %ld: %s\n", ctx.thread_id, strerror(err));
            exit(EXIT_FAILURE);
        }

        if(thread_res < 0) continue;
        printf("%ld\t%s\n", thread_res, ctx.file_path);
    }

    //TODO: Implement mdu solution

    return 0;
}

/**
    Parse command line options.
    May mutate the order of arguments in the argv pointer array.

    @param argc Number of arguments
    @param argv Argument pointer array

    @return struct cfg holding options configuration.
        Does not return but exits with EXIT_FAILURE if incorrect options are passed.
*/
struct cfg options(int argc, char* argv[])
{
    struct cfg cfg = { .nrof_threads = 1 };
    int opt;
    while((opt = getopt(argc, argv, "j:")) != -1)
    {
        switch(opt)
        {
            case 'j':
                cfg.nrof_threads = atoi(optarg);
                if(cfg.nrof_threads < 1)
                {
                    fprintf(stderr, "Bad number of threads argument: %s\n", optarg);
                    exit(EXIT_FAILURE);
                }
                break;
            default:
                fprintf(stderr, "Usage: mdu [-j number_of_threads] file ...\n");
                exit(EXIT_FAILURE);
                break;

        }
    }
    cfg.nrof_file_args = argc - optind;
    cfg.file_args = (const char *const *)(argv + optind);
    return cfg;
}

/**
    TODO: Document
    Number of 512-byte blocks allocated to a file.

    @param path to the file

    @return The block count.
        -1 if file could not be read.
*/
int64_t file_block_count(const char* const path)
{
    struct stat statbuf;
    if(lstat(path, &statbuf))
    {
        fprintf(stderr, "mdu: cannot access '%s': %s\n", path, strerror(errno));
        return -1;
    }

    int64_t block_count = statbuf.st_blocks;

    if(S_ISDIR(statbuf.st_mode))
    {
        int64_t res = dir_block_count(path);
        if(res < 0) return -1;
        block_count += res;
    }

    return block_count;
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
        DEBUG_EXPR("malloc() error");
        return NULL;
    }
    int res = snprintf(buf, buf_size, "%s/%s", path, name);
    assert((size_t)res == buf_size - 1);
    return buf;
}

/**
    TODO: Document

    @param path to directory

    @return TODO:
*/
int64_t dir_block_count(const char* const path)
{
    DIR* dir = opendir(path);
    if(!dir)
    {
        fprintf(stderr, "mdu: cannot open directory '%s': %s\n", path, strerror(errno));
        return -1;
    }

    int64_t sum_block_count = 0;
    struct dirent* ent;
    while( (errno = 0, ent = readdir(dir)) )
    {
        //Ignore current and parent directories
        if(!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..")) continue;

        char* const ent_path = pathcat(path, ent->d_name);
        if(!ent_path)
        {
            //TODO: Handle catastrophic error
            DEBUG_EXPR("TODO: catastrophic error?");
            return -1;
        }

        int64_t ent_block_count = file_block_count(ent_path);
        //Silently ignore errors
        //TODO: Handle catastrophic error?
        if(ent_block_count > 0)
        {
            sum_block_count += ent_block_count;
        }

        free(ent_path);
    }
    if(errno)
    {
        fprintf(stderr, "mdu: cannot read directory '%s': %s\n", path, strerror(errno));
        //TODO:
        DEBUG_EXPR("TODO: catastrophic error?");
        //TODO: Also attempt to close directory? That'll likely also fail.
        return -1;
    }

    if(closedir(dir))
    {
        fprintf(stderr, "mdu: cannot close directory '%s': %s\n", path, strerror(errno));
        //TODO:
        DEBUG_EXPR("TODO: catastrophic error?");
        return -1;
    }

    return sum_block_count;
}

void* thread_worker(void* arg)
{
    thread_context_t* p_ctx = (thread_context_t*)arg;

    int64_t blocks = file_block_count(p_ctx->file_path);

    return (void*)blocks;
}
