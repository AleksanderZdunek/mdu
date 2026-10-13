/** @file
    Calculate device usage of file tree in 512-byte blocks,
    using multithreading.

    @author Aleksander Zdunek
    @date 2026-10-10
*/

#ifndef BLOCK_COUNT_MULTITHREADED_H
#define BLOCK_COUNT_MULTITHREADED_H
#include <stdint.h>
#include <stddef.h>

/**
    Calculate number of 512-byte blocks allocated to a file or directory.
    Does so with multiple thread. In case of directory will traverse the
    file tree and sum up allocated blocks for all files and directories in
    the file tree.

    @param path to the file or directory

    @return Total block cound of file tree.
        Negative on error.
*/
int64_t block_count_multithreaded(const char* path, const size_t nrof_threads);

#endif //BLOCK_COUNT_MULTITHREADED_H
