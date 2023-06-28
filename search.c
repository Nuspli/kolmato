#include "search.h"

u64 whiteSquares = 0xAA55AA55AA55AA55;
u64 blackSquares = 0x55AA55AA55AA55AA;

u64 AFILE = 0x8080808080808080;
u64 BFILE = 0x4040404040404040;
u64 CFILE = 0x2020202020202020;
u64 DFILE = 0x1010101010101010;
u64 EFILE = 0x0808080808080808;
u64 FFILE = 0x0404040404040404;
u64 GFILE = 0x0202020202020202;
u64 HFILE = 0x0101010101010101;

u64 dangerMasks[64] = {0};

void initDangerMasks() {
    for (int i = 0; i < 64; i++) {
        dangerMasks[i] |= kingAttacks[i];
        dangerMasks[i] |= knightAttacks[i];

        if (i % 8 >= 2) {
            dangerMasks[i] |= (1ULL >> 2);
        }
        if (i % 8 <= 5) {
            dangerMasks[i] |= (1ULL << 2);
        }
        if (i / 8 >= 2) {
            dangerMasks[i] |= (1ULL >> 16);
        }
        if (i / 8 <= 5) {
            dangerMasks[i] |= (1ULL << 16);
        }
    }
}

int centerBonus[64] = {
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 2, 2, 2, 2, 2, 2, 1,
    1, 2, 3, 3, 3, 3, 2, 1,
    1, 2, 3, 4, 4, 3, 2, 1,
    1, 2, 3, 4, 4, 3, 2, 1,
    1, 2, 3, 3, 3, 3, 2, 1,
    1, 2, 2, 2, 2, 2, 2, 1,
    1, 1, 1, 1, 1, 1, 1, 1
};

/*
dangerMask for g1:

    0 0 0 0 0 0 0 0
    0 0 0 0 0 0 0 0
    0 0 0 0 0 0 0 0
    0 0 0 0 0 0 0 0
    0 0 0 0 0 1 1 1
    0 0 0 0 1 1 1 1
    0 0 0 0 1 1 0 1

*/

bool equalBoards(struct bitboards_t *b1, struct bitboards_t *b2) {
    // was used for debugging the boards as pointers

    for (int i = 0; i < 64; i++) {
        if (b1->pieceList[i] != b2->pieceList[i]) {
            printf("piece list mismatch\n");
            printf("copy: [\n");
            for (int j = 63; j >= 0; j--) {
                printf("%d ", b1->pieceList[j]);
                if (j % 8 == 0) {
                    printf("\n");
                }
            }
            printf(" ]\n");
            printf("orig: [\n");
            for (int j = 63; j >= 0; j--) {
                printf("%d ", b2->pieceList[j]);
                if (j % 8 == 0) {
                    printf("\n");
                }
            }
            printf(" ]\n");
            return false;
        }
    }

    if (
        b1->color == b2->color &&
        b1->enPassantSquare == b2->enPassantSquare &&
        b1->whiteCastleKingSide == b2->whiteCastleKingSide &&
        b1->whiteCastleQueenSide == b2->whiteCastleQueenSide &&
        b1->blackCastleKingSide == b2->blackCastleKingSide &&
        b1->blackCastleQueenSide == b2->blackCastleQueenSide &&
        b1->whitePawns == b2->whitePawns &&
        b1->whiteKnights == b2->whiteKnights &&
        b1->whiteBishops == b2->whiteBishops &&
        b1->whiteRooks == b2->whiteRooks &&
        b1->whiteQueens == b2->whiteQueens &&
        b1->whiteKing == b2->whiteKing &&
        b1->blackPawns == b2->blackPawns &&
        b1->blackKnights == b2->blackKnights &&
        b1->blackBishops == b2->blackBishops &&
        b1->blackRooks == b2->blackRooks &&
        b1->blackQueens == b2->blackQueens &&
        b1->blackKing == b2->blackKing &&
        b1->allPieces == b2->allPieces &&
        b1->whitePieces == b2->whitePieces &&
        b1->blackPieces == b2->blackPieces &&
        b1->hash == b2->hash && 
        b1->whiteEvalOpening == b2->whiteEvalOpening &&
        b1->whiteEvalEndgame == b2->whiteEvalEndgame &&
        b1->blackEvalOpening == b2->blackEvalOpening &&
        b1->blackEvalEndgame == b2->blackEvalEndgame

    ) {
        return true;
    } else {
        if (b1->color != b2->color) {
            printf("color\n");
        }
        if (b1->enPassantSquare != b2->enPassantSquare) {
            printf("enPassantSquare\n");
        }
        if (b1->whiteCastleKingSide != b2->whiteCastleKingSide) {
            printf("whiteCastleKingSide\n");
        }
        if (b1->whiteCastleQueenSide != b2->whiteCastleQueenSide) {
            printf("whiteCastleQueenSide\n");
        }
        if (b1->blackCastleKingSide != b2->blackCastleKingSide) {
            printf("blackCastleKingSide\n");
        }
        if (b1->blackCastleQueenSide != b2->blackCastleQueenSide) {
            printf("blackCastleQueenSide\n");
        }
        if (b1->whitePawns != b2->whitePawns) {
            printf("whitePawns\n");
        }
        if (b1->whiteKnights != b2->whiteKnights) {
            printf("whiteKnights\n");
        }
        if (b1->whiteBishops != b2->whiteBishops) {
            printf("whiteBishops\n");
        }
        if (b1->whiteRooks != b2->whiteRooks) {
            printf("whiteRooks\n");
        }
        if (b1->whiteQueens != b2->whiteQueens) {
            printf("whiteQueens\n");
        }
        if (b1->whiteKing != b2->whiteKing) {
            printf("whiteKing\n");
        }
        if (b1->blackPawns != b2->blackPawns) {
            printf("blackPawns\n");
        }
        if (b1->blackKnights != b2->blackKnights) {
            printf("blackKnights\n");
        }
        if (b1->blackBishops != b2->blackBishops) {
            printf("blackBishops\n");
        }
        if (b1->blackRooks != b2->blackRooks) {
            printf("blackRooks\n");
        }
        if (b1->blackQueens != b2->blackQueens) {
            printf("blackQueens\n");
        }
        if (b1->blackKing != b2->blackKing) {
            printf("blackKing\n");
        }
        if (b1->allPieces != b2->allPieces) {
            printf("allPieces\n");
        }
        if (b1->whitePieces != b2->whitePieces) {
            printf("whitePieces\n");
        }
        if (b1->blackPieces != b2->blackPieces) {
            printf("blackPieces\n");
        }
        if (b1->hash != b2->hash) {
            printf("hash\n");
        }
        if (b1->whiteEvalOpening != b2->whiteEvalOpening) {
            printf("whiteEvalOpening\n");
        }
        if (b1->whiteEvalEndgame != b2->whiteEvalEndgame) {
            printf("whiteEvalEndgame\n");
        }
        if (b1->blackEvalOpening != b2->blackEvalOpening) {
            printf("blackEvalOpening\n");
        }
        if (b1->blackEvalEndgame != b2->blackEvalEndgame) {
            printf("blackEvalEndgame\n");
        }

        return false;
    }
}

void quickSortArrayDec(move_t moves[], int values[], int left, int right) {
    // sorts the moves decreasingly in order of their evaluation
    if (left < right) {
        // Choose pivot value
        int pivotValue = values[left + (right - left) / 2];

        // Partition the arrays
        int i = left, j = right;
        while (i <= j) {
            while (values[i] > pivotValue) {i++;}
            while (values[j] < pivotValue) {j--;}
            if (i <= j) {
                // Swap elements
                int tmpValue = values[i];
                values[i] = values[j];
                values[j] = tmpValue;
                move_t tmpMove = moves[i];
                moves[i] = moves[j];
                moves[j] = tmpMove;
                i++;
                j--;
            }
        }

        // Recursively sort the sub-arrays
        quickSortArrayDec(moves, values, left, j);
        quickSortArrayDec(moves, values, i, right);
    }
}

