#ifndef SEARCH_H
#define SEARCH_H

#include <time.h>
#include "bitboards.h"
#include "move.h"
#include "moveGeneration.h"
#include "eval.h"
#include "counters.h"
#include "tables.h"
#include "check.h"
#include "print.h"
#include "book.h"
#include "fen.h"

#define INF 100000

int evaluate(struct bitboards_t *BITBOARDS);

int quiescenceSearch(struct bitboards_t *BITBOARDS, int alpha, int beta, int depth);

bool isThreeFoldRepetition(struct bitboards_t *BITBOARDS);

int negaMax(struct bitboards_t *BITBOARDS, int depth, int alpha, int beta, bool allowNullMove); // , int checkExtensions, int promoExtensions

move_t getBookMove(u64 hash);

void engineMove();

u64 perft(int depth, bitboards_t *BITBOARDS, int originalDepth);

#endif