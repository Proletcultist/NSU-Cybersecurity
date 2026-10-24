#include <stdio.h>
#include <stdint.h>
#include "kuznechik.h"

int main() {
    uint8_t a[16] = {
        0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x00,
        0xff, 0xee, 0xdd, 0xcc, 0xbb, 0xaa, 0x99, 0x88
    };

    uint8_t key[32] = {
        0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,
        0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
        0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10,
        0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef
    };

    uint8_t round_keys[10][16];
    uint8_t out[16];

    kuznechik_calculate_round_keys(
        sizeof(key),
        sizeof(round_keys) / sizeof(round_keys[0]),
        sizeof(round_keys[0]),
        key,
        round_keys
    );
    for (size_t i = 0; i < 10; i++) {
        printf("K%d: ", i + 1);
        for (size_t j = 0; j < 16; j++) {
            printf("%02x", round_keys[i][j]);
        }
        printf("\n");
    }

    kuznechik_encrypt(
        sizeof(a),
        sizeof(round_keys) / sizeof(round_keys[0]),
        a, round_keys, out
    );

    for (size_t i = 0; i < sizeof(out); i++) {
        printf("%02x", out[i]);
    }
    printf("\n");

    return 0;
}
