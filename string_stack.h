/** @file
    A stack of string pointers

    @author Aleksander Zdunek
    @date 2026-10-10
*/
#ifndef STRING_STACK_H
#define STRING_STACK_H
#include <stddef.h> //size_t
#include <stdbool.h>

typedef struct string_stack
{
    const char** stack;
    size_t capacity;
    size_t size;

} string_stack_t;

/**
    Create empty string stack.
    @return zero-initialized  string_stack structure
*/
string_stack_t string_stack_init(void);

/**
    Push a string onto the stack

    @param stack Pointer to string_stack structure
    @param string Pointer to string to put on the stack

    @return true on success
            false on failure to grow the capacity of the stack to accomodate the
            push.
*/
bool string_stack_push(string_stack_t* stack, const char* string);

/**
    Remove the top element from the stack
    @param stack Pointer to string_stack structure

    @return Pointer to the removed element
        NULL if stack is empty (or the top element is NULL)
*/
const char* string_stack_pop(string_stack_t* stack);

/**
    Reset size and capacity to 0 and free all dynamically allocated memory
    associated with the stack, including all string pointers on the stack.
    WARNING: This assumes all strings stored on the stack were dynamically
    allocated and freeable with free, and that the stack contains no duplicate
    pointer elements.

    @param stack Pointer to string_stack structure
*/
void string_stack_clean(string_stack_t* stack);

#endif //STRING_STACK_H
