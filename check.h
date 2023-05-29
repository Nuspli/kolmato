#include "bitboards.h"
#include "move.h"
#include "moveGeneration.h"

bool canCaptureOpponentsKing(struct bitboards_t * bitboards);

bool isInCheck(struct bitboards_t * bitboards);

bool isIllegalCastle(struct move_t move, struct bitboards_t boards);

bool hasLegalMoves(struct move_t *possible, struct bitboards_t boards, int numMoves);