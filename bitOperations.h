#ifndef BIT_OPERATIONS_H
#define BIT_OPERATIONS_H

#include <stdint.h>
#include <stdbool.h>

#define bitCount(num) __builtin_popcountll(num)
#define lsb(num) __builtin_ctzll(num)
#define setBit(num, index) num |= (1ULL << index)
#define flipBit(num, index) num ^= (1ULL << index)
#define checkBit(num, index) num & (1ULL << index)
#define bit(index) 1ULL << index

#endif