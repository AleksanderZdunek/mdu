/** @file
    Calculate device usage of file tree in 512-byte blocks,
    using multithreading.

    @author Aleksander Zdunek
    @date 2026-10-19
*/

#ifndef BLOCK_COUNT_MULTITHREADED_H
#define BLOCK_COUNT_MULTITHREADED_H
#include <stdint.h>
#include <stddef.h>
#include <errno.h>

struct result
{
    int64_t block_count;
    int error;
};

/**
    Calculate number of 512-byte blocks allocated to a file or directory.
    Does so with multiple thread. In case of directory will traverse the
    file tree and sum up allocated blocks for all files and directories in
    the file tree.

    @param path to the file or directory

    @return Struct holding total of file tree block count, and error code.
        Error code is 0 on success.
        Error code is EACCES if a recoverable file access error occured. In this case the
        thread block sum is still considered meaningful but will not include
        the block count for files that could not be accessed.
        Other error code on critical error. See <error.h>
*/
struct result block_count_multithreaded(const char* path, const size_t nrof_threads);

#endif //BLOCK_COUNT_MULTITHREADED_H
