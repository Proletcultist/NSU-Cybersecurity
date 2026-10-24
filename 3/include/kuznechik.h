#pragma once

#include <stddef.h>
#include <stdint.h>

typedef enum kuznechik_res {
    KUZNECHIK_OK,
    KUZNECHIK_WRONG_SIZE
} kuznechik_res_t;

kuznechik_res_t kuznechik_calculate_round_keys(
    size_t master_key_size,
    size_t keys_amount,
    size_t round_key_size,
    const uint8_t master_key[master_key_size],
    uint8_t res[keys_amount][round_key_size]
);

kuznechik_res_t kuznechik_encrypt(
    size_t block_size,
    size_t keys_amount,
    const uint8_t block[block_size],
    const uint8_t round_keys[keys_amount][block_size],
    uint8_t res[block_size]
);

kuznechik_res_t kuznechik_decrypt(
    size_t block_size,
    size_t keys_amount,
    const uint8_t block[block_size],
    const uint8_t round_keys[keys_amount][block_size],
    uint8_t res[block_size]
);