void quickSortArrayInc(move_t moves[], int values[], int left, int right) {
    // Sorts the moves increasingly in order of their evaluation
    if (left < right) {
        int pivotValue = values[left + (right - left) / 2];

        int i = left, j = right;
        while (i <= j) {
            while (values[i] < pivotValue) {i++;}
            while (values[j] > pivotValue) {j--;}
            if (i <= j) {
                int tmpValue = values[i];
                values[i] = values[j];
                values[j] = tmpValue;
                move_t tmpMove = moves[i];
                moves[i] = moves[j];
                moves[j] = tmpMove;
                i++;
                j--;
            }
        }

        quickSortArrayInc(moves, values, left, j);
        quickSortArrayInc(moves, values, i, right);
    }
}

move_t killerMoves[128] = {0};

void orderMoves(move_t *moves, bitboards_t *BITBOARDS, int numMoves) {
    int values[numMoves];
    move_t tableMove = tableGetMove(transTable, BITBOARDS->hash);
        
    if (BITBOARDS->color) {
        for (int i = 0; i < numMoves; i++) {
            values[i] = 0;
            if (tableMove == moves[i]) {
                values[i] = INF;
                continue;
            }
            // capture
            if (BITBOARDS->pieceList[mTo(moves[i])] || mIsEp(moves[i])) {
                if (mIsEp(moves[i])) {
                    values[i] -= blackEvalTables[1][0][mTo(moves[i]) - 8];
                } else {
                    values[i] -= blackEvalTables[1][BITBOARDS->pieceList[mTo(moves[i])] * -1 - 1][mTo(moves[i])];
                }
                values[i] -= pieceValues[1][BITBOARDS->pieceList[mFrom(moves[i])] - 1];
            }
                values[i] += whiteEvalTables[1][BITBOARDS->pieceList[mFrom(moves[i])] - 1][mTo(moves[i])] - 
                             whiteEvalTables[1][BITBOARDS->pieceList[mFrom(moves[i])] - 1][mFrom(moves[i])];
        }

        quickSortArrayDec(moves, values, 0, numMoves-1);

    } else {
        for (int i = 0; i < numMoves; i++) {
            values[i] = 0;
            if (tableMove == moves[i]) {
                values[i] = -INF;
                continue;
            }
            
            if (BITBOARDS->pieceList[mTo(moves[i])] || mIsEp(moves[i])) {
                if (mIsEp(moves[i])) {
                    values[i] -= whiteEvalTables[1][0][mTo(moves[i]) + 8];
                } else {
                    values[i] -= whiteEvalTables[1][BITBOARDS->pieceList[mTo(moves[i])] - 1][mTo(moves[i])];
                }
                values[i] += pieceValues[1][BITBOARDS->pieceList[mFrom(moves[i])] * -1 - 1];
            }
                values[i] += blackEvalTables[1][BITBOARDS->pieceList[mFrom(moves[i])] * -1 - 1][mTo(moves[i])] - 
                             blackEvalTables[1][BITBOARDS->pieceList[mFrom(moves[i])] * -1 - 1][mFrom(moves[i])];
            
        }

        quickSortArrayInc(moves, values, 0, numMoves-1);
    }
}

void orderCaptures(move_t *moves, bitboards_t *BITBOARDS, int numMoves) {
    int values[numMoves];
    move_t tableMove = tableGetMove(quietTable, BITBOARDS->hash);
        
    if (BITBOARDS->color) {
        for (int i = 0; i < numMoves; i++) {
            values[i] = 0;

            if (tableMove == moves[i]) {
                values[i] = INF;
                continue;
            }

            values[i] += whiteEvalTables[1][BITBOARDS->pieceList[mFrom(moves[i])] - 1][mTo(moves[i])] - whiteEvalTables[1][BITBOARDS->pieceList[mFrom(moves[i])] - 1][mFrom(moves[i])];
            if (mIsEp(moves[i])) {
                values[i] -= blackEvalTables[1][0][mTo(moves[i]) - 8];
            } else {
                values[i] -= blackEvalTables[1][BITBOARDS->pieceList[mTo(moves[i])] * -1 - 1][mTo(moves[i])];
            }
            values[i] -= pieceValues[1][BITBOARDS->pieceList[mFrom(moves[i])] - 1];
        }

        quickSortArrayDec(moves, values, 0, numMoves-1);

    } else {
        for (int i = 0; i < numMoves; i++) {
            values[i] = 0;

            if (tableMove == moves[i]) {
                values[i] = INF;
                continue;
            }

            values[i] += blackEvalTables[1][BITBOARDS->pieceList[mFrom(moves[i])] * -1 - 1][mTo(moves[i])] - blackEvalTables[1][BITBOARDS->pieceList[mFrom(moves[i])] * -1 - 1][mFrom(moves[i])];
            if (mIsEp(moves[i])) {
                values[i] -= whiteEvalTables[1][0][mTo(moves[i]) + 8];
            } else {
                values[i] -= whiteEvalTables[1][BITBOARDS->pieceList[mTo(moves[i])] - 1][mTo(moves[i])];
            }
            values[i] += pieceValues[1][BITBOARDS->pieceList[mFrom(moves[i])] * -1 - 1];
        }

        quickSortArrayInc(moves, values, 0, numMoves-1);
    }
}

bool drawEvalByMaterial(bitboards_t *BITBOARDS) {

    if (bitCount(BITBOARDS->allPieces) == 2) {
        // only kings
        return true;

    } else if ((BITBOARDS->whitePawns | BITBOARDS->blackPawns) == 0) {
        // no pawns
        if ((BITBOARDS->whiteRooks | BITBOARDS->blackRooks | BITBOARDS->whiteQueens | BITBOARDS->blackQueens) == 0) {
            // no rooks or queens
            if ((BITBOARDS->whiteBishops | BITBOARDS->blackBishops) == 0) {
                // no bishops
                if (bitCount(BITBOARDS->whiteKnights) < 3 && bitCount(BITBOARDS->blackKnights) < 3) {
                    // less than 3 knights can not mate
                    return true;
                }

            } else if ((BITBOARDS->whiteKnights | BITBOARDS->blackKnights) == 0) {
                // there are bishops but no knights
                // only mate would be with bishop pair vs no bishops
                if (abs(bitCount(BITBOARDS->whiteBishops) - bitCount(BITBOARDS->blackBishops)) < 2) {
                    return true;
                }

            } else if ((bitCount(BITBOARDS->whiteKnights) < 3 && BITBOARDS->whiteBishops == 0) || 
                       (bitCount(BITBOARDS->whiteBishops) == 1 && BITBOARDS->whiteKnights == 0)) {
                
                if ((bitCount(BITBOARDS->blackKnights) < 3 && BITBOARDS->blackBishops == 0) || 
                    (bitCount(BITBOARDS->blackBishops) == 1 && BITBOARDS->blackKnights == 0)) {
                    // both players either have not enough knights or only one bishop
                    return true;
                }
            }

        } else if ((BITBOARDS->whiteQueens | BITBOARDS->blackQueens) == 0) {
            // there are rooks but no queens
            if (bitCount(BITBOARDS->whiteRooks) == 1 && bitCount(BITBOARDS->blackRooks) == 1) {
                // both have exactly one rook
                if ((bitCount(BITBOARDS->whiteKnights) + bitCount(BITBOARDS->whiteBishops)) < 2 && 
                    (bitCount(BITBOARDS->blackKnights) + bitCount(BITBOARDS->blackBishops)) < 2) {
                    // but not enough minor pieces to mate
                    return true;
                }

            } else if (bitCount(BITBOARDS->whiteRooks) == 1 && BITBOARDS->blackRooks == 0) {
                // white has exactly one rook   
                 if ((BITBOARDS->whiteKnights | BITBOARDS->whiteBishops == 0) && 
                    // but no minor pieces
                    // black has 1 or 2 minor pieces
                     (((bitCount(BITBOARDS->blackKnights) + bitCount(BITBOARDS->blackBishops)) == 1) ||
                      ((bitCount(BITBOARDS->blackKnights) + bitCount(BITBOARDS->blackBishops)) == 2))) {
                        return true;
                    }
            } else if (bitCount(BITBOARDS->blackRooks) == 1 && BITBOARDS->whiteRooks == 0) {
                // black has exactly one rook
                if ((BITBOARDS->blackKnights | BITBOARDS->blackBishops == 0) &&
                    // but no minor pieces
                    // white has 1 or 2 minor pieces
                    (((bitCount(BITBOARDS->whiteKnights) + bitCount(BITBOARDS->whiteBishops)) == 1) ||
                     ((bitCount(BITBOARDS->whiteKnights) + bitCount(BITBOARDS->whiteBishops)) == 2))) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool sameSquareColor(u64 square1, u64 square2) {
    // checks if two squares are the same color
    if ((square1 & whiteSquares) && (square2 & whiteSquares)) {
        return true;
    } else if ((square1 & blackSquares) && (square2 & blackSquares)) {
        return true;
    }
    return false;
}

bool insufficientMaterial(bitboards_t *BITBOARDS) {
    // checks if there is enough material left to mate
    if (bitCount(BITBOARDS->allPieces) == 2) {
        // only kings left
        return true;

    } else if ((BITBOARDS->whitePawns | BITBOARDS->blackPawns) == 0) {
        // no pawns that could promote
        if ((BITBOARDS->whiteRooks | BITBOARDS->blackRooks | BITBOARDS->whiteQueens | BITBOARDS->blackQueens) == 0) {
            // no queens or rooks
            if (bitCount(BITBOARDS->allPieces) == 3) {
                // only kings and one minor piece left
                return true;
            } else if (bitCount(BITBOARDS->allPieces) == 4) {
                // both players have one minor piece left and if those are bishops they are on the same color
                // note that in theory it is possible to mate with two knights and also with bishop vs bishop on different colors
                if (bitCount(BITBOARDS->whiteBishops) == 1 && bitCount(BITBOARDS->blackBishops) == 1 && sameSquareColor(BITBOARDS->whiteBishops, BITBOARDS->blackBishops)) {
                    return true;
                }
            }
        }
    }

    return false;
}

void resetSearch() {
    searchMovesPlayed = 0;
    evalCalls = 0;
    moveCalls = 0;
    possibleCalls = 0;
    quiescenceCalls = 0;
    nodes = 0;
    quietNodes = 0;
    transpositions = 0;
    quietTranspositions = 0;
    for (int i = 0; i < 128; i++) killerMoves[i] = (move_t) 0;
}

int quiescenceSearch(struct bitboards_t *BITBOARDS, int alpha, int beta, int depth) {
    // continues the search until a quiet position is reached so that the evaluation will not be able to change much in the next few moves
    quietVisits++;
    quietNodes++;

    int value;

    if (tableGetEntry(quietTable, BITBOARDS->hash, depth, &value, alpha, beta)) {
        quietTranspositions++;
        return value;
    }

    if (quietNodes & 0b11111111111) {
        if (((double)(clock() - searchStartTime)) / (CLOCKS_PER_SEC / 1000) > maxTime*1000) {
            return 0;
        }
    }

    int whiteAttacks[64] = {0};
    int blackAttacks[64] = {0};

    move_t moves[MAX_NUM_MOVES];
    int moveCount = 0;
    
    moveCount = getAttacks(BITBOARDS, &moves[0], &whiteAttacks[0], &blackAttacks[0]);

    // start with the normal evaluation since the players can not be forced to capture
    evalCalls++;

    if (drawEvalByMaterial(BITBOARDS)) {
        value = 0;

    } else {
        value = 0;

        int evalWhite = 0;
        int evalBlack = 0;

        // general strategies
        // todo: figure out the best weights for these

        int doubledPawnsWhite = 0;
        int doubledPawnsBlack = 0;
        int tripledPawnsWhite = 0;
        int tripledPawnsBlack = 0;

        doubledPawnsWhite += bitCount(BITBOARDS->whitePawns & AFILE) == 2;
        doubledPawnsWhite += bitCount(BITBOARDS->whitePawns & BFILE) == 2;
        doubledPawnsWhite += bitCount(BITBOARDS->whitePawns & CFILE) == 2;
        doubledPawnsWhite += bitCount(BITBOARDS->whitePawns & DFILE) == 2;
        doubledPawnsWhite += bitCount(BITBOARDS->whitePawns & EFILE) == 2;
        doubledPawnsWhite += bitCount(BITBOARDS->whitePawns & FFILE) == 2;
        doubledPawnsWhite += bitCount(BITBOARDS->whitePawns & GFILE) == 2;
        doubledPawnsWhite += bitCount(BITBOARDS->whitePawns & HFILE) == 2;

        doubledPawnsBlack += bitCount(BITBOARDS->blackPawns & AFILE) == 2;
        doubledPawnsBlack += bitCount(BITBOARDS->blackPawns & BFILE) == 2;
        doubledPawnsBlack += bitCount(BITBOARDS->blackPawns & CFILE) == 2;
        doubledPawnsBlack += bitCount(BITBOARDS->blackPawns & DFILE) == 2;
        doubledPawnsBlack += bitCount(BITBOARDS->blackPawns & EFILE) == 2;
        doubledPawnsBlack += bitCount(BITBOARDS->blackPawns & FFILE) == 2;
        doubledPawnsBlack += bitCount(BITBOARDS->blackPawns & GFILE) == 2;
        doubledPawnsBlack += bitCount(BITBOARDS->blackPawns & HFILE) == 2;

        tripledPawnsWhite += bitCount(BITBOARDS->whitePawns & AFILE) == 3;
        tripledPawnsWhite += bitCount(BITBOARDS->whitePawns & BFILE) == 3;
        tripledPawnsWhite += bitCount(BITBOARDS->whitePawns & CFILE) == 3;
        tripledPawnsWhite += bitCount(BITBOARDS->whitePawns & DFILE) == 3;
        tripledPawnsWhite += bitCount(BITBOARDS->whitePawns & EFILE) == 3;
        tripledPawnsWhite += bitCount(BITBOARDS->whitePawns & FFILE) == 3;
        tripledPawnsWhite += bitCount(BITBOARDS->whitePawns & GFILE) == 3;
        tripledPawnsWhite += bitCount(BITBOARDS->whitePawns & HFILE) == 3;

        tripledPawnsBlack += bitCount(BITBOARDS->blackPawns & AFILE) == 3;
        tripledPawnsBlack += bitCount(BITBOARDS->blackPawns & BFILE) == 3;
        tripledPawnsBlack += bitCount(BITBOARDS->blackPawns & CFILE) == 3;
        tripledPawnsBlack += bitCount(BITBOARDS->blackPawns & DFILE) == 3;
        tripledPawnsBlack += bitCount(BITBOARDS->blackPawns & EFILE) == 3;
        tripledPawnsBlack += bitCount(BITBOARDS->blackPawns & FFILE) == 3;
        tripledPawnsBlack += bitCount(BITBOARDS->blackPawns & GFILE) == 3;
        tripledPawnsBlack += bitCount(BITBOARDS->blackPawns & HFILE) == 3;

        evalWhite -= doubledPawnsWhite * 50;
        evalWhite -= tripledPawnsWhite * 100;
        evalBlack += doubledPawnsBlack * 50;
        evalBlack += tripledPawnsBlack * 100;

        // pawn structure

        int isolatedPawnsWhite = 0;
        int isolatedPawnsBlack = 0;

        isolatedPawnsWhite += ((BITBOARDS->whitePawns & AFILE) && !(BITBOARDS->whitePawns & BFILE));
        isolatedPawnsWhite += ((BITBOARDS->whitePawns & BFILE) && !(BITBOARDS->whitePawns & AFILE) && !(BITBOARDS->whitePawns & CFILE));
        isolatedPawnsWhite += ((BITBOARDS->whitePawns & CFILE) && !(BITBOARDS->whitePawns & BFILE) && !(BITBOARDS->whitePawns & DFILE));
        isolatedPawnsWhite += ((BITBOARDS->whitePawns & DFILE) && !(BITBOARDS->whitePawns & CFILE) && !(BITBOARDS->whitePawns & EFILE));
        isolatedPawnsWhite += ((BITBOARDS->whitePawns & EFILE) && !(BITBOARDS->whitePawns & DFILE) && !(BITBOARDS->whitePawns & FFILE));
        isolatedPawnsWhite += ((BITBOARDS->whitePawns & FFILE) && !(BITBOARDS->whitePawns & EFILE) && !(BITBOARDS->whitePawns & GFILE));
        isolatedPawnsWhite += ((BITBOARDS->whitePawns & GFILE) && !(BITBOARDS->whitePawns & FFILE) && !(BITBOARDS->whitePawns & HFILE));
        isolatedPawnsWhite += ((BITBOARDS->whitePawns & HFILE) && !(BITBOARDS->whitePawns & GFILE));

        isolatedPawnsBlack += ((BITBOARDS->blackPawns & AFILE) && !(BITBOARDS->blackPawns & BFILE));
        isolatedPawnsBlack += ((BITBOARDS->blackPawns & BFILE) && !(BITBOARDS->blackPawns & AFILE) && !(BITBOARDS->blackPawns & CFILE));
        isolatedPawnsBlack += ((BITBOARDS->blackPawns & CFILE) && !(BITBOARDS->blackPawns & BFILE) && !(BITBOARDS->blackPawns & DFILE));
        isolatedPawnsBlack += ((BITBOARDS->blackPawns & DFILE) && !(BITBOARDS->blackPawns & CFILE) && !(BITBOARDS->blackPawns & EFILE));
        isolatedPawnsBlack += ((BITBOARDS->blackPawns & EFILE) && !(BITBOARDS->blackPawns & DFILE) && !(BITBOARDS->blackPawns & FFILE));
        isolatedPawnsBlack += ((BITBOARDS->blackPawns & FFILE) && !(BITBOARDS->blackPawns & EFILE) && !(BITBOARDS->blackPawns & GFILE));
        isolatedPawnsBlack += ((BITBOARDS->blackPawns & GFILE) && !(BITBOARDS->blackPawns & FFILE) && !(BITBOARDS->blackPawns & HFILE));
        isolatedPawnsBlack += ((BITBOARDS->blackPawns & HFILE) && !(BITBOARDS->blackPawns & GFILE));

        evalWhite -= isolatedPawnsWhite * 25;
        evalBlack += isolatedPawnsBlack * 25;

        // bishop pair

        evalWhite += (bitCount(BITBOARDS->whiteBishops) >= 2) * 30;
        evalBlack -= (bitCount(BITBOARDS->blackBishops) >= 2) * 30;

        // rook on open file
        // means there are no pawns on the file

        int rooksOnOpenFileWhite = 0;
        int rooksOnOpenFileBlack = 0;

        rooksOnOpenFileWhite += (BITBOARDS->whiteRooks & AFILE) && !((BITBOARDS->whitePawns | BITBOARDS->blackPawns) & AFILE);
        rooksOnOpenFileWhite += (BITBOARDS->whiteRooks & BFILE) && !((BITBOARDS->whitePawns | BITBOARDS->blackPawns) & BFILE);
        rooksOnOpenFileWhite += (BITBOARDS->whiteRooks & CFILE) && !((BITBOARDS->whitePawns | BITBOARDS->blackPawns) & CFILE);
        rooksOnOpenFileWhite += (BITBOARDS->whiteRooks & DFILE) && !((BITBOARDS->whitePawns | BITBOARDS->blackPawns) & DFILE);
        rooksOnOpenFileWhite += (BITBOARDS->whiteRooks & EFILE) && !((BITBOARDS->whitePawns | BITBOARDS->blackPawns) & EFILE);
        rooksOnOpenFileWhite += (BITBOARDS->whiteRooks & FFILE) && !((BITBOARDS->whitePawns | BITBOARDS->blackPawns) & FFILE);
        rooksOnOpenFileWhite += (BITBOARDS->whiteRooks & GFILE) && !((BITBOARDS->whitePawns | BITBOARDS->blackPawns) & GFILE);
        rooksOnOpenFileWhite += (BITBOARDS->whiteRooks & HFILE) && !((BITBOARDS->whitePawns | BITBOARDS->blackPawns) & HFILE);

        rooksOnOpenFileBlack += (BITBOARDS->blackRooks & AFILE) && !((BITBOARDS->whitePawns | BITBOARDS->blackPawns) & AFILE);
        rooksOnOpenFileBlack += (BITBOARDS->blackRooks & BFILE) && !((BITBOARDS->whitePawns | BITBOARDS->blackPawns) & BFILE);
        rooksOnOpenFileBlack += (BITBOARDS->blackRooks & CFILE) && !((BITBOARDS->whitePawns | BITBOARDS->blackPawns) & CFILE);
        rooksOnOpenFileBlack += (BITBOARDS->blackRooks & DFILE) && !((BITBOARDS->whitePawns | BITBOARDS->blackPawns) & DFILE);
        rooksOnOpenFileBlack += (BITBOARDS->blackRooks & EFILE) && !((BITBOARDS->whitePawns | BITBOARDS->blackPawns) & EFILE);
        rooksOnOpenFileBlack += (BITBOARDS->blackRooks & FFILE) && !((BITBOARDS->whitePawns | BITBOARDS->blackPawns) & FFILE);
        rooksOnOpenFileBlack += (BITBOARDS->blackRooks & GFILE) && !((BITBOARDS->whitePawns | BITBOARDS->blackPawns) & GFILE);
        rooksOnOpenFileBlack += (BITBOARDS->blackRooks & HFILE) && !((BITBOARDS->whitePawns | BITBOARDS->blackPawns) & HFILE);

        // semi open means there is only one color for pawns on the file

        int rooksOnSemiOpenFileWhite = 0;
        int rooksOnSemiOpenFileBlack = 0;

        rooksOnSemiOpenFileWhite += ((BITBOARDS->whiteRooks & AFILE) && !(BITBOARDS->whitePawns & BITBOARDS->blackPawns & AFILE));
        rooksOnSemiOpenFileWhite += ((BITBOARDS->whiteRooks & BFILE) && !(BITBOARDS->whitePawns & BITBOARDS->blackPawns & BFILE));
        rooksOnSemiOpenFileWhite += ((BITBOARDS->whiteRooks & CFILE) && !(BITBOARDS->whitePawns & BITBOARDS->blackPawns & CFILE));
        rooksOnSemiOpenFileWhite += ((BITBOARDS->whiteRooks & DFILE) && !(BITBOARDS->whitePawns & BITBOARDS->blackPawns & DFILE));
        rooksOnSemiOpenFileWhite += ((BITBOARDS->whiteRooks & EFILE) && !(BITBOARDS->whitePawns & BITBOARDS->blackPawns & EFILE));
        rooksOnSemiOpenFileWhite += ((BITBOARDS->whiteRooks & FFILE) && !(BITBOARDS->whitePawns & BITBOARDS->blackPawns & FFILE));
        rooksOnSemiOpenFileWhite += ((BITBOARDS->whiteRooks & GFILE) && !(BITBOARDS->whitePawns & BITBOARDS->blackPawns & GFILE));
        rooksOnSemiOpenFileWhite += ((BITBOARDS->whiteRooks & HFILE) && !(BITBOARDS->whitePawns & BITBOARDS->blackPawns & HFILE));

        rooksOnSemiOpenFileBlack += ((BITBOARDS->blackRooks & AFILE) && !(BITBOARDS->whitePawns & BITBOARDS->blackPawns & AFILE));
        rooksOnSemiOpenFileBlack += ((BITBOARDS->blackRooks & BFILE) && !(BITBOARDS->whitePawns & BITBOARDS->blackPawns & BFILE));
        rooksOnSemiOpenFileBlack += ((BITBOARDS->blackRooks & CFILE) && !(BITBOARDS->whitePawns & BITBOARDS->blackPawns & CFILE));
        rooksOnSemiOpenFileBlack += ((BITBOARDS->blackRooks & DFILE) && !(BITBOARDS->whitePawns & BITBOARDS->blackPawns & DFILE));
        rooksOnSemiOpenFileBlack += ((BITBOARDS->blackRooks & EFILE) && !(BITBOARDS->whitePawns & BITBOARDS->blackPawns & EFILE));
        rooksOnSemiOpenFileBlack += ((BITBOARDS->blackRooks & FFILE) && !(BITBOARDS->whitePawns & BITBOARDS->blackPawns & FFILE));
        rooksOnSemiOpenFileBlack += ((BITBOARDS->blackRooks & GFILE) && !(BITBOARDS->whitePawns & BITBOARDS->blackPawns & GFILE));
        rooksOnSemiOpenFileBlack += ((BITBOARDS->blackRooks & HFILE) && !(BITBOARDS->whitePawns & BITBOARDS->blackPawns & HFILE));

        evalWhite += rooksOnOpenFileWhite * 20;
        evalWhite += rooksOnSemiOpenFileWhite * 10;
        evalBlack -= rooksOnOpenFileBlack * 20;
        evalBlack -= rooksOnSemiOpenFileBlack * 10;

        // king safety
        // having more pieces around the king is good

        u64 kingRingWhite = kingAttacks[lsb(BITBOARDS->whiteKing)];
        u64 kingRingBlack = kingAttacks[lsb(BITBOARDS->blackKing)];

        evalWhite += bitCount(kingRingWhite & BITBOARDS->whitePieces) * 10;
        evalBlack -= bitCount(kingRingBlack & BITBOARDS->blackPieces) * 10;

        // more attacks in the danger zuone of the king is, well, dangerous

        int dangerWhite = 0;
        int dangerBlack = 0;
        
        // the kingRing gets weighted twice as it is also inside the danger mask

        while (kingRingWhite) {
            u8 b = lsb(kingRingWhite);
            flipBit(kingRingWhite, b);
            dangerWhite += blackAttacks[b];
        }

        while (kingRingBlack) {
            u8 b = lsb(kingRingBlack);
            flipBit(kingRingBlack, b);
            dangerBlack += whiteAttacks[b];
        }

        u64 dangerMaskWhite = dangerMasks[lsb(BITBOARDS->whiteKing)];
        u64 dangerMaskBlack = dangerMasks[lsb(BITBOARDS->blackKing)];

        while (dangerMaskWhite) {
            u8 b = lsb(dangerMaskWhite);
            flipBit(dangerMaskWhite, b);
            dangerWhite += blackAttacks[b];
        }

        while (dangerMaskBlack) {
            u8 b = lsb(dangerMaskBlack);
            flipBit(dangerMaskBlack, b);
            dangerBlack += whiteAttacks[b];
        }

        evalWhite -= dangerWhite * 25;
        evalBlack += dangerBlack * 25;

        // mobility based on how many squares are under attack by both players

        for (int i = 0; i < 64; i++) {
            // if both players fight for a square, the one with more attackers gets the bonus
            // squares in the center are worth more
            evalWhite += (centerBonus[i] * (whiteAttacks[i] - blackAttacks[i]) / 2);
            evalBlack += (centerBonus[i] * (whiteAttacks[i] - blackAttacks[i]) / 2);
        }

        if (((BITBOARDS->whiteEvalOpening + abs(BITBOARDS->blackEvalOpening)) + (4000 / fullMoveCount)) < 4000) {
            // important for endgame eval
            evalWhite += BITBOARDS->whiteEvalEndgame;
            evalBlack += BITBOARDS->blackEvalEndgame;

            int distBetweenKings = abs(lsb(BITBOARDS->blackKing) / 8 - lsb(BITBOARDS->whiteKing) / 8) + abs(lsb(BITBOARDS->blackKing) % 8 - lsb(BITBOARDS->whiteKing) % 8);

            if (evalWhite > abs(evalBlack)) {

                if (distBetweenKings < 4) {
                    evalWhite -= 10 * distBetweenKings;
                } else {
                    evalWhite -= 10 * distBetweenKings * 2;
                }

                evalWhite += kingEvalWhiteEndgame[lsb(BITBOARDS->whiteKing)] / 2;

            } else if (abs(evalBlack) > evalWhite) {
                    
                if (distBetweenKings < 4) {
                    evalBlack += 10 * distBetweenKings;
                } else {
                    evalBlack += 10 * distBetweenKings * 2;
                }

                evalBlack += kingEvalBlackEndgame[lsb(BITBOARDS->blackKing)] / 2;
            }

        } else if (((BITBOARDS->whiteEvalOpening + abs(BITBOARDS->blackEvalOpening)) + (2000 / fullMoveCount)) < 6000) {
            // middle game eval
            evalWhite += (BITBOARDS->whiteEvalOpening + BITBOARDS->whiteEvalEndgame) / 2;
            evalBlack = (BITBOARDS->blackEvalOpening + BITBOARDS->blackEvalEndgame) / 2;
            // exposed king is even worse in the middle game
            if (bitCount(kingRingWhite) < 2) {
                evalWhite -= 100;
            }
            if (bitCount(kingRingBlack) < 2) {
                evalBlack += 100;
            }

        } else {
            // opening eval
            evalWhite += BITBOARDS->whiteEvalOpening;
            evalBlack += BITBOARDS->blackEvalOpening;

            // todo: add some stuff here like moving the same piece twice in a row
        }

        value += ((int)(((evalWhite + evalBlack) * 17700) / (evalWhite - evalBlack + 10000))) * (BITBOARDS->color ? 1 : -1);

    }

    if (value >= beta) {
        tableSetEntry(quietTable, BITBOARDS->hash, depth, value, LOWERBOUND);
        return beta;
    }
    if (value > alpha) {
        alpha = value;
    }

    orderCaptures(&moves[0], BITBOARDS, moveCount);
    
    int flag = UPPERBOUND;
    
    for (int i = 0; i < moveCount; i++) {

        move_t move = moves[i];
        struct undo_t undo;
        // the data that is necessary to undo the move later is stored in the undo struct
        doMove(move, BITBOARDS, &undo);

        if (canCaptureOpponentsKing(BITBOARDS)) {
            // after doing the move the player to move is the opponent
            // so we need to check if the opponent can capture its opponents king
            // which means the move would leave the king in check and is therefore illegal
            undoMove(move, BITBOARDS, &undo);
            continue;
        }

        // now the side to move has changed and the sign of the evaluation needs to be swapped
        // we also need to change the sign of alpha and beta
        // this is because alpha is the best assured score for the side to move and beta is the best assured score for the side not to move
        // since the side to move has changed the assured scores have changed as well
        int value = -quiescenceSearch(BITBOARDS, -beta, -alpha, depth - 1);

        undoMove(move, BITBOARDS, &undo);
                
        // update alpha and try to prune

        // delta pruning
        int delta = 890;
        if (mPromo(move)) {
            delta += 800;
        }

        if (value < alpha - delta) {
            return alpha;
        }
        
        if (value >= beta) {
            tableSetEntry(quietTable, BITBOARDS->hash, depth, value, LOWERBOUND);
            tableSetMove(quietTable, BITBOARDS->hash, depth, moves[i]);
            return beta;
        }

        if (value > alpha) {
            alpha = value;
            flag = EXACT;
            tableSetMove(quietTable, BITBOARDS->hash, depth, moves[i]);
        }
    }

    // return the best guaranteed score
    tableSetEntry(quietTable, BITBOARDS->hash, depth, alpha, flag);
    return alpha;
}

bool isRepetition(struct bitboards_t *BITBOARDS) {
    // check if the position has already occoured during the game
    for (int i = 0; i < gameMovesPlayed; i++) {
        if (gameHistory[i] == BITBOARDS->hash) {
            return true;
        }
    }
    return false;
}

bool isThreeFoldRepetition(struct bitboards_t *BITBOARDS) {
    // check if the position has already occoured three times during the game
    int count = 0;
    for (int i = 0; i < gameMovesPlayed; i++) {
        if (gameHistory[i] == BITBOARDS->hash) {
            count++;
        }
    }
    return count >= 3;
}

int futilityMargin[4] = {
    0, 90, 310, 490
};

// used for futility pruning
// futility pruning is a way to prune moves that are very unlikely to improve the position (at lower depths)
// we add the margin to the static evaluation and if the result is smaller than alpha we dont need to search the move

// for example, at depth 1 we add 100 to the static evaluation and if thats not good enough we dont need to search the move
// of course this only makes sence if the move is not a capture or a promotion and only changing the position slightly

// for the other pruning methods that follow, please check the respective articles on chessprogramming.org

int negaMax(struct bitboards_t *BITBOARDS, int depth, int alpha, int beta, bool allowNullMove) {
    // negamax framework, works the same as the classic minimax but withouth the need to check if its the maximizing or minimizing player

    /*
    at its core it looks like this:

    if (depth <= 0) {
        return quiescenceSearch(BITBOARDS, alpha, beta, 0);
    }

    generate moves

    for each (legal) move in moves {
        do move
        value = -negaMax(BITBOARDS, depth - 1, -beta, -alpha)
        undo move

        if (value >= beta) {
            return beta;
        }
        if (value > alpha) {
            alpha = value;
        }
    }

    return alpha
    */
    // the rest is just to make it more efficient in a way that the good moves are prioritized

    nodes++;
    visits++;

    int value;
    bool futilityPruning = false;

    if (tableGetEntry(transTable, BITBOARDS->hash, depth, &value, alpha, beta)) {

        transpositions++;
        return value;
    }

    if (nodes & 0b11111111111) {
        // every 2048 nodes we check if the time is up
        if (((double)(clock() - searchStartTime)) / (CLOCKS_PER_SEC / 1000) > maxTime*1000) {
            return 0;
        }
    }

    if (isRepetition(BITBOARDS)) {
        // as repetition mostly leads to repetition we can assume that the position is a draw
        return 0;
    }

    if (depth <= 0) {
        // when the depth limit is reached we use quiescence search to get a more accurate evaluation
        quiescenceCalls++;
        return quiescenceSearch(BITBOARDS, alpha, beta, 0);
    }

    struct undo_t undo;

    bool inCheck = isInCheck(BITBOARDS);

    if (inCheck) {
        // extend the search if the king is in check as when in check there are less moves that can be searched
        // and we dont want to miss a forced mate easily
        // depth++;

    } else {

        int eval;
        if (drawEvalByMaterial(BITBOARDS)) {
            eval = 0;

        } else {
            int evalWhite;
            int evalBlack;

            if (((BITBOARDS->whiteEvalOpening + abs(BITBOARDS->blackEvalOpening)) + (4000 / fullMoveCount)) < 4000) {

                evalWhite = BITBOARDS->whiteEvalEndgame;
                evalBlack = BITBOARDS->blackEvalEndgame;

            } else if (((BITBOARDS->whiteEvalOpening + abs(BITBOARDS->blackEvalOpening)) + (2000 / fullMoveCount)) < 6000) {

                evalWhite = (BITBOARDS->whiteEvalOpening + BITBOARDS->whiteEvalEndgame) / 2;
                evalBlack = (BITBOARDS->blackEvalOpening + BITBOARDS->blackEvalEndgame) / 2;

            } else {
                evalWhite = BITBOARDS->whiteEvalOpening;
                evalBlack = BITBOARDS->blackEvalOpening;
            }

            eval = ((int)(((evalWhite + evalBlack) * 17700) / (evalWhite - evalBlack + 10000))) * (BITBOARDS->color ? 1 : -1);
        }

        // evalution pruning
        if (depth < 3 && abs(beta - 1) > -INF + 100) {
            int evalMargin = 100 * depth;
            if (eval - evalMargin >= beta) {
                return eval - evalMargin;
            }
        }

        // null move pruning
        if (depth >= 3 && allowNullMove) {
        
            BITBOARDS->color = !BITBOARDS->color;
            u64 ep = BITBOARDS->enPassantSquare;
            if (BITBOARDS->enPassantSquare >= 0) {
                BITBOARDS->hash ^= ZOBRIST_TABLE[BITBOARDS->enPassantSquare][12];
            }
            BITBOARDS->enPassantSquare = -1;
            BITBOARDS->hash ^= whiteToMove;

            value = -negaMax(BITBOARDS, depth - 1 - 2, -beta, -beta + 1, false);
            
            BITBOARDS->hash ^= whiteToMove;
            BITBOARDS->color = !BITBOARDS->color;
            BITBOARDS->enPassantSquare = ep;
            if (BITBOARDS->enPassantSquare >= 0) {
                BITBOARDS->hash ^= ZOBRIST_TABLE[BITBOARDS->enPassantSquare][12];
            }

            if (value >= beta) {
                return beta;
            }
        }

        // razoring
        value = eval + 125;
        int newValue;
        
        if (value < beta) {
            if (depth == 1) {
                newValue = quiescenceSearch(BITBOARDS, alpha, beta, 0);
                quiescenceCalls++;
                return (newValue > value) ? newValue : value;
            }
        }
        
        value += 175;

        if (value < beta && depth < 4) {
            newValue = quiescenceSearch(BITBOARDS, alpha, beta, 0);
            quiescenceCalls++;
            if (newValue < beta) {return (newValue > value) ? newValue : value;}
        }

        // futility pruning condition
        if (depth < 4 && abs(alpha) < INF && eval + futilityMargin[depth] <= alpha) {
            futilityPruning = true;
        }
    }

    move_t moves[MAX_NUM_MOVES];
    int moveCount = 0;

    if (BITBOARDS->color) { // white
        moveCount = possiblemoves(
            BITBOARDS->color, 
            BITBOARDS->allPieces, BITBOARDS->enPassantSquare, BITBOARDS->whitePieces, BITBOARDS->blackPieces, 
            BITBOARDS->whitePawns, BITBOARDS->whiteKnights, BITBOARDS->whiteBishops, BITBOARDS->whiteRooks, BITBOARDS->whiteQueens, BITBOARDS->whiteKing, 
            BITBOARDS->whiteCastleQueenSide, BITBOARDS->whiteCastleKingSide, &moves[0]
        );
    } else { // black
        moveCount = possiblemoves(
            BITBOARDS->color, 
            BITBOARDS->allPieces, BITBOARDS->enPassantSquare, BITBOARDS->blackPieces, BITBOARDS->whitePieces, 
            BITBOARDS->blackPawns, BITBOARDS->blackKnights, BITBOARDS->blackBishops, BITBOARDS->blackRooks, BITBOARDS->blackQueens, BITBOARDS->blackKing, 
            BITBOARDS->blackCastleQueenSide, BITBOARDS->blackCastleKingSide, &moves[0]
        );
    }

    orderMoves(&moves[0], BITBOARDS, moveCount);

    int legalMoves = 0;
    int flag = UPPERBOUND;

    for (int i = 0; i < moveCount; i++) {

        move_t move = moves[i];

        if (mCastle(move) && (isIllegalCastle(move, BITBOARDS) || inCheck)) {
            continue;
        }

        doMove(move, BITBOARDS, &undo);

        if (canCaptureOpponentsKing(BITBOARDS)) {
            undoMove(move, BITBOARDS, &undo);
            continue;
        }

        // futility pruning
        if (futilityPruning && legalMoves && undo.capturedPiece == 0 && mPromo(move) == 0 && !isInCheck(BITBOARDS)) {
            undoMove(move, BITBOARDS, &undo);
            continue;
        }

        if (legalMoves == 0) {
            value = -negaMax(BITBOARDS, depth - 1, -beta, -alpha, true);
        } else {
            // late move reduction
            if (
                legalMoves > 3 && depth > 2 && !inCheck && undo.capturedPiece == 0 && mPromo(move) == 0 &&
                (mFrom(move) != mFrom(killerMoves[searchMovesPlayed]) || mTo(move) != mFrom(killerMoves[searchMovesPlayed])) && 
                (mFrom(move) != mFrom(killerMoves[searchMovesPlayed + 1]) || mTo(move) != mFrom(killerMoves[searchMovesPlayed + 1]))
            ) {
                value = -negaMax(BITBOARDS, depth - 2, -alpha - 1, -alpha, true);
            } else {
                value = alpha + 1;
            }

            if (value > alpha) {
                value = -negaMax(BITBOARDS, depth - 1, -alpha - 1, -alpha, true);
                
                if (value > alpha && value < beta) {
                    value = -negaMax(BITBOARDS, depth - 1, -beta, -alpha, true);
                }
            }
        }

        undoMove(move, BITBOARDS, &undo);

        legalMoves++;

        if (value >= beta) {
            tableSetEntry(transTable, BITBOARDS->hash, depth, value, LOWERBOUND);
            
            tableSetMove(transTable, BITBOARDS->hash, depth, move);

            if (undo.capturedPiece == 0) {

                killerMoves[64+searchMovesPlayed] = killerMoves[searchMovesPlayed];
                killerMoves[searchMovesPlayed] = move;

            }

            return beta;
        }

        if (value > alpha) {
            alpha = value;
            flag = EXACT;
            tableSetMove(transTable, BITBOARDS->hash, depth, move);
        }
    }

    if (legalMoves == 0) {
        if (isInCheck(BITBOARDS)) {
            // mate
            return -INF + depth;
        } else {
            // stalemate
            return 0;
        }
    }

    tableSetEntry(transTable, BITBOARDS->hash, depth, alpha, flag);

    return alpha;
}

move_t iterDeepening(move_t *possible, struct bitboards_t *bitboards, int numMoves) {
    // find the best move using iterative deepening
    // this also includes the principal variation search which could also be used in the negamax function
    move_t best;
    int values[numMoves];

    // ordering moves here is not necessary because at depth 0 the moves will be ordered by the orderMoves function
    searchStartTime = clock();
    
    bool stopSearch = false;
    int s = bitboards->color ? -1 : 1;
    int bestScore = 0;

    printf("pseudo legal move amount: %d\n", numMoves);

    for (int depth = 0; depth < maxDepth; depth++) {
        // aspiration window
        int low = 25;
        int high = 25;

        while (1) {
            // try searching with the window until it fits
            int alpha = bestScore - low;
            int beta = bestScore + high;

            printf("Depth: %d - \n", depth+1);

            for (int i = 0; i < numMoves; i++) {

                int moveEval;

                if (stopSearch) {
                    moveEval = 0;
                } else {
                    visits = 0;
                    quietVisits = 0;

                    if (mCastle(possible[i]) && (isIllegalCastle(possible[i], bitboards) || isInCheck(bitboards))) {
                        values[i] = s * INF;
                        continue;
                    }

                    struct undo_t undo;
                    doMove(possible[i], bitboards, &undo);

                    if (canCaptureOpponentsKing(bitboards)) {
                        values[i] = s * INF;
                        undoMove(possible[i], bitboards, &undo);
                        continue;
                    }

                    printf("%s%s: ", notation[mFrom(possible[i])], notation[mTo(possible[i])]);
                    int t = transpositions;

                    moveEval = s * negaMax(bitboards, depth, -INF, INF, true); // start negamax search to evaluate the pv move
                    undoMove(possible[i], bitboards, &undo);

                    printf("%-10d quiet: %-10d eval: %d transpositions: %d\n", visits, quietVisits, moveEval, transpositions - t);
                }

                values[i] = moveEval;

                if (((double)(clock() - searchStartTime)) / (CLOCKS_PER_SEC / 1000) > maxTime*1000) {
                    printf("time out\n");
                    break;

                } else if ((bitboards->color && (moveEval >= 99000)) || (!bitboards->color && (moveEval <= -99000))) {
                    printf("checkmate found\n");
                    return possible[i];
                    stopSearch = true;
                }
            }

            if (((double)(clock() - searchStartTime)) / (CLOCKS_PER_SEC / 1000) > maxTime*1000) {
                break;
            }

            if (bitboards->color) {
                quickSortArrayDec(&possible[0], values, 0, numMoves-1);
            } else {
                quickSortArrayInc(&possible[0], values, 0, numMoves-1);
            }

            printf("\n>>> best move: %s%s value: %d\n\n", notation[mFrom(possible[0])], notation[mTo(possible[0])], values[0]);
            bestScore = values[0];
            tableSetMove(transTable, bitboards->hash, depth, possible[0]);

            // aspiration window fail
            if (bestScore == alpha) {
                low *= 4;
                printf("repeating search...\n");
            }
            else if (bestScore == beta) {
                printf("repeating search...\n");
                high *= 4;
            }
            else {
                // aspiration window success, break out of the loop
                break;
            }

            if (numMoves == 1 || numMoves >= 2 && values[1] == s * INF) {
                printf("move is forced\n");
                return possible[0];
            }
        }
        if (((double)(clock() - searchStartTime)) / (CLOCKS_PER_SEC / 1000) > maxTime*1000) {
            break;
        }
    }

    printf("\ntranspositions found: %d (%d quiet)\n", transpositions, quietTranspositions);

    best = possible[0];
    
    if (values[0] == -100000 || values[0] == 100000) {
        // this happens on a position where the engine is already checkmated
        // which should never be the case
        move_t move = 0;
        return move;
    } else {
        return best;
    }
}

move_t getBookMove(u64 hash) {

    if (useBook && gameMovesPlayed < 20) {
        // > 20 is not necessary because the book is not that big

        struct book_t page = bookEntries[hash % BOOK_SIZE];

        printf("book entries: %d\n", page.numEntries);

        for (int i = 0; i < page.numEntries; i++) {
            // read all entries on the page
            if (page.entries[i].hash == hash) {

                printf("num moves: %d\n", page.entries[i].numMoves);

                quickSortArrayDec(page.entries[i].moves, page.entries[i].occourences, 0, page.entries[i].numMoves - 1);

                for (int j = 0; j < page.entries[i].numMoves; j++) {
                    printf("%s%s %d\n", notation[mFrom(page.entries[i].moves[j])], notation[mTo(page.entries[i].moves[j])], page.entries[i].occourences[j]);
                }
                // randomize the move selection
                int index;
                if (page.entries[i].numMoves > 3) {
                    // moves are weighted by occourences
                    if (rand() % 100 > 10) {
                        // 90% chance to pick the first move
                        index = 0;
                    } else {
                        if (rand() % 100 > 10) {
                            // 9% chance to pick the second move
                            index = 1;
                        } else {
                            // 1% chance to pick the third move
                            index = 2;
                        }
                    }
                    printf("random index: %d\n", index);
                } else {
                    index = 0;
                }

                return page.entries[i].moves[index];
            }
        }

        printf("out of book\n");
    }
    
    // "null" move
    return 0;
}

void engineMove() {
    // find and make the engine move
    clock_t startTime = clock();
    move_t best;
    // probe the book
    move_t bookMove = getBookMove(bitboards->hash);
    int numMoves = 0;
    if (bookMove != 0) {
        best = bookMove;

    } else {
        move_t possible[MAX_NUM_MOVES];
        if (bitboards->color) {
            numMoves = possiblemoves(
                bitboards->color, 
                bitboards->allPieces, bitboards->enPassantSquare, bitboards->whitePieces, bitboards->blackPieces, 
                bitboards->whitePawns, bitboards->whiteKnights, bitboards->whiteBishops, bitboards->whiteRooks, bitboards->whiteQueens, bitboards->whiteKing, 
                bitboards->whiteCastleQueenSide, bitboards->whiteCastleKingSide, &possible[0]
            );
        } else {
            numMoves = possiblemoves(
                bitboards->color, 
                bitboards->allPieces, bitboards->enPassantSquare, bitboards->blackPieces, bitboards->whitePieces, 
                bitboards->blackPawns, bitboards->blackKnights, bitboards->blackBishops, bitboards->blackRooks, bitboards->blackQueens, bitboards->blackKing, 
                bitboards->blackCastleQueenSide, bitboards->blackCastleKingSide, &possible[0]
            );
        }

        best = iterDeepening(possible, bitboards, numMoves);
    }

    if (best != 0) { // if there is a legal move
        updateFenClocks(best);

        struct undo_t undo;
        doMove(best, bitboards, &undo);

        if (isThreeFoldRepetition(bitboards)) {
            printf("threefold repetition\n");
            freeTables();
            freeBoards();
            if (useBook) {
                free(bookEntries);
            }
            exit(0);
        }
        gameHistory[gameMovesPlayed++] = bitboards->hash;

        printf("final move: %s%s\n", notation[mFrom(best)], notation[mTo(best)]);
        int s = bitboards->color ? 1 : -1;
        printf("quick evaluation: %d\n", s * quiescenceSearch(bitboards, -INF, INF, 0));
        printf("board after move\n");
        printBoard(bitboards);

        printf("whiteEvalOpening: %d\n", bitboards->whiteEvalOpening);
        printf("blackEvalOpening: %d\n", bitboards->blackEvalOpening);
        printf("fullMoveCount: %d\n", fullMoveCount);

        if (((bitboards->whiteEvalOpening + abs(bitboards->blackEvalOpening)) + (4000 / fullMoveCount)) < 4000) {
            printf("endgame\n");

        } else if (((bitboards->whiteEvalOpening + abs(bitboards->blackEvalOpening)) + (2000 / fullMoveCount)) < 6000) {
            printf("middlegame\n");

        } else {
            printf("opening\n");
        }

        printf("nodes searched: %d (%d quiet)\n", nodes, quietNodes);
    } else {
        bitboards->color = !bitboards->color;
        if (canCaptureOpponentsKing(bitboards)) {
            printf("engine is mate\n");
        } else {
            printf("draw\n");
        }
        exit(0);
    }

    // log some stats
    clock_t end_time = clock();
    double elapsed_time = ((double)(end_time - startTime)) / (CLOCKS_PER_SEC / 1000); // Convert to milliseconds
    printf("Elapsed time: %.2f milliseconds\n", elapsed_time);
    printf("nodes per second: %d (%d quiet)\n", (int)(nodes/elapsed_time*1000), (int)(quietNodes/elapsed_time*1000));
    printf("evaluations per second: %d\n", (int)(evalCalls/elapsed_time*1000));

    // checkmate detection
    move_t othermoves[MAX_NUM_MOVES];
    int numOtherMoves = 0;
    if (bitboards->color) {
        numOtherMoves = possiblemoves(
            bitboards->color, 
            bitboards->allPieces, bitboards->enPassantSquare, bitboards->whitePieces, bitboards->blackPieces, 
            bitboards->whitePawns, bitboards->whiteKnights, bitboards->whiteBishops, bitboards->whiteRooks, bitboards->whiteQueens, bitboards->whiteKing, 
            bitboards->whiteCastleQueenSide, bitboards->whiteCastleKingSide, &othermoves[0]
        );
    } else {
        numOtherMoves = possiblemoves(
            bitboards->color, 
            bitboards->allPieces, bitboards->enPassantSquare, bitboards->blackPieces, bitboards->whitePieces, 
            bitboards->blackPawns, bitboards->blackKnights, bitboards->blackBishops, bitboards->blackRooks, bitboards->blackQueens, bitboards->blackKing, 
            bitboards->blackCastleQueenSide, bitboards->blackCastleKingSide, &othermoves[0]
        );
    }

    if (!hasLegalMoves(othermoves, bitboards, numOtherMoves)) {
        bitboards->color = !bitboards->color;
        if (canCaptureOpponentsKing(bitboards)) {
            printf("player is mate\n");
        } else {
            printf("draw\n");
        }
        exit(0);
    }

    printf("--------------------function calls-------------------\n");
    printf("eval: %d moves: %d possible: %d quiet: %d\n", evalCalls, moveCalls, possibleCalls, quiescenceCalls);
    resetSearch();
}

u64 perft(int depth, struct bitboards_t *BITBOARDS, int originalDepth) {
    // perft function for debugging

    if (depth == 0) {
        return 1;
    }

    move_t possible[MAX_NUM_MOVES];
    int numMoves = 0;
    if (BITBOARDS->color) {
        numMoves = possiblemoves(
            BITBOARDS->color, 
            BITBOARDS->allPieces, BITBOARDS->enPassantSquare, BITBOARDS->whitePieces, BITBOARDS->blackPieces, 
            BITBOARDS->whitePawns, BITBOARDS->whiteKnights, BITBOARDS->whiteBishops, BITBOARDS->whiteRooks, BITBOARDS->whiteQueens, BITBOARDS->whiteKing, 
            BITBOARDS->whiteCastleQueenSide, BITBOARDS->whiteCastleKingSide, &possible[0]
        );
    } else {
        numMoves = possiblemoves(
            BITBOARDS->color, 
            BITBOARDS->allPieces, BITBOARDS->enPassantSquare, BITBOARDS->blackPieces, BITBOARDS->whitePieces, 
            BITBOARDS->blackPawns, BITBOARDS->blackKnights, BITBOARDS->blackBishops, BITBOARDS->blackRooks, BITBOARDS->blackQueens, BITBOARDS->blackKing, 
            BITBOARDS->blackCastleQueenSide, BITBOARDS->blackCastleKingSide, &possible[0]
        );
    }
    
    int nodes = 0;
    bitboards_t copy = *BITBOARDS;

    for (int i = 0; i < numMoves; i++) {
        if (mCastle(possible[i]) && (isIllegalCastle(possible[i], BITBOARDS) || isInCheck(BITBOARDS))) {
            continue;
        }
        struct undo_t undo;
        doMove(possible[i], BITBOARDS, &undo);
        if (canCaptureOpponentsKing(BITBOARDS)) {
            undoMove(possible[i], BITBOARDS, &undo);
            continue;
        }

        int nodesbefore = nodes;
        nodes += perft(depth-1, BITBOARDS, originalDepth);
        undoMove(possible[i], BITBOARDS, &undo);
        if (!equalBoards(&copy, BITBOARDS)) {

            printBoard(&copy);
            printBoard(BITBOARDS);

            printf("move %s %s\n", notation[mFrom(possible[i])], notation[mTo(possible[i])]);
            printf("undo: ");
            printf("%d %d %d %d\n", undo.capturedPiece, undo.enPassantSquare, undo.castleQueenSide, undo.castleKingSide);
            exit(0);
        }

        if (depth == originalDepth && nodes != nodesbefore) {
            printf("%s%s:", notation[mFrom(possible[i])], notation[mTo(possible[i])]);
            printf(" %d\n", nodes-nodesbefore);
        }
        
    }

    return nodes;
}