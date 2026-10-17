#pragma once

#include <stddef.h>
#include <stdio.h>

#define SHA3_OK 0
#define SHA3_WRONG_SIZE 1
#define SHA3_IO_ERROR 2
#define SHA3_ALLOCATION_ERROR 3

int sha3_256_hashfile(FILE *file, char *buff, size_t size);
