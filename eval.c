#include "eval.h"

// each piece is worth a different amount of material depending on where it is located
// blacks pieces are negative as it is the minimizing player

// pawns are good in the center and when they are about to promote
int pawnEvalBlack[64] = {
    // from whites view:
    // bottom
    - 90,- 90,- 90,- 90,- 90,- 90,- 90,- 90,
    -140,-140,-140,-150,-150,-140,-140,-140,
    -100,-100,-110,-130,-130,-110,-100,-100,
    - 95,- 95,-100,-125,-120,-100,- 95,- 95,
    - 90,- 90,- 90,-120,-115,- 90,- 90,- 90,
    - 95,- 85,- 80,- 90,- 90,- 80,- 85,- 95,
    - 95,-100,-100,- 65,- 75,-100,-100,- 95,
    - 90,- 90,- 90,- 90,- 90,- 90,- 90,- 90
    // top
};

// rooks are good when infiltrating whites pawn starting rank
int rookEvalBlack[64] = {
    -500,-500,-500,-500,-500,-500,-500,-500,
    -505,-510,-510,-510,-510,-510,-510,-505,
    -495,-500,-500,-500,-500,-500,-500,-495,
    -495,-500,-500,-500,-500,-500,-500,-495,
    -495,-500,-500,-500,-500,-500,-500,-495,
    -495,-500,-500,-500,-500,-500,-500,-495,
    -495,-500,-500,-500,-500,-500,-500,-495,
    -500,-500,-500,-505,-505,-500,-500,-500
};

// knights are best in the center and worst in a corner
int knightEvalBlack[64] = {
    -280,-290,-300,-300,-300,-300,-290,-280,
    -290,-310,-330,-330,-330,-330,-310,-290,
    -300,-330,-340,-345,-345,-340,-330,-300,
    -300,-335,-345,-350,-350,-345,-335,-300,
    -300,-330,-345,-350,-350,-345,-330,-300,
    -300,-335,-340,-345,-345,-340,-335,-300,
    -290,-310,-330,-335,-335,-330,-310,-290,
    -280,-290,-300,-300,-300,-300,-290,-280
};

// same goes for the bishops
int bishopEvalBlack[64] = {
    -310,-320,-320,-320,-320,-320,-320,-310,
    -320,-330,-330,-330,-330,-330,-330,-320,
    -320,-330,-335,-340,-340,-335,-330,-320,
    -320,-335,-335,-340,-340,-335,-335,-320,
    -320,-330,-340,-340,-340,-340,-330,-320,
    -320,-340,-340,-340,-340,-340,-340,-320,
    -320,-335,-330,-330,-330,-330,-335,-320,
    -310,-320,-320,-320,-320,-320,-320,-310
};

int queenEvalBlack[64] = {
    -880,-890,-890,-895,-895,-890,-890,-880,
    -890,-900,-900,-900,-900,-900,-900,-890,
    -890,-900,-905,-905,-905,-905,-900,-890,
    -895,-900,-905,-905,-905,-905,-900,-895,
    -895,-900,-905,-905,-905,-905,-900,-900,
    -890,-900,-905,-905,-905,-905,-905,-890,
    -890,-900,-900,-900,-900,-905,-900,-890,
    -880,-890,-890,-895,-895,-890,-890,-880
};

// a castled king is good and walking it into the open is not
int kingEvalBlack[64] = {
     30,  40,  40,  50,  50,  40,  40,  30,
     30,  40,  40,  50,  50,  40,  40,  30,
     30,  40,  40,  50,  50,  40,  40,  30,
     30,  40,  40,  50,  50,  40,  40,  30,
     20,  30,  30,  40,  40,  30,  30,  20,
     10,  20,  20,  20,  20,  20,  20,  10,
    -20, -20,   0,   0,   0,   0, -20, -20,
    -20, -30, -10,   0,   0, -10, -30, -20
};

int pawnEvalWhite[64] = {
     90,  90,  90,  90,  90,  90,  90,  90,
     95, 100, 100,  65,  75, 100, 100,  95,
     95,  85,  80,  90,  90,  80,  85,  95,
     90,  90,  90, 120, 115,  90,  90,  90,
     95,  95, 100, 125, 120, 100,  95,  95,
    100, 100, 110, 130, 130, 110, 100, 100,
    140, 140, 140, 150, 150, 140, 140, 140,
     90,  90,  90,  90,  90,  90,  90,  90
};

int rookEvalWhite[64] = {
    500, 500, 500, 505, 505, 500, 500, 500,
    495, 500, 500, 500, 500, 500, 500, 495,
    495, 500, 500, 500, 500, 500, 500, 495,
    495, 500, 500, 500, 500, 500, 500, 495,
    495, 500, 500, 500, 500, 500, 500, 495,
    495, 500, 500, 500, 500, 500, 500, 495,
    505, 510, 510, 510, 510, 510, 510, 505,
    500, 500, 500, 500, 500, 500, 500, 500
};

int knightEvalWhite[64] = {
    280, 290, 300, 300, 300, 300, 290, 280,
    290, 310, 330, 335, 335, 330, 310, 290,
    300, 335, 340, 345, 345, 340, 335, 300,
    300, 330, 345, 350, 350, 345, 330, 300,
    300, 335, 345, 350, 350, 345, 335, 300,
    300, 330, 340, 345, 345, 340, 330, 300,
    290, 310, 330, 330, 330, 330, 310, 290,
    280, 290, 300, 300, 300, 300, 290, 280
};

int bishopEvalWhite[64] = {
    310, 320, 320, 320, 320, 320, 320, 310,
    320, 335, 330, 330, 330, 330, 335, 320,
    320, 340, 340, 340, 340, 340, 340, 320,
    320, 330, 340, 340, 340, 340, 330, 320,
    320, 335, 335, 340, 340, 335, 335, 320,
    320, 330, 335, 340, 340, 335, 330, 320,
    320, 330, 330, 330, 330, 330, 330, 320,
    310, 320, 320, 320, 320, 320, 320, 310
};

int queenEvalWhite[64] = {
    880, 890, 890, 895, 895, 890, 890, 880,
    890, 900, 900, 900, 900, 905, 900, 890,
    890, 900, 905, 905, 905, 905, 905, 890,
    895, 900, 905, 905, 905, 905, 900, 900,
    895, 900, 905, 905, 905, 905, 900, 895,
    890, 900, 905, 905, 905, 905, 900, 890,
    890, 900, 900, 900, 900, 900, 900, 890,
    880, 890, 890, 895, 895, 890, 890, 880
};

int kingEvalWhite[64] = {
     20,  30,  10,   0,   0,  10,  30,  20,
     20,  20,   0,   0,   0,   0,  20,  20,
    -10, -20, -20, -20, -20, -20, -20, -10,
    -20, -30, -30, -40, -40, -30, -30, -20,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30
};

int kingEvalEnd[64] = {
      0,  50, 150, 200, 200, 150, 50,  0,
     50, 100, 250, 300, 300, 250, 100, 50,
    150, 250, 350, 400, 400, 350, 250, 150,
    200, 300, 400, 500, 500, 400, 300, 200,
    200, 300, 400, 500, 500, 400, 300, 200,
    150, 250, 350, 400, 400, 350, 250, 150,
     50, 100, 250, 300, 300, 200, 100, 50,
      0,  50, 100, 200, 200, 100, 50,  0
};

int pieceValues[6] = {100, 300, 300, 500, 900, 0};
int *whiteEvalTables[6] = {pawnEvalWhite, knightEvalWhite, bishopEvalWhite, rookEvalWhite, queenEvalWhite, kingEvalWhite};
int *blackEvalTables[6] = {pawnEvalBlack, knightEvalBlack, bishopEvalBlack, rookEvalBlack, queenEvalBlack, kingEvalBlack};