/** @file
    Macros for printing debug strings and error messages to stderr.
    Lines are prefixed with string file, line number and function.

    @author Aleksander Zdunek
    @date 2026-10-10
*/

#ifndef DEBUG_H
#define DEBUG_H
#include <stdio.h>

#define DEBUG_EXPR(expr) fprintf(stderr, "%s:%d:%s(): %s: 0x%llX\n", __FILE__, __LINE__, __func__, #expr, (unsigned long long)(expr))
#define DEBUG_STR(expr) fprintf(stderr, "%s:%d:%s(): %s: %s\n", __FILE__, __LINE__, __func__, #expr, (const char*)(expr))
#define DEBUG_PRINT(str) fprintf(stderr, "%s:%d:%s(): %s\n", __FILE__, __LINE__, __func__, str)

#endif //DEBUG_H
