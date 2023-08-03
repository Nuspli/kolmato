#ifndef MOVE_GENERATION_H
#define MOVE_GENERATION_H

#include <stdbool.h>
#include "print.h"
#include "bitboards.h"
#include "magic.h"
#include "move.h"


#define sameRank(a, b) ((a / 8) == (b / 8))
#define sameFile(a, b) ((a % 8) == (b % 8))

extern u64 knightAttacks[64];
extern u64 kingAttacks[64];
extern u64 bishopAttacks[64][512];
extern u64 rookAttacks[64][4096];

extern u64 notRightmostFileMask;
extern u64 notLeftmostFileMask;
extern u64 pawnPromotionMask;

extern u64 rank[8];
extern u64 file[8];
extern u64 adjFiles[8];
extern u64 squaresBetween[64][64];
extern u64 dangerMask[64];

void initKnightAttacks();

void initKingAttacks();

void initSlidingPieceAttacks(bool isRook);

void initSquaresBetween();

void getPins(bitboards_t *BB, u64 *pinned, u8 *pinners);

u64 generateRookMoves(int square, u64 occupancy);

u64 generateBishopMoves(int square, u64 occupancy);

u64 getEnemyAttackMask(bitboards_t *BB);
                        
int getMoves(bitboards_t *BB, move_t *MOVES, u64 checkers, u64 pinned, u8 *pinners, u64 attacked);

int getCaptures(bitboards_t *BB, move_t *CAPTURES, u64 checkers, u64 pinned, u8 *pinners, u64 attacked);

void initDangerMasks();

void getMobility(bitboards_t *BB, int *mobility, int *kingSafety, int *kingUnSafety);

#endif