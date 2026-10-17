#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "sha3.h"
#include "sponge.h"

#define MODULO(x, n) (((x) % (n) + (n)) % (n))

static const uint64_t rhomatrix[5][5] = {
    {0, 36, 3, 41, 18},
    {1, 44, 10, 45, 2},
    {62, 6, 43, 15, 61},
    {28, 55, 25, 21, 56},
    {27, 20, 39, 8, 14},
};

static const uint64_t rndc[24] = {
    0x0000000000000001ULL, 0x0000000000008082ULL,
    0x800000000000808aULL, 0x8000000080008000ULL,
    0x000000000000808bULL, 0x0000000080000001ULL,
    0x8000000080008081ULL, 0x8000000000008009ULL,
    0x000000000000008aULL, 0x0000000000000088ULL,
    0x0000000080008009ULL, 0x000000008000000aULL,
    0x000000008000808bULL, 0x800000000000008bULL,
    0x8000000000008089ULL, 0x8000000000008003ULL,
    0x8000000000008002ULL, 0x8000000000000080ULL,
    0x000000000000800aULL, 0x800000008000000aULL,
    0x8000000080008081ULL, 0x8000000000008080ULL,
    0x0000000080000001ULL, 0x8000000080008008ULL
};

static inline uint64_t rotl64(uint64_t x, uint64_t n) {
    // Get last n bits of x
    uint64_t last = x >> (64 - n);

    return (x << n) | last;
}

static int sha3_permutator(char *data, size_t size) {
    // SHA3 permutation is defined for the state consisting of 5 * 5 words
    // of size 64 bits (8 bytes)
    if (size != 5 * 5 * 8) {
        return SHA3_PERMUTATOR_FAULT;
    }

    uint64_t (*words)[5] = (uint64_t (*)[5]) data;

    for (int round = 0; round < 24; round++) {
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
                words[row][col] = rotl64(words[row][col], rhomatrix[row][col]);

                // pi step: words[3i + 2j][i] = words[i][j]
                buff[MODULO(3 * row + 2 * col, 5)][row] = words[row][col];
            }
        }

        // chi step: words[i][j] ^= ~words[i][j+1] & words[i][j+2]
        for (int row = 0; row < 5; row++) {
            for (int col = 0; col < 5; col++) {
                words[row][col] = buff[row][col] ^ (~buff[row][MODULO(col + 1, 5)] & buff[row][MODULO(col + 2, 5)]);
            }
        }

        words[0][0] ^= rndc[round];
    }
    
    return SHA3_OK;
}

int sha3_256(FILE *file, char *out, size_t size) {
    if (size != 256 / 8) {
        return SHA3_WRONG_SIZE;
    }

    // Buffer for input/output of sponge
    char buff[1088 / 8];
    sponge_construct_t *sponge = sponge_construct_new(
        /*bitrate*/     sizeof(buff),
        /*capacity*/    512 / 8,
        /*permutator*/  sha3_permutator
    );
    if (sponge == NULL) {
        return SHA3_ALLOCATION_ERROR;
    }

    size_t bytes_read = fread(buff, 1, sizeof(buff), file);
    while (bytes_read == sizeof(buff)) {
        int res = sponge_construct_absorb(sponge, buff, bytes_read);      
        if (res != SPONGE_OK) {
            sponge_construct_del(sponge);
            return SHA3_SPONGE_FAULT;
        }

        bytes_read = fread(buff, 1, sizeof(buff), file);
    }

    if (ferror(file)) {
        sponge_construct_del(sponge);
        return SHA3_IO_ERROR;
    }

    // Add domain separator bits (01) and start of padding pattern (10*)
    // as next byte, and fill remaining bytes with pattern (0*1)
    // NOTE: bits in bytes must be in LSB order
    buff[bytes_read] = 0b00000110;
    bytes_read++;
    memset(buff, 0, sizeof(buff) - bytes_read);
    bytes_read += sizeof(buff) - bytes_read;
    buff[bytes_read - 1] |= 0b1000000; 

    int res = sponge_construct_absorb(sponge, buff, bytes_read);      
    if (res != SPONGE_OK) {
        sponge_construct_del(sponge);
        return SHA3_SPONGE_FAULT;
    }

    sponge_construct_squeeze(sponge, buff, sizeof(buff));
    memcpy(out, buff, size);

    sponge_construct_del(sponge);
    return SHA3_OK;
}
