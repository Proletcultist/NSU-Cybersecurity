#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "sha3.h"

#define MODULO(x, n) (((x) % (n) + (n)) % (n))

typedef struct sha3_sponge {
    size_t bitrate, capacity;
    uint64_t state[5][5];
} sha3_sponge_t;

static const unsigned int rhomatrix[5][5] = {
    {0,   1, 62, 28, 27},
    {36, 44,  6, 55, 20},
    {3,  10, 43, 25, 39},
    {41, 45, 15, 21,  8},
    {18,  2, 61, 56, 14}
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

static inline uint64_t rotl64(uint64_t x, uint64_t n);
static int sha3_sponge_construct(sha3_sponge_t *sponge, size_t bitrate, size_t capacity);
static int sha3_sponge_absorb(sha3_sponge_t *sponge, char *data, size_t size);
static int sha3_sponge_squeeze(sha3_sponge_t *sponge, char *buf, size_t size);
static void sha3_sponge_permutate(sha3_sponge_t *sponge);

static inline uint64_t rotl64(uint64_t x, uint64_t n) {
    n %= 64;

    if (n == 0) {
        return x;
    }

    // Get last n bits of x
    uint64_t last = x >> (64 - n);

    return (x << n) | last;
}

static int sha3_sponge_construct(sha3_sponge_t *sponge, size_t bitrate, size_t capacity) {
    if (bitrate + capacity != sizeof(sponge->state)) {
        return SHA3_WRONG_SIZE;
    }

    *sponge = (sha3_sponge_t) {
        .bitrate = bitrate,
        .capacity = capacity
    };
    memset(sponge->state, 0, sizeof(sponge->state));

    return SHA3_OK;
}

static int sha3_sponge_absorb(sha3_sponge_t *sponge, char *data, size_t size) {
    if (size != sponge->bitrate) {
        return SHA3_WRONG_SIZE;
    }

    char *state = (char*) sponge->state;
    for (size_t i = 0; i < size; i++) {
        state[i] ^= data[i];
    }

    sha3_sponge_permutate(sponge);

    return SHA3_OK;
}

static int sha3_sponge_squeeze(sha3_sponge_t *sponge, char *buf, size_t size) {
    if (size != sponge->bitrate) {
        return SHA3_WRONG_SIZE;
    }

    memcpy(buf, sponge->state, size);

    sha3_sponge_permutate(sponge);

    return SHA3_OK;
}

static void sha3_sponge_permutate(sha3_sponge_t *sponge) {
    for (int round = 0; round < 24; round++) {
        // Calculate parity for each 5-bit column,
        // so parity[i][k] = parity(words[0..4][i][k])
        uint64_t parity[5] = {0};
        for (int col = 0; col < 5; col++) {
            for (int row = 0; row < 5; row++) {
                parity[col] ^= sponge->state[row][col];
            }
        }

        uint64_t buff[5][5];
        for (int row = 0; row < 5; row++) {
            for (int col = 0; col < 5; col++) {
                // theta step: words[i][j][k] ^= parity(words[0..4][j-1][k]) ^ parity(words[0..4][j+1][k-1])
                sponge->state[row][col] ^= parity[MODULO(col - 1, 5)] ^ rotl64(parity[MODULO(col + 1, 5)], 1);

                // rho step: bitwise left rotate by one of a triangle numbers
                sponge->state[row][col] = rotl64(sponge->state[row][col], rhomatrix[row][col]);

                // pi step: words[3i + 2j][i] = words[i][j]
                buff[MODULO(3 * row + 2 * col, 5)][row] = sponge->state[row][col];
            }
        }

        // chi step: words[i][j] ^= ~words[i][j+1] & words[i][j+2]
        for (int row = 0; row < 5; row++) {
            for (int col = 0; col < 5; col++) {
                sponge->state[row][col] = buff[row][col] ^ (~buff[row][MODULO(col + 1, 5)] & buff[row][MODULO(col + 2, 5)]);
            }
        }

        sponge->state[0][0] ^= rndc[round];
    }
}

int sha3_256_hashfile(FILE *file, char *out, size_t size) {
    if (size != 256 / 8) {
        return SHA3_WRONG_SIZE;
    }

    // Buffer for input/output of sponge
    char buff[1088 / 8];

    // Sponge initialization
    sha3_sponge_t sponge;
    int res = sha3_sponge_construct(
                        &sponge,
        /*bitrate*/     1088 / 8,
        /*capacity*/    512 / 8
    );
    if (res != SHA3_OK) {
        return SHA3_INTERNAL_FAULT;
    }

    // Read full blocks from file
    size_t bytes_read = fread(buff, 1, sizeof(buff), file);
    while (bytes_read == sizeof(buff)) {
        int res = sha3_sponge_absorb(&sponge, buff, bytes_read);      
        if (res != SHA3_OK) {
            return SHA3_INTERNAL_FAULT;
        }

        bytes_read = fread(buff, 1, sizeof(buff), file);
    }

    if (ferror(file)) {
        return SHA3_IO_ERROR;
    }

    // Add domain separator bits (01) and start of padding pattern (10*)
    // as next byte, and fill remaining bytes with pattern (0*1)
    // NOTE: bits in bytes must be in LSB order
    buff[bytes_read] = 0b00000110;
    bytes_read++;
    memset(buff + bytes_read, 0, sizeof(buff) - bytes_read);
    bytes_read += sizeof(buff) - bytes_read;
    buff[bytes_read - 1] |= 0b10000000; 

    // Absorb last block
    res = sha3_sponge_absorb(&sponge, buff, bytes_read);      
    if (res != SHA3_OK) {
        return SHA3_INTERNAL_FAULT;
    }

    // Squeeze one block from sponge
    res = sha3_sponge_squeeze(&sponge, buff, sizeof(buff));
    if (res != SHA3_OK) {
        return SHA3_INTERNAL_FAULT;
    }

    // Output first 256 bits
    memcpy(out, buff, size);

    return SHA3_OK;
}
