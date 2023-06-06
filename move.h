#ifndef MOVE_H
#define MOVE_H
#include <stdio.h>
#include "bitboards.h"
#include "counters.h"
#include "eval.h"

#define MAX_NUM_MOVES 256
#define KINGSIDE 1
#define QUEENSIDE 2

typedef struct move_t {
    u8 from;
    u8 to;
    u8 pieceType;
    u8 castle;
    u8 isEnPassantCapture;
    u8 createsEnPassant;
    u8 promotesTo;
} move_t;

typedef struct undo_t {
    int8_t capturedPiece;
    bool castleKingSide;
    bool castleQueenSide;
    u64 enPassantSquare;
} undo_t;

void doMove(struct move_t *move, struct bitboards_t *BB, struct undo_t *undo);

struct move_t *buildMove(char *move, struct bitboards_t *BITBOARDS);

void undoMove(struct move_t *move, struct bitboards_t *BB, struct undo_t *undo);

#endif