#include <stdint.h>
#include "gf256.h"

uint8_t gf256_add(uint8_t a, uint8_t b) {
    return a ^ b;
}

// TODO: Write comments
uint8_t gf256_mul(uint8_t a, uint8_t b, uint8_t mod_lo) {
    uint8_t res = 0;

    while (b > 0) {
        if (b & 1) {
            res ^= a;
        }

        b >>= 1;

        if (a >> 7) {
            a <<= 1;
            a ^= mod_lo;
        } else {
            a <<= 1;
        }
    }

    return res;
}
