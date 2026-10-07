#include <stdio.h>

#define DEBUG_EXPR(expr) fprintf(stderr, "%s:%d:%s(): %s: 0x%llX\n", __FILE__, __LINE__, __func__, #expr, (unsigned long long)(expr))

int main(int argc, char* argv[])
{
    printf("Starting point for mdu\n");
    DEBUG_EXPR(argc);
    DEBUG_EXPR(argv);
    return 0;
}
