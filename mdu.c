/** @file
    Calculate disk usage of files

    TODO: More detail

    @author Aleksander Zdunek
    @date 2026-10-10
*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "block_count_multithreaded.h"

//TODO: rename cfg -> config
struct cfg
{
    int nrof_threads;
    size_t nrof_file_args;
    const char *const *file_args;
};
struct cfg options(int argc, char* argv[]);

int main(int argc, char* argv[])
{
    const struct cfg cfg = options(argc, argv);

    //TODO: Implement mdu solution

    for(size_t i = 0; i < cfg.nrof_file_args; ++i)
    {
        int64_t block_count = block_count_multithreaded(cfg.file_args[i], cfg.nrof_threads);
        if(block_count < 0) continue;
        printf("%ld\t%s\n", block_count, cfg.file_args[i]);
    }

    //TODO: return appropriate error code
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
