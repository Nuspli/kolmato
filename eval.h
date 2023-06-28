#ifndef EVAL_H
#define EVAL_H

#define OPENING 0
#define ENDGAME 1
#define MIDGAME 2

extern int pawnEvalBlackOpening[64];
extern int rookEvalBlackOpening[64];
extern int knightEvalBlackOpening[64];
extern int bishopEvalBlackOpening[64];
extern int queenEvalBlackOpening[64];
extern int kingEvalBlackOpening[64];
extern int pawnEvalWhiteOpening[64];
extern int rookEvalWhiteOpening[64];
extern int knightEvalWhiteOpening[64];
extern int bishopEvalWhiteOpening[64];
extern int queenEvalWhiteOpening[64];
extern int kingEvalWhiteOpening[64];

extern int pawnEvalBlackEndgame[64];
extern int rookEvalBlackEndgame[64];
extern int knightEvalBlackEndgame[64];
extern int bishopEvalBlackEndgame[64];
extern int queenEvalBlackEndgame[64];
extern int kingEvalBlackEndgame[64];
extern int pawnEvalWhiteEndgame[64];
extern int rookEvalWhiteEndgame[64];
extern int knightEvalWhiteEndgame[64];
extern int bishopEvalWhiteEndgame[64];
extern int queenEvalWhiteEndgame[64];
extern int kingEvalWhiteEndgame[64];

extern int pieceValues[2][6];
extern int *whiteEvalTables[2][6];
extern int *blackEvalTables[2][6];

#endif