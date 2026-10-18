/** @file
    Calculate disk usage of files
    Show 512-byte block count allocated to each file or file tree in a list.
    The list of files is given as command line arguments.
    The number of threads to use for calculating a file tree can be specified
    with the -j option. Default is 1 thread.

    @author Aleksander Zdunek
    @date 2026-10-18
*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "block_count_multithreaded.h"

struct config
{
    int nrof_threads;
    size_t nrof_file_args;
    const char *const *file_args;
};
struct config options(int argc, char* argv[]);

int main(int argc, char* argv[])
{
    const struct config config = options(argc, argv);

    for(size_t i = 0; i < config.nrof_file_args; ++i)
    {
        int64_t block_count = block_count_multithreaded(config.file_args[i], config.nrof_threads);
        if(block_count < 0) return 1;
        printf("%ld\t%s\n", block_count, config.file_args[i]);
    }

    return 0;
}

/**
    Parse command line options.
    May mutate the order of arguments in the argv pointer array.

    @param argc Number of arguments
    @param argv Argument pointer array

    @return struct config holding options configuration.
        Does not return but exits with EXIT_FAILURE if incorrect options are passed.
*/
struct config options(int argc, char* argv[])
{
    struct config cfg = { .nrof_threads = 1 };
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

    //Because apparently this is what test 7/13 in the public test suite expects
    if(!cfg.nrof_file_args)
    {
        fprintf(stderr, "usage: ./mdu {FILE}\n");
        exit(EXIT_FAILURE);
    }

    return cfg;
}
