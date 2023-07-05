#ifndef MOVE_GENERATION_H
#define MOVE_GENERATION_H

#include <stdbool.h>
#include "print.h"
#include "bitboards.h"
#include "magic.h"
#include "move.h"

extern u64 knightAttacks[64];
extern u64 kingAttacks[64];
extern u64 bishopAttacks[64][512];
extern u64 rookAttacks[64][4096];

extern u64 rightmostFileMask;
extern u64 leftmostFileMask;
extern u64 whitePawnStartRank;
extern u64 blackPawnStartRank;
extern u64 pawnPromotionMask;

void initKnightAttacks();

void initKingAttacks();

void initSlidingPieceAttacks(bool isRook);

u64 generateRookMoves(int square, u64 occupancy);

u64 generateBishopMoves(int square, u64 occupancy);

u64 generateRookAttacks(int rookIndex, u64 occupancy, u64 enemyPieces);

u64 generateBishopAttacks(int bishopIndex, u64 occupancy, u64 enemyPieces);
                        
int getMoves(bitboards_t *BB, move_t *MOVES, int *whiteAttacks, int *blackAttacks);

int getAttacks(bitboards_t *BB, move_t *CAPTURES, int *whiteAttacks, int *blackAttacks);

#endif