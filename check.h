#ifndef CHECK_H
#define CHECK_H

#include "bitboards.h"
#include "move.h"
#include "moveGeneration.h"

bool canCaptureOpponentsKing(struct bitboards_t *BITBOARDS);

bool isInCheck(struct bitboards_t *BITBOARDS);

bool isIllegalCastle(move_t move, struct bitboards_t *BITBOARDS);

bool hasLegalMoves(move_t *possible, struct bitboards_t *BITBOARDS, int numMoves);

#endif