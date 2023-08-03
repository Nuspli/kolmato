#ifndef MAGIC_H
#define MAGIC_H

#include <stdio.h>
#include "bitboards.h"
#include "random.h"

extern int rookRelevantOccupancyAmount[64];

extern int bishopRelevantOccupancyAmount[64];

extern u64 bishopAttackMasksNoEdges[64];

extern u64 rookAttackMasksNoEdges[64];

extern u64 bishopAttackMasksWithEdges[64];

extern u64 rookAttackMasksWithEdges[64];

extern u64 rookMagics[64];

extern u64 bishopMagics[64];

u64 normBishopAttacks(int square, u64 occupied);

u64 normRookAttacks(int square, u64 occupied);

u64 setOccupancy(int index, int maskBits, u64 attackMask);

void generateNewMagics();

#endif