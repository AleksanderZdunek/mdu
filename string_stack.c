/** @file
    A stack of string pointers

    @author Aleksander Zdunek
    @date 2026-10-09
*/
#include <stdlib.h>
#include "string_stack.h"

string_stack_t string_stack_init(void)
{
    return (string_stack_t){0};
}

/**
    Grow stack capacity by 1
    @param stack Pointer to string_stack structure
    @return true on success
            false on realloc() error
*/
static bool string_stack_grow(string_stack_t* stack)
{
    const char** new_stack = realloc(stack->stack, stack->capacity + 1);
    if(!new_stack) return false;
    new_stack[stack->capacity] = NULL;
    stack->stack = new_stack;
    stack->capacity += 1;
    return true;
}

bool string_stack_push(string_stack_t* stack, const char* string)
{
    if(stack->size >= stack->capacity)
    {
        if(!string_stack_grow(stack)) return false;
    }
    stack->stack[stack->size++] = string;
    return true;
}

const char* string_stack_pop(string_stack_t* stack)
{
    if(!stack->size) return NULL;
    const char* r = stack->stack[--stack->size];
    stack->stack[stack->size] = NULL;
    return r;
}

void string_stack_clean(string_stack_t* stack)
{
    while(stack->size)
    {
        free((char*)string_stack_pop(stack));
    }
    stack->capacity = 0;
    free(stack->stack);
    stack->stack = NULL;
}
