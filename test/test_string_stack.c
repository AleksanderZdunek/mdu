#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "../string_stack.h"

#define DEBUG_EXPR(expr) fprintf(stderr, "%s:%d:%s(): %s: 0x%llX\n", __FILE__, __LINE__, __func__, #expr, (unsigned long long)(expr))
#define DEBUG_STR(expr) fprintf(stderr, "%s:%d:%s(): %s: %s\n", __FILE__, __LINE__, __func__, #expr, (char*)(expr))

static const char* malloc_str(const char* const str)
{
    size_t buf_size = strlen(str) + 1;
    char* buf = malloc(buf_size);
    assert(buf);
    int res = snprintf(buf, buf_size, "%s", str);
    assert((size_t)res == buf_size - 1);
    return buf;
}

int main(void)
{
    const char* hello = malloc_str("Hello");
    const char* world = malloc_str("World!");

    string_stack_t stack = string_stack_init();
    DEBUG_EXPR(stack.stack == NULL);
    DEBUG_EXPR(stack.capacity == 0);
    DEBUG_EXPR(stack.size == 0);
    DEBUG_EXPR(string_stack_push(&stack, world));
    DEBUG_EXPR(string_stack_push(&stack, hello));
    DEBUG_EXPR(stack.size == 2);
    DEBUG_STR(string_stack_pop(&stack));
    DEBUG_STR(string_stack_pop(&stack));
    DEBUG_EXPR(stack.stack != NULL);
    DEBUG_EXPR(stack.capacity == 2);
    DEBUG_EXPR(stack.size == 0);
    DEBUG_EXPR(string_stack_push(&stack, hello));
    DEBUG_EXPR(string_stack_push(&stack, world));
    DEBUG_EXPR(stack.size == 2);
    string_stack_clean(&stack);
    DEBUG_STR("string_stack_clean(&stack)");
    DEBUG_EXPR(stack.stack == NULL);
    DEBUG_EXPR(stack.capacity == 0);
    DEBUG_EXPR(stack.size == 0);

    //These are expected to error out
    // free((char*)hello);
    // free((char*)world);
    return 0;
}