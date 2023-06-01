#ifndef FEN_H
#define FEN_H

#include <stdio.h>
#include "counters.h"
#include "bitboards.h"
#include "move.h"

void fenToPosition(char* fen, int position[64]);

void updateFenClocks(struct move_t move);

#endif