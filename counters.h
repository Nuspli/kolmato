#ifndef COUNTERS_H
#define COUNTERS_H

#include <stdbool.h>
#include <time.h>
#include <stdint.h>

extern int possibleCalls;
extern int moveCalls;
extern int evalCalls;
extern int quiescenceCalls;

extern int nodes;
extern int quietNodes;
extern int visits;
extern int quietVisits;
extern int transpositions;
extern int quietTranspositions;

extern int fullMoveCount;
extern int halfMoveCount;

extern int maxTime;
extern int maxDepth;

extern bool useBook;
extern char *bookName;
extern clock_t searchStartTime;

extern int gameMovesPlayed;
extern int searchMovesPlayed;

extern uint64_t gameHistory[1000];

#endif