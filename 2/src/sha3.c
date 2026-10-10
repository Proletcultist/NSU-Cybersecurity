#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "sha3.h"
#include "sponge.h"

#define MODULO(x, n) (((x) % (n) + (n)) % (n))

static uint64_t rhomatrix[25] = {0, 36, 3, 41, 18, 1, 44, 10, 45, 2, 62, 6, 43, 15, 61, 28, 55, 25, 21, 56, 27, 20, 39, 8, 14};

static inline uint64_t rotl64(uint64_t x, uint64_t n) {
    // Get last n bits of x
    uint64_t last = x >> (64 - n);

    return (x << n) | last;
}

int sha3_permutator(char *data, size_t size) {
    // SHA3 permutation is defined for the state consisting of 5 * 5 words
    // of size 64 bits (8 bytes)
    if (size != 5 * 5 * 8) {
        return SHA3_PERMUTATOR_FAULT;
    }

    uint64_t (*words)[5] = (uint64_t (*)[5]) data;

    // Calculate parity for each 5-bit column,
    // so parity[i][k] = parity(words[0..4][i][k])
    uint64_t parity[5] = {0};
    for (int col = 0; col < 5; col++) {
        for (int row = 0; row < 5; row++) {
            parity[col] ^= words[row][col];
        }
    }

    uint64_t buff[5][5];
    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 5; col++) {
            // theta step: words[i][j][k] ^= parity(words[0..4][j-1][k]) ^ parity(words[0..4][j+1][k-1])
            words[row][col] ^= parity[MODULO(col - 1, 5)] ^ rotl64(parity[MODULO(col + 1, 5)], 1);

            // rho step: bitwise left rotate by one of a triangle numbers
            words[row][col] = rotl64(words[row][col], rhomatrix[row + col]);

            // pi step: words[3i + 2j][i] = words[i][j]
            buff[MODULO(3 * row + 2 * col, 5)][row] = words[row][col];
        }
    }
    memcpy(words, buff, sizeof(buff));

    // chi step: words[i][j] ^= ~words[i][j+1] & words[i][j+2]
    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 5; col++) {
            words[row][col] ^= ~words[row][MODULO(col + 1, 5)] & words[row][MODULO(col + 2, 5)];
        }
    }
    
    return SHA3_OK;
}

int sha3_256(FILE *file, char *buff, size_t size) {
    if (size != 256) {
        return SHA3_WRONG_SIZE;
    }

    return SHA3_OK;
}
