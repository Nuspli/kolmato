#ifndef COUNTERS_H
#define COUNTERS_H

#include <stdbool.h>

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

extern int moveCount;
extern int halfMoveCount;

extern int maxTime;
extern int maxDepth;

extern bool useBook;

#endif