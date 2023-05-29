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

struct bitboards_t doMove(struct move_t move, struct bitboards_t bitboards);

struct bitboards_t doNullMove(struct bitboards_t bitboards);

struct move_t buildMove(char *move, struct bitboards_t bitboards);

#endif