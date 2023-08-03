#ifndef EVAL_H
#define EVAL_H

#define OPENING 0
#define ENDGAME 1
#define MIDGAME 2

#define PAWNVALUE 90
#define KNIGHTVALUE 310
#define BISHOPVALUE 320
#define ROOKVALUE 490
#define QUEENVALUE 880

extern int materialValues[6];

extern int pawnEvalWhiteOpeningPosition[64];
extern int rookEvalWhiteOpeningPosition[64];
extern int knightEvalWhiteOpeningPosition[64];
extern int bishopEvalWhiteOpeningPosition[64];
extern int queenEvalWhiteOpeningPosition[64];
extern int kingEvalWhiteOpeningPosition[64];

extern int pawnEvalWhiteEndgamePosition[64];
extern int rookEvalWhiteEndgamePosition[64];
extern int knightEvalWhiteEndgamePosition[64];
extern int bishopEvalWhiteEndgamePosition[64];
extern int queenEvalWhiteEndgamePosition[64];
extern int kingEvalWhiteEndgamePosition[64];

extern int *whitePositionTables[2][6];

#endif