#pragma once

#include <stdint.h>

/*
Galois field GF(2^8) operations
*/

// Map numbers a and b to the polynomials from GF(2^8) field
// Add these polynomials in terms of GF(2^8) field
// Map result back to the uint8_t number
uint8_t gf256_add(uint8_t a, uint8_t b);

// Map numbers a and b to the polynomials from GF(2^8) field
// Get modulo for multiplication by maping mod_lo to the element of GF(2^8) field and adding x^8
// Multiply these polynomials in terms of GF(2^8) field
// Map result back to the uint8_t number
uint8_t gf256_mul(uint8_t a, uint8_t b, uint8_t mod_lo);
