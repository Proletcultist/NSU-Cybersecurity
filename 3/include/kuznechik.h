#pragma once

#include <stddef.h>
#include <stdint.h>

typedef enum kuznechik_res {
    KUZNECHIK_OK,
    KUZNECHIK_WRONG_SIZE
} kuznechik_res_t;

kuznechik_res_t kuznechik_encrypt(
    size_t block_size,
    size_t key_size,
    const uint8_t block[block_size],
    const uint8_t key[key_size],
    uint8_t res[block_size]
);
