#ifndef MOVE_H
#define MOVE_H

#include <stdio.h>
#include "bitboards.h"
#include "counters.h"
#include "eval.h"
#include "print.h"

#define MAX_NUM_MOVES 218
#define KINGSIDE        1
#define QUEENSIDE       2

#define mFrom(move)    (move &    0x3F)        // 0000 0000 0000 0011 1111
#define mTo(move)     ((move &   0xFC0) >>  6) // 0000 0000 1111 1100 0000
#define mPromo(move)  ((move &  0x7000) >> 12) // 0000 0111 0000 0000 0000
#define mCastle(move) ((move & 0x18000) >> 15) // 0001 1000 0000 0000 0000
#define mIsEp(move)   ((move & 0x20000) >> 17) // 0010 0000 0000 0000 0000
#define mCrEp(move)   ((move & 0x40000) >> 18) // 0100 0000 0000 0000 0000

#define MOVE(from, to, promo, castle, isEp, crEp) \
    ( \
    (from)         | \
    (to     <<  6) | \
    (promo  << 12) | \
    (castle << 15) | \
    (isEp   << 17) | \
    (crEp   << 18) \
    )

#define move_t unsigned int

// typedef struct move_t {
//     u8 from;
//     u8 to;
//     unsigned int castle : 2;
//     bool isEnPassantCapture;
//     bool createsEnPassant;
//     unsigned int promotesTo : 3;
// } move_t;



typedef struct undo_t {
    int  capturedPiece : 4;
    bool castleKingSide;
    bool castleQueenSide;
    int enPassantSquare : 7;
} undo_t;

void doMove(move_t move, struct bitboards_t *BB, struct undo_t *undo);

void undoMove(move_t move, struct bitboards_t *BB, struct undo_t *undo);

u64 doMoveLight(move_t move, struct bitboards_t *BB);

move_t buildMove(char *move, struct bitboards_t *BITBOARDS);

#endif