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

#define DEBUG_EXPR(expr) fprintf(stderr, "%s:%d:%s(): %s: 0x%llX\n", __FILE__, __LINE__, __func__, #expr, (unsigned long long)(expr))

struct cfg
{
    int nrof_threads;
    size_t nrof_file_args;
    const char *const *file_args;
};
struct cfg options(int argc, char* argv[]);
int64_t file_block_count(const char* const path);

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
        int64_t blocks = file_block_count(cfg.file_args[i]);
        if(blocks < 0) continue;
        printf("%ld\t%s\n", blocks, cfg.file_args[i]);
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
        fprintf(stderr, "mdu: can not access '%s': %s\n", path, strerror(errno));
        return -1;
    }

    if(S_ISDIR(statbuf.st_mode))
    {
        //TODO:
        DEBUG_EXPR("TODO: handle directory");
    }

    return statbuf.st_blocks;
}
