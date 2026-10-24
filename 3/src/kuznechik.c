#include <string.h>
#include <stddef.h>
#include "kuznechik.h"

#include "gf256.h"

#define GF_MOD 0xc3 // Polynomial x^8+x^7+x^6+x+1 without x^8 to use as mod in GF(2^8) field ops

static const uint8_t round_consts[32][16] = {
    {0x6e, 0xa2, 0x76, 0x72, 0x6c, 0x48, 0x7a, 0xb8,
    0x5d, 0x27, 0xbd, 0x10, 0xdd, 0x84, 0x94, 0x01},
    {0xdc, 0x87, 0xec, 0xe4, 0xd8, 0x90, 0xf4, 0xb3,
    0xba, 0x4e, 0xb9, 0x20, 0x79, 0xcb, 0xeb, 0x02},
    {0xb2, 0x25, 0x9a, 0x96, 0xb4, 0xd8, 0x8e, 0x0b,
    0xe7, 0x69, 0x04, 0x30, 0xa4, 0x4f, 0x7f, 0x03},
    {0x7b, 0xcd, 0x1b, 0x0b, 0x73, 0xe3, 0x2b, 0xa5,
    0xb7, 0x9c, 0xb1, 0x40, 0xf2, 0x55, 0x15, 0x04},
    {0x15, 0x6f, 0x6d, 0x79, 0x1f, 0xab, 0x51, 0x1d,
    0xea, 0xbb, 0x0c, 0x50, 0x2f, 0xd1, 0x81, 0x05},
    {0xa7, 0x4a, 0xf7, 0xef, 0xab, 0x73, 0xdf, 0x16,
    0x0d, 0xd2, 0x08, 0x60, 0x8b, 0x9e, 0xfe, 0x06},
    {0xc9, 0xe8, 0x81, 0x9d, 0xc7, 0x3b, 0xa5, 0xae,
    0x50, 0xf5, 0xb5, 0x70, 0x56, 0x1a, 0x6a, 0x07},
    {0xf6, 0x59, 0x36, 0x16, 0xe6, 0x05, 0x56, 0x89,
    0xad, 0xfb, 0xa1, 0x80, 0x27, 0xaa, 0x2a, 0x08},
    {0x98, 0xfb, 0x40, 0x64, 0x8a, 0x4d, 0x2c, 0x31,
    0xf0, 0xdc, 0x1c, 0x90, 0xfa, 0x2e, 0xbe, 0x09},
    {0x2a, 0xde, 0xda, 0xf2, 0x3e, 0x95, 0xa2, 0x3a,
    0x17, 0xb5, 0x18, 0xa0, 0x5e, 0x61, 0xc1, 0x0a},
    {0x44, 0x7c, 0xac, 0x80, 0x52, 0xdd, 0xd8, 0x82,
    0x4a, 0x92, 0xa5, 0xb0, 0x83, 0xe5, 0x55, 0x0b},
    {0x8d, 0x94, 0x2d, 0x1d, 0x95, 0xe6, 0x7d, 0x2c,
    0x1a, 0x67, 0x10, 0xc0, 0xd5, 0xff, 0x3f, 0x0c},
    {0xe3, 0x36, 0x5b, 0x6f, 0xf9, 0xae, 0x07, 0x94,
    0x47, 0x40, 0xad, 0xd0, 0x08, 0x7b, 0xab, 0x0d},
    {0x51, 0x13, 0xc1, 0xf9, 0x4d, 0x76, 0x89, 0x9f,
    0xa0, 0x29, 0xa9, 0xe0, 0xac, 0x34, 0xd4, 0x0e},
    {0x3f, 0xb1, 0xb7, 0x8b, 0x21, 0x3e, 0xf3, 0x27,
    0xfd, 0x0e, 0x14, 0xf0, 0x71, 0xb0, 0x40, 0x0f},
    {0x2f, 0xb2, 0x6c, 0x2c, 0x0f, 0x0a, 0xac, 0xd1,
    0x99, 0x35, 0x81, 0xc3, 0x4e, 0x97, 0x54, 0x10},
    {0x41, 0x10, 0x1a, 0x5e, 0x63, 0x42, 0xd6, 0x69,
    0xc4, 0x12, 0x3c, 0xd3, 0x93, 0x13, 0xc0, 0x11},
    {0xf3, 0x35, 0x80, 0xc8, 0xd7, 0x9a, 0x58, 0x62,
    0x23, 0x7b, 0x38, 0xe3, 0x37, 0x5c, 0xbf, 0x12},
    {0x9d, 0x97, 0xf6, 0xba, 0xbb, 0xd2, 0x22, 0xda,
    0x7e, 0x5c, 0x85, 0xf3, 0xea, 0xd8, 0x2b, 0x13},
    {0x54, 0x7f, 0x77, 0x27, 0x7c, 0xe9, 0x87, 0x74,
    0x2e, 0xa9, 0x30, 0x83, 0xbc, 0xc2, 0x41, 0x14},
    {0x3a, 0xdd, 0x01, 0x55, 0x10, 0xa1, 0xfd, 0xcc,
    0x73, 0x8e, 0x8d, 0x93, 0x61, 0x46, 0xd5, 0x15},
    {0x88, 0xf8, 0x9b, 0xc3, 0xa4, 0x79, 0x73, 0xc7,
    0x94, 0xe7, 0x89, 0xa3, 0xc5, 0x09, 0xaa, 0x16},
    {0xe6, 0x5a, 0xed, 0xb1, 0xc8, 0x31, 0x09, 0x7f,
    0xc9, 0xc0, 0x34, 0xb3, 0x18, 0x8d, 0x3e, 0x17},
    {0xd9, 0xeb, 0x5a, 0x3a, 0xe9, 0x0f, 0xfa, 0x58,
    0x34, 0xce, 0x20, 0x43, 0x69, 0x3d, 0x7e, 0x18},
    {0xb7, 0x49, 0x2c, 0x48, 0x85, 0x47, 0x80, 0xe0,
    0x69, 0xe9, 0x9d, 0x53, 0xb4, 0xb9, 0xea, 0x19},
    {0x05, 0x6c, 0xb6, 0xde, 0x31, 0x9f, 0x0e, 0xeb,
    0x8e, 0x80, 0x99, 0x63, 0x10, 0xf6, 0x95, 0x1a},
    {0x6b, 0xce, 0xc0, 0xac, 0x5d, 0xd7, 0x74, 0x53,
    0xd3, 0xa7, 0x24, 0x73, 0xcd, 0x72, 0x01, 0x1b},
    {0xa2, 0x26, 0x41, 0x31, 0x9a, 0xec, 0xd1, 0xfd,
    0x83, 0x52, 0x91, 0x03, 0x9b, 0x68, 0x6b, 0x1c},
    {0xcc, 0x84, 0x37, 0x43, 0xf6, 0xa4, 0xab, 0x45,
    0xde, 0x75, 0x2c, 0x13, 0x46, 0xec, 0xff, 0x1d},
    {0x7e, 0xa1, 0xad, 0xd5, 0x42, 0x7c, 0x25, 0x4e,
    0x39, 0x1c, 0x28, 0x23, 0xe2, 0xa3, 0x80, 0x1e},
    {0x10, 0x03, 0xdb, 0xa7, 0x2e, 0x34, 0x5f, 0xf6,
    0x64, 0x3b, 0x95, 0x33, 0x3f, 0x27, 0x14, 0x1f},
    {0x5e, 0xa7, 0xd8, 0x58, 0x1e, 0x14, 0x9b, 0x61,
    0xf1, 0x6a, 0xc1, 0x45, 0x9c, 0xed, 0xa8, 0x20}
};

static kuznechik_res_t kuznechik_l(
    size_t block_size, const uint8_t block[restrict block_size],
    uint8_t *restrict res
) {
    static const uint8_t l_vec[16] = {
        148, 32, 133, 16, 194, 192, 1, 251,
        1, 192, 194, 16, 133, 32, 148, 1
    };

    if (block_size != 16) {
        return KUZNECHIK_WRONG_SIZE;
    }

    *res = 0;
    for (size_t i = 0; i < block_size; i++) {
        *res = gf256_add(*res, gf256_mul(l_vec[i], block[i], GF_MOD));
    }

    return KUZNECHIK_OK;
}

// block_a xor block_b -> res
// All arrays must not overlap partially, they can only:
// 1. Not overlap at all
// 2. Be same arrays
static void kuznechik_X(
    size_t size, 
    const uint8_t block_a[size], 
    const uint8_t block_b[size],
    uint8_t res[size]
) {
    for (size_t i = 0; i < size; i++) {
        res[i] = block_a[i] ^ block_b[i];
    }
}

// S kuznechik transformation
static void kuznechik_S(
    size_t size,
    const uint8_t block[size], 
    uint8_t res[size]
) {
    static const uint8_t Pi[256] = {
           252, 238, 221,  17, 207, 110,  49,  22, 251, 196, 250,
           218,  35, 197,   4,  77, 233, 119, 240, 219, 147,  46,
           153, 186,  23,  54, 241, 187,  20, 205,  95, 193, 249,
            24, 101,  90, 226,  92, 239,  33, 129,  28,  60,  66,
           139,   1, 142,  79,   5, 132,   2, 174, 227, 106, 143,
           160,   6,  11, 237, 152, 127, 212, 211,  31, 235,  52,
            44,  81, 234, 200,  72, 171, 242,  42, 104, 162, 253,
            58, 206, 204, 181, 112,  14,  86,   8,  12, 118,  18,
           191, 114,  19,  71, 156, 183,  93, 135,  21, 161, 150,
            41,  16, 123, 154, 199, 243, 145, 120, 111, 157, 158,
           178, 177,  50, 117,  25,  61, 255,  53, 138, 126, 109,
            84, 198, 128, 195, 189,  13,  87, 223, 245,  36, 169,
            62, 168,  67, 201, 215, 121, 214, 246, 124,  34, 185,
             3, 224,  15, 236, 222, 122, 148, 176, 188, 220, 232,
            40,  80,  78,  51,  10,  74, 167, 151,  96, 115,  30,
             0,  98,  68,  26, 184,  56, 130, 100, 159,  38,  65,
           173,  69,  70, 146,  39,  94,  85,  47, 140, 163, 165,
           125, 105, 213, 149,  59,   7,  88, 179,  64, 134, 172,
            29, 247,  48,  55, 107, 228, 136, 217, 231, 137, 225,
            27, 131,  73,  76,  63, 248, 254, 141,  83, 170, 144,
           202, 216, 133,  97,  32, 113, 103, 164,  45,  43,   9,
            91, 203, 155,  37, 208, 190, 229, 108,  82,  89, 166,
           116, 210, 230, 244, 180, 192, 209, 102, 175, 194,  57,
            75,  99, 182
    };

    for (size_t i = 0; i < size; i++) {
        res[i] = Pi[block[i]];
    }
}

static void kuznechik_S_reverse(
    size_t size,
    const uint8_t block[size], 
    uint8_t res[size]
) {
    static const uint8_t Pi_reverse[256] = {
        165,  45,  50, 143,  14,  48,  56, 192,  84, 230, 158,
         57,  85, 126,  82, 145, 100,   3,  87,  90,  28,  96,
          7,  24,  33, 114, 168, 209,  41, 198, 164,  63, 224,
         39, 141,  12, 130, 234, 174, 180, 154,  99,  73, 229,
         66, 228,  21, 183, 200,   6, 112, 157,  65, 117,  25,
        201, 170, 252,  77, 191,  42, 115, 132, 213, 195, 175,
         43, 134, 167, 177, 178,  91,  70, 211, 159, 253, 212,
         15, 156,  47, 155,  67, 239, 217, 121, 182,  83, 127,
        193, 240,  35, 231,  37,  94, 181,  30, 162, 223, 166,
        254, 172,  34, 249, 226,  74, 188,  53, 202, 238, 120,
          5, 107,  81, 225,  89, 163, 242, 113,  86,  17, 106,
        137, 148, 101, 140, 187, 119,  60, 123,  40, 171, 210,
         49, 222, 196,  95, 204, 207, 118,  44, 184, 216,  46,
         54, 219, 105, 179,  20, 149, 190,  98, 161,  59,  22,
        102, 233,  92, 108, 109, 173,  55,  97,  75, 185, 227,
        186, 241, 160, 133, 131, 218,  71, 197, 176,  51, 250,
        150, 111, 110, 194, 246,  80, 255,  93, 169, 142,  23,
         27, 151, 125, 236,  88, 247,  31, 251, 124,   9,  13,
        122, 103,  69, 135, 220, 232,  79,  29,  78,   4, 235,
        248, 243,  62,  61, 189, 138, 136, 221, 205,  11,  19,
        152,   2, 147, 128, 144, 208,  36,  52, 203, 237, 244,
        206, 153,  16,  68,  64, 146,  58,   1,  38,  18,  26,
         72, 104, 245, 129, 139, 199, 214,  32,  10,   8,   0,
         76, 215, 116
    };

    for (size_t i = 0; i < size; i++) {
        res[i] = Pi_reverse[block[i]];
    }
}

static kuznechik_res_t kuznechik_R(
    size_t size,
    const uint8_t block[size], 
    uint8_t res[size]
) {
    uint8_t new_a15 = 0;
    kuznechik_res_t kres = kuznechik_l(size, block, &new_a15);
    if (kres != KUZNECHIK_OK) {
        return kres;
    }

    memmove(res + 1, block, size - 1);
    *res = new_a15;

    return KUZNECHIK_OK;
}

static kuznechik_res_t kuznechik_R_reverse(
    size_t size,
    const uint8_t block[size], 
    uint8_t res[size]
) {
    uint8_t old_a15 = block[0];
    memmove(res, block + 1, size - 1);
    res[size - 1] = old_a15;

    uint8_t new_a0 = 0;
    kuznechik_res_t kres = kuznechik_l(size, res, &new_a0);
    if (kres != KUZNECHIK_OK) {
        return kres;
    }

    res[size - 1] = new_a0;

    return KUZNECHIK_OK;
}

static kuznechik_res_t kuznechik_L(
    size_t size,
    const uint8_t block[size],
    uint8_t res[size]
) {
    kuznechik_res_t kres = kuznechik_R(size, block, res);
    for (int i = 0; i < 15 && kres == KUZNECHIK_OK; i++) {
        kres = kuznechik_R(size, res, res);
    }

    return kres;
}

static kuznechik_res_t kuznechik_L_reverse(
    size_t size,
    const uint8_t block[size],
    uint8_t res[size]
) {
    kuznechik_res_t kres = kuznechik_R_reverse(size, block, res);
    for (int i = 0; i < 15 && kres == KUZNECHIK_OK; i++) {
        kres = kuznechik_R_reverse(size, res, res);
    }

    return kres;
}

static kuznechik_res_t kuznechik_F(
    size_t key_size,
    size_t round_const_size,
    const uint8_t key[key_size],
    const uint8_t round_const[round_const_size],
    uint8_t res[key_size]
) {
    if (round_const_size != 16 || key_size != 32) {
        return KUZNECHIK_WRONG_SIZE;
    }

    const uint8_t *key_lo = key;
    const uint8_t *key_hi = key + round_const_size;

    uint8_t *res_lo = res;
    uint8_t *res_hi = res + round_const_size;

    uint8_t old_key_lo[round_const_size];
    memcpy(old_key_lo, key_lo, round_const_size);

    kuznechik_res_t kres;

    // Transform key_lo
    kuznechik_X(round_const_size, key_lo, round_const, res_lo);
    kuznechik_S(round_const_size, res_lo, res_lo);
    kres = kuznechik_L(round_const_size, res_lo, res_lo);
    if (kres != KUZNECHIK_OK) {
        return kres;
    }

    kuznechik_X(round_const_size, res_lo, key_hi, res_lo);
    memcpy(res_hi, old_key_lo, round_const_size);

    return KUZNECHIK_OK;
}

kuznechik_res_t kuznechik_calculate_round_keys(
    size_t master_key_size,
    size_t keys_amount,
    size_t round_key_size,
    const uint8_t master_key[restrict master_key_size],
    uint8_t res[restrict keys_amount][round_key_size]
) {
    if (keys_amount != 10 || master_key_size != 32 || round_key_size != 16) {
        return KUZNECHIK_WRONG_SIZE;
    }

    // Copy master key to res[0] and res[1]
    memcpy(&res[0], master_key, master_key_size);

    // Calculate res[i] and res[i + 1] from res[i - 2] and res[i - 1] at once
    for (size_t i = 2; i < keys_amount; i += 2) {
        memcpy(&res[i], &res[i - 2], master_key_size);

        for (uint8_t j = 8 * (i / 2 - 1); j < 8 * (i / 2 - 1) + 8; j++) {
            kuznechik_res_t kres = kuznechik_F(
                master_key_size,
                sizeof(round_consts[j]),
                (uint8_t*) &res[i],
                round_consts[j],
                (uint8_t*) &res[i]
            );

            if (kres != KUZNECHIK_OK) {
                return kres;
            }
        }
    }

    return KUZNECHIK_OK;
}

kuznechik_res_t kuznechik_encrypt(
    size_t block_size,
    size_t keys_amount,
    const uint8_t block[block_size],
    const uint8_t round_keys[keys_amount][block_size],
    uint8_t res[block_size]
) {
    if (keys_amount != 10 || block_size != 16) {
        return KUZNECHIK_WRONG_SIZE;
    }

    memcpy(res, block, block_size);

    kuznechik_res_t kres;
    for (int i = 0; i < 9; i++) {
        kuznechik_X(block_size, res, round_keys[i], res);
        kuznechik_S(block_size, res, res);
        kres = kuznechik_L(block_size, res, res);
        if (kres != KUZNECHIK_OK) {
            return kres;
        }
    }

    kuznechik_X(block_size, res, round_keys[9], res);

    return KUZNECHIK_OK;
}

kuznechik_res_t kuznechik_decrypt(
    size_t block_size,
    size_t keys_amount,
    const uint8_t block[block_size],
    const uint8_t round_keys[keys_amount][block_size],
    uint8_t res[block_size]
) {
    if (keys_amount != 10 || block_size != 16) {
        return KUZNECHIK_WRONG_SIZE;
    }

    memcpy(res, block, block_size);

    kuznechik_res_t kres;
    for (int i = 0; i < 9; i++) {
        kuznechik_X(block_size, res, round_keys[9 - i], res);

        kres = kuznechik_L_reverse(block_size, res, res);
        if (kres != KUZNECHIK_OK) {
            return kres;
        }

        kuznechik_S_reverse(block_size, res, res);
    }

    kuznechik_X(block_size, res, round_keys[0], res);

    return KUZNECHIK_OK;
}
