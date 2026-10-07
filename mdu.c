#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define DEBUG_EXPR(expr) fprintf(stderr, "%s:%d:%s(): %s: 0x%llX\n", __FILE__, __LINE__, __func__, #expr, (unsigned long long)(expr))

int main(int argc, char* argv[])
{
    printf("Starting point for mdu\n");

    int nrof_threads = 1;
    int opt;
    while((opt = getopt(argc, argv, "j:")) != -1)
    {
        switch(opt)
        {
            case 'j':
                nrof_threads = atoi(optarg);
                if(nrof_threads < 1)
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
    const size_t nrof_file_args = argc - optind;
    const char *const *const file_args = (const char *const *const)(argv + optind);

    DEBUG_EXPR(nrof_threads);
    for(size_t i = 0; i < nrof_file_args; ++i)
    {
        puts(file_args[i]);
    }

    return 0;
}
