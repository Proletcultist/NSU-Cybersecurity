#pragma once

#include <stddef.h>
#include <stdio.h>

#define SHA3_OK 0
#define SHA3_WRONG_SIZE 1
#define SHA3_PERMUTATOR_FAULT 2
#define SHA3_IO_ERROR 3
#define SHA3_SPONGE_FAULT 4
#define SHA3_ALLOCATION_ERROR 5

int sha3_256(FILE *file, char *buff, size_t size);
