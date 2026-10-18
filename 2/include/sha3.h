#pragma once

#include <stddef.h>
#include <stdio.h>

typedef enum sha3_result {
    SHA3_OK,            // No errors occured
    SHA3_WRONG_SIZE,    // Wrong size of buffer
    SHA3_IO_ERROR,      // IO error occured: errno can contain any error specified for the routine read(2)
    SHA3_INTERNAL_ERROR // Internal error of SHA3 implementation occured
} sha3_result_t;

sha3_result_t sha3_256_hashfile(FILE *file, char *buff, size_t size);
