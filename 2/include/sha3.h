#pragma once

#include <stddef.h>
#include <stdio.h>

typedef enum sha3_result {
    SHA3_OK,
    SHA3_WRONG_SIZE,
    SHA3_IO_ERROR,
    SHA3_INTERNAL_FAULT
} sha3_result_t;

sha3_result_t sha3_256_hashfile(FILE *file, char *buff, size_t size);
