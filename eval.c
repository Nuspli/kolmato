#include "eval.h"

// each piece is worth a different amount of material depending on the square it is on and the phase of the game
// blacks pieces are negative as it is the minimizing player

int materialValues[6] = {
    90, 310, 320, 490, 880, 0
};

int pawnEvalWhiteOpeningPosition[64] = {
    0,  0,   0,  0,   0,   0,   0,  0,
    5,  15,  15, -15, -15, 5,   15, 5,
    0,  -10, -5, 5,   5,   -10, -5, 5,
    5,  5,   5,  15,  15,  5,   5,  -5,
    10, 0,   10, 30,  30,  10,  0,  10,
    15, 20,  25, 50,  50,  40,  40, 40,
    50, 60,  75, 100, 100, 80,  80, 80,
    0,  0,   0,  0,   0,   0,   0,  0
};

int pawnEvalWhiteEndgamePosition[64] = {
    0,   0,   0,   0,   0,   0,   0,   0,
    -5,  -5, -5,   -5,  -5,  -5,  -5,  -5,
    0,   0,   0,   0,   0,   0,   0,   0,
    5,   5,   5,   5,   5,   5,   5,   5,
    20,  20,  20,  20,  20,  20,  20,  20,
    50,  50,  50,  50,  50,  50,  50,  50,
    100, 100, 100, 100, 100, 100, 100, 100,
    0,   0,   0,   0,   0,   0,   0,   0
};

int knightEvalWhiteOpeningPosition[64] = {
    -50, -45, -35, -25, -25, -35, -45, -50,
    -45, -20, 0,   5 ,  5,   0,   -20, -45,
    -35, 5,   0,   15,  15,  0,   5,   -35,
    -25, -5,  20,  20,  20,  20,  -5,  -25,
    -30, 10,  25,  25,  25,  25,  10,  -30,
    -35, 5,   15,  10,  10,  15,  5,   -35,
    -45, -20, 0,   0,   0,   0,   -20, -45,
    -50, -40, -30, -25, -25, -30, -40, -50
};

int knightEvalWhiteEndgamePosition[64] = {
    -50, -45, -35, -30, -30, -35, -45, -50,
    -45, -20, 0,   0,   0,   0,   -20, -45,
    -35, 0,   5,   10,  10,  5,   0,   -35,
    -30, 5,   10,  15,  15,  10,  5,   -30,
    -30, 5,   10,  15,  15,  10,  5,   -30,
    -35, 0,   5,   10,  10,  5,   0,   -35,
    -45, -20, 0,   0 ,  0,   0,   -20, -45,
    -50, -40, -30, -25, -25, -35, -40, -50
};

int bishopEvalWhiteOpeningPosition[64] = {
    -15, -10, -15, -10, -10, -10, -10, -15,
    -10, 10,  5,   5,   5,   5,   10,  -10,
    -10, 15,  15,  10,  10,  15,  15,  -10,
    -5,  5,   10,  15,  15,  10,  5,   -5,
    -15, 5,   10,  15,  15,  10,  5,   -15,
    -15, 5,   0,   15,  15,  0,   5,   -15, 
    -15, 5,   5,   5,   5,   5,   5,   -15, 
    -15, -15, -10, -10, -10, -10, -15, -15
};

int bishopEvalWhiteEndgamePosition[64] = {
    -5,  -5, -5,   -5,  -5,  -5,  -5,  -5, 
    -5,  5,   5,   5,   5,   5,   5,   -5,
    -5,  5,   10,  10,  10,  10,  5,   -5,
    -5,  5,   10,  15,  15,  10,  5,   -5,
    -5,  5,   10,  15,  15,  10,  5,   -5, 
    -5,  5,   10,  10,  10,  10,  5,   -5,  
    -5,  5,   5,   5,   5,   5,   5,   -5,  
    -5,  -5,  -5,  -5,  -5,  -5,  -5,  -5
 };

int rookEvalWhiteOpeningPosition[64] = {
    -5,  -10, 0,  10, 10, 0,  -10, -5, 
    -10, -10, -5, 0,  0,  -5, -10, -10,
    -5,  0,   0,  0,  0,  0,  0,   -5,
    0,   0,   0,  0,  0,  0,  0,   0,  
    0,   0,   0,  0,  0,  0,  0,   0, 
    0,   0,   0,  0,  0,  0,  0,   0, 
    5,   5,   5,  5,  5,  5,  5,   5,  
    0,   0,   0,  0,  0,  0,  0,   0
};

int rookEvalWhiteEndgamePosition[64] = {0};

int queenEvalWhiteOpeningPosition[64] = {
    -20, -10, -10, -5, -5, -10, -10, -20,
    -10, 0,   5,   0,  0,  0,   0,   -10,
    -10, 5,   5,   5,  5,  5,   0,   -10,
    0,   0,   5,   5,  5,  5,   0,   -5,
    -5,  0,   5,   5,  5,  5,   0,   -5,
    -10, 0,   5,   5,  5,  5,   0,   -10,
    -10, 0,   0,   0,  0,  0,   0,   -10,
    -20, -10, -10, -5, -5, -10, -10, -20
};

int queenEvalWhiteEndgamePosition[64] = {0};

int kingEvalWhiteOpeningPosition[64] = {
     20,  35,  10,   5,   0,  15,  30,  20,
     15,  20,   0,   0,   5,   0,  15,  15,
    -10, -20, -15, -20, -20, -20, -20, -15,
    -20, -35, -30, -40, -45, -30, -30, -20,
    -30, -40, -40, -50, -55, -40, -40, -30,
    -30, -40, -40, -45, -50, -40, -45, -30,
    -30, -35, -45, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
};

int kingEvalWhiteEndgamePosition[64] = {
    -50, -25, -25, -25, -30, -25, -30, -50,
    -35, -25,   5,   0,   5,   0, -30, -35,
    -30,  -5,  15,  35,  35,  15, -15, -35,
    -35,  -5,  30,  35,  35,  35, -10, -30,
    -35,  -5,  25,  35,  35,  35, -15, -35,
    -35, -15,  20,  30,  35,  15, -10, -30,
    -30, -20, -15,   5,   5,  -5, -25, -30,
    -50, -40, -30, -20, -20, -30, -40, -50
};

int *whitePositionTables[2][6] = {
    {pawnEvalWhiteOpeningPosition, knightEvalWhiteOpeningPosition, bishopEvalWhiteOpeningPosition, rookEvalWhiteOpeningPosition, queenEvalWhiteOpeningPosition, kingEvalWhiteOpeningPosition},
    {pawnEvalWhiteEndgamePosition, knightEvalWhiteEndgamePosition, bishopEvalWhiteEndgamePosition, rookEvalWhiteEndgamePosition, queenEvalWhiteEndgamePosition, kingEvalWhiteEndgamePosition}
};