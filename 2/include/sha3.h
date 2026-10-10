#pragma once

#include <stddef.h>
#include <stdio.h>

#define SHA3_OK 0
#define SHA3_WRONG_SIZE 1
#define SHA3_PERMUTATOR_FAULT 2

int sha3_256(FILE *file, char *buff, size_t size);
