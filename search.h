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

void quickSortArrayDec(struct move_t structs[], int values[], int left, int right);

void quickSortArrayInc(struct move_t structs[], int values[], int left, int right);

void orderMoves(struct move_t * moves, bitboards_t BITBOARDS, int numMoves);

void orderCaptures(struct move_t *moves, bitboards_t BITBOARDS, int numMoves);

int quiescenceSearch(struct bitboards_t BITBOARDS, int alpha, int beta, int depth);

int negaMax(struct bitboards_t BITBOARDS, int ply, int alpha, int beta, int depth);

struct move_t iterDeepening(struct move_t *possible, struct bitboards_t bitboards, int numMoves);

struct move_t getBookMove(u64 hash);

void engineMove();

int perft(int depth, bitboards_t bitboards, int originalDepth);