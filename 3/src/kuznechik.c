#include <string.h>
#include <stddef.h>
#include "kuznechik.h"

#include "gf256.h"

#define GF_MOD 0xc3 // Polynomial x^8+x^7+x^6+x+1 without x^8 to use as mod in GF(2^8) field ops

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
    kuznechik_res_t kres = kuznechik_l(size, block, &new_a0);
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

kuznechik_res_t kuznechik_encrypt(
    size_t block_size,
    size_t key_size,
    const uint8_t block[block_size],
    const uint8_t key[key_size],
    uint8_t res[block_size]
) {
    if (block_size != 16 || key_size != 32) {
        return KUZNECHIK_WRONG_SIZE;
    }

    uint8_t iter_key[32];
    uint8_t round_const[16];
    memcpy(iter_key, key, key_size);
    memcpy(res, block, block_size);

    kuznechik_res_t kres;
    for (int i = 0; i < 4; i++) {
        kuznechik_X(block_size, res, iter_key, res);
        kuznechik_S(block_size, res, res);
        kres = kuznechik_L(block_size, res, res);
        if (kres != KUZNECHIK_OK) {
            return kres;
        }

        kuznechik_X(block_size, res, iter_key + key_size / 2, res);
        kuznechik_S(block_size, res, res);
        kres = kuznechik_L(block_size, res, res);
        if (kres != KUZNECHIK_OK) {
            return kres;
        }

        for (uint8_t j = 8 * i + 1; j <= 8 * i + 8; j++) {
            memset(round_const, 0, sizeof(round_const));
            round_const[sizeof(round_const) - 1] = j;
            kres = kuznechik_L(sizeof(round_const), round_const, round_const);
            if (kres != KUZNECHIK_OK) {
                return kres;
            }

            kres = kuznechik_F(key_size, sizeof(round_const), iter_key, round_const, iter_key);
            if (kres != KUZNECHIK_OK) {
                return kres;
            }
        }
    }

    kuznechik_X(block_size, res, iter_key, res);
    kuznechik_S(block_size, res, res);
    kres = kuznechik_L(block_size, res, res);
    if (kres != KUZNECHIK_OK) {
        return kres;
    }

    kuznechik_X(block_size, res, iter_key + key_size / 2, res);

    return KUZNECHIK_OK;
}
