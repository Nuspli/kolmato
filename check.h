#ifndef CHECK_H
#define CHECK_H

#include "bitboards.h"
#include "move.h"
#include "moveGeneration.h"

bool canCaptureOpponentsKing(struct bitboards_t *BITBOARDS);

bool isInCheck(struct bitboards_t *BITBOARDS, u64 *checkers);

bool isInCheckLight(struct bitboards_t *BITBOARDS);

#endif