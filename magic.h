#include <stdio.h>
#include "bitboards.h"
#include "random.h"

extern int rookRelevantOccupancyAmount[64];

extern int bishopRelevantOccupancyAmount[64];

extern u64 bishopAttackMasks[64];

extern u64 rookAttackMasks[64];

extern u64 rookMagics[64];

extern u64 bishopMagics[64];

u64 normBishopAttacks(int square, u64 occupied);

u64 normRookAttacks(int square, u64 occupied);

u64 setOccupancy(int index, int maskBits, u64 attackMask);

u64 getMagic(int square, int relevantOccupiedBitAmount, bool isRook);

void generateNewMagics();