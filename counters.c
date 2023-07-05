#include "counters.h"

// function call counters for benchmarking
int possibleCalls = 0;
int moveCalls = 0;
int evalCalls = 0;
int quiescenceCalls = 0;
// search information
int nodes = 0;
int quietNodes = 0;
int visits = 0;
int quietVisits = 0;
int transpositions = 0;
int quietTranspositions = 0;
// counters for the fen output
int fullMoveCount = 0;
int halfMoveCount = 0;

int maxTime;
int maxDepth;

bool useBook = false;
char *bookName;
clock_t searchStartTime = 0;

int gameMovesPlayed = 0;
int searchMovesPlayed = 0;

uint64_t gameHistory[1000] = {0};
uint64_t searchHistory[1000] = {0};