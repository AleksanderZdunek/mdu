/** @file
    Calculate disk usage of files

    @author Aleksander Zdunek
    @date 2026-10-07
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define DEBUG_EXPR(expr) fprintf(stderr, "%s:%d:%s(): %s: 0x%llX\n", __FILE__, __LINE__, __func__, #expr, (unsigned long long)(expr))

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

    DEBUG_EXPR(cfg.nrof_threads);
    for(size_t i = 0; i < cfg.nrof_file_args; ++i)
    {
        puts(cfg.file_args[i]);
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
