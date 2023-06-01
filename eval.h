#ifndef EVAL_H
#define EVAL_H

extern int pawnEvalBlack[64];

extern int rookEvalBlack[64];

extern int knightEvalBlack[64];

extern int bishopEvalBlack[64];
  
extern int queenEvalBlack[64];

extern int kingEvalBlack[64];

extern int pawnEvalWhite[64];

extern int rookEvalWhite[64];

extern int knightEvalWhite[64];

extern int bishopEvalWhite[64];

extern int queenEvalWhite[64];

extern int kingEvalWhite[64];

extern int kingEvalEnd[64];

extern int pieceValues[6];
extern int *whiteEvalTables[6];
extern int *blackEvalTables[6];

#endif