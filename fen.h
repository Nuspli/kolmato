#ifndef FEN_H
#define FEN_H

#include <stdio.h>
#include "counters.h"
#include "bitboards.h"
#include "move.h"

void fenToPosition(char* fen, bitboards_t *bitboards);

void updateFenClocks(move_t move);

#endif