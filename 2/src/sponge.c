#include <stdlib.h>
#include <string.h>
#include "sponge.h"

sponge_construct_t* sponge_construct_new(size_t bitrate, size_t capacity, sponge_permutator_t permutator) {
    sponge_construct_t *ret = malloc(sizeof(sponge_construct_t) + bitrate + capacity);

    *ret = (sponge_construct_t) {
        .bitrate = bitrate,
        .capacity = capacity,
        .permutator = permutator,
    };

    memset(ret->data, 0, ret->bitrate + ret->capacity);

    return ret;
}

void sponge_construct_del(sponge_construct_t *sponge) {
    free(sponge);
}

int sponge_construct_absorb(sponge_construct_t *sponge, char *data, size_t size) {
    if (size != sponge->bitrate) {
        return SPONGE_WRONG_SIZE;
    }

    for (size_t i = 0; i < size; i++) {
        sponge->data[i] ^= data[i];
    }

    int res = sponge->permutator(sponge->data, sponge->bitrate + sponge->capacity);

    if (res != 0) {
        return SPONGE_PERMUTATOR_FAULT;
    }

    return SPONGE_OK;
}

int sponge_construct_squeeze(sponge_construct_t *sponge, char *buf, size_t size) {
    if (size != sponge->bitrate) {
        return SPONGE_WRONG_SIZE;
    }

    memcpy(buf, sponge->data, size);

    int res = sponge->permutator(sponge->data, sponge->bitrate + sponge->capacity);
    
    if (res != 0) {
        return SPONGE_PERMUTATOR_FAULT;
    }

    return SPONGE_OK;
}
