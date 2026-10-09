#pragma once

#include <stddef.h>

#define SPONGE_OK 0
#define SPONGE_WRONG_SIZE 1

typedef void (*sponge_permutator_t)(char *data, size_t size);

typedef struct sponge_construct {
    size_t bitrate, capacity;
    sponge_permutator_t permutator;
    char data[];
} sponge_construct_t;

sponge_construct_t* sponge_construct_new(size_t bitrate, size_t capacity, sponge_permutator_t permutator);
void sponge_construct_del(sponge_construct_t *sponge);
int sponge_construct_absorb(sponge_construct_t *sponge, char *data, size_t size);
int sponge_construct_squeeze(sponge_construct_t *sponge, char *buf, size_t size);
