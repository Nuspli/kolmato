#include "counters.h"

int possibleCalls = 0;
int moveCalls = 0;
int evalCalls = 0;
int quiescenceCalls = 0;

int nodes = 0;
int quietNodes = 0;
int visits = 0;
int quietVisits = 0;
int transpositions = 0;
int quietTranspositions = 0;

int moveCount = 0;
int halfMoveCount = 0;

int lastfrom = -1;
int lastto = -1;

int maxTime;
int maxDepth;

bool useBook = false;