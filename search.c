#include "search.h"

u64 whiteSquares = 0xAA55AA55AA55AA55;
u64 blackSquares = 0x55AA55AA55AA55AA;

u64 aboutToPromoteWhite = 0x00FF000000000000;
u64 aboutToPromoteBlack = 0x000000000000FF00;

u64 AFILE = 0x8080808080808080;
u64 BFILE = 0x4040404040404040;
u64 CFILE = 0x2020202020202020;
u64 DFILE = 0x1010101010101010;
u64 EFILE = 0x0808080808080808;
u64 FFILE = 0x0404040404040404;
u64 GFILE = 0x0202020202020202;
u64 HFILE = 0x0101010101010101;

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


bool drawEvalByMaterial(bitboards_t *BITBOARDS) {

    if (bitCount(BITBOARDS->bits[allPieces]) == 2) {
        // only kings
        return true;

    } else if ((BITBOARDS->bits[whitePawns] | BITBOARDS->bits[blackPawns]) == 0) {
        // no pawns
        if ((BITBOARDS->bits[whiteRooks] | BITBOARDS->bits[blackRooks] | BITBOARDS->bits[whiteQueens] | BITBOARDS->bits[blackQueens]) == 0) {
            // no rooks or queens
            if ((BITBOARDS->bits[whiteBishops] | BITBOARDS->bits[blackBishops]) == 0) {
                // no bishops
                if (bitCount(BITBOARDS->bits[whiteKnights]) < 3 && bitCount(BITBOARDS->bits[blackKnights]) < 3) {
                    // less than 3 knights can not mate
                    return true;
                }

            } else if ((BITBOARDS->bits[whiteKnights] | BITBOARDS->bits[blackKnights]) == 0) {
                // there are bishops but no knights
                // only mate would be with bishop pair vs no bishops
                if (abs(bitCount(BITBOARDS->bits[whiteBishops]) - bitCount(BITBOARDS->bits[blackBishops])) < 2) {
                    return true;
                }

            } else if ((bitCount(BITBOARDS->bits[whiteKnights]) < 3 && BITBOARDS->bits[whiteBishops] == 0) || 
                       (bitCount(BITBOARDS->bits[whiteBishops]) == 1 && BITBOARDS->bits[whiteKnights] == 0)) {
                
                if ((bitCount(BITBOARDS->bits[blackKnights]) < 3 && BITBOARDS->bits[blackBishops] == 0) || 
                    (bitCount(BITBOARDS->bits[blackBishops]) == 1 && BITBOARDS->bits[blackKnights] == 0)) {
                    // both players either have not enough knights or only one bishop
                    return true;
                }
            }

        } else if ((BITBOARDS->bits[whiteQueens] | BITBOARDS->bits[blackQueens]) == 0) {
            // there are rooks but no queens
            if (bitCount(BITBOARDS->bits[whiteRooks]) == 1 && bitCount(BITBOARDS->bits[blackRooks]) == 1) {
                // both have exactly one rook
                if ((bitCount(BITBOARDS->bits[whiteKnights]) + bitCount(BITBOARDS->bits[whiteBishops])) < 2 && 
                    (bitCount(BITBOARDS->bits[blackKnights]) + bitCount(BITBOARDS->bits[blackBishops])) < 2) {
                    // but not enough minor pieces to mate
                    return true;
                }

            } else if (bitCount(BITBOARDS->bits[whiteRooks]) == 1 && BITBOARDS->bits[blackRooks] == 0) {
                // white has exactly one rook   
                 if ((BITBOARDS->bits[whiteKnights] | BITBOARDS->bits[whiteBishops] == 0) && 
                    // but no minor pieces
                    // black has 1 or 2 minor pieces
                     (((bitCount(BITBOARDS->bits[blackKnights]) + bitCount(BITBOARDS->bits[blackBishops])) == 1) ||
                      ((bitCount(BITBOARDS->bits[blackKnights]) + bitCount(BITBOARDS->bits[blackBishops])) == 2))) {
                        return true;
                    }
            } else if (bitCount(BITBOARDS->bits[blackRooks]) == 1 && BITBOARDS->bits[whiteRooks] == 0) {
                // black has exactly one rook
                if ((BITBOARDS->bits[blackKnights] | BITBOARDS->bits[blackBishops] == 0) &&
                    // but no minor pieces
                    // white has 1 or 2 minor pieces
                    (((bitCount(BITBOARDS->bits[whiteKnights]) + bitCount(BITBOARDS->bits[whiteBishops])) == 1) ||
                     ((bitCount(BITBOARDS->bits[whiteKnights]) + bitCount(BITBOARDS->bits[whiteBishops])) == 2))) {
                    return true;
                }
            }
        }
    }
    return false;
}

int evaluate(bitboards_t *BITBOARDS) {

    evalCalls++;

    if (drawEvalByMaterial(BITBOARDS)) {
        return 0;
    }

    int evalWhite = 0;
    int evalBlack = 0;

    int materialWhite = 0;
    int materialBlack = 0;

    int activityWhiteOpening = 0;
    int activityBlackOpening = 0;
    int activityWhiteEndgame = 0;
    int activityBlackEndgame = 0;

    u64 bPawns, bBishops, bRooks, wPawns, wBishops, wRooks;

    // pawn structure

    int doubledPawnsWhite = 0;
    int doubledPawnsBlack = 0;
    int isolatedPawnsWhite = 0;
    int isolatedPawnsBlack = 0;
    int blockedPawnsWhite = 0;
    int blockedPawnsBlack = 0;
    int passedPawnsWhite = 0;
    int passedPawnsBlack = 0;

    // rook on open file
    // means there are no pawns on the file

    int rooksOnOpenFileWhite = 0;
    int rooksOnOpenFileBlack = 0;

    // semi open means there is only one color of pawns on the file

    int rooksOnSemiOpenFileWhite = 0;
    int rooksOnSemiOpenFileBlack = 0;

    bPawns = BITBOARDS->bits[blackPawns];
    bBishops = BITBOARDS->bits[blackBishops];
    bRooks = BITBOARDS->bits[blackRooks];

    wPawns = BITBOARDS->bits[whitePawns];
    wBishops = BITBOARDS->bits[whiteBishops];
    wRooks = BITBOARDS->bits[whiteRooks];

    // material

    evalWhite += BITBOARDS->whiteMaterial;
    evalBlack += BITBOARDS->blackMaterial;

    // todo: add some tactics for knights and queens too

    while (wPawns) {
        u8 b = lsb(wPawns);
        wPawns &= wPawns - 1;

        doubledPawnsWhite += bitCount((bit(b) ^ file[b % 8]) & BITBOARDS->bits[whitePawns]);
        isolatedPawnsWhite += (adjFiles[b % 8] & BITBOARDS->bits[whitePawns]) == 0;
        blockedPawnsWhite += ((file[b % 8] << ((b / 8 + 1) * 8)) & BITBOARDS->bits[blackPieces]) != 0;
        passedPawnsWhite += (((file[b % 8] | adjFiles[b % 8]) << ((b / 8 + 1) * 8)) & BITBOARDS->bits[blackPawns]) == 0;
    }

    while (bPawns) {
        u8 b = lsb(bPawns);
        bPawns &= bPawns - 1;

        doubledPawnsBlack += bitCount((bit(b) ^ file[b % 8]) & BITBOARDS->bits[blackPawns]);
        isolatedPawnsBlack += (adjFiles[b % 8] & BITBOARDS->bits[blackPawns]) == 0;
        blockedPawnsBlack += ((file[b % 8] >> ((b / 8 - 1) * 8)) & BITBOARDS->bits[whitePieces]) != 0;
        passedPawnsBlack += (((file[b % 8] | adjFiles[b % 8]) >> ((b / 8 - 1) * 8)) & BITBOARDS->bits[whitePawns]) == 0;
    }

    while (wBishops) {
        u8 b = lsb(wBishops);
        wBishops &= wBishops - 1;
        // bishop being blocked by own pawns is bad
        evalWhite -= ((bit(b) & whiteSquares) > 0) * bitCount(whiteSquares & BITBOARDS->bits[whitePawns]);
        evalWhite -= ((bit(b) & blackSquares) > 0) * bitCount(blackSquares & BITBOARDS->bits[whitePawns]);
    }

    while (bBishops) {
        u8 b = lsb(bBishops);
        bBishops &= bBishops - 1;

        evalBlack += ((bit(b) & whiteSquares) > 0) * bitCount(whiteSquares & BITBOARDS->bits[blackPawns]);
        evalBlack += ((bit(b) & blackSquares) > 0) * bitCount(blackSquares & BITBOARDS->bits[blackPawns]);
    }

    while (wRooks) {
        u8 b = lsb(wRooks);
        wRooks &= wRooks - 1;

        rooksOnOpenFileWhite += (file[b % 8] & (BITBOARDS->bits[whitePawns] | BITBOARDS->bits[blackPawns])) == 0;
        rooksOnSemiOpenFileWhite += (file[b % 8] & (BITBOARDS->bits[whitePawns] | BITBOARDS->bits[blackPawns])) == 1;
    }

    while (bRooks) {
        u8 b = lsb(bRooks);
        bRooks &= bRooks - 1;

        rooksOnOpenFileBlack += (file[b % 8] & (BITBOARDS->bits[whitePawns] | BITBOARDS->bits[blackPawns])) == 0;
        rooksOnSemiOpenFileBlack += (file[b % 8] & (BITBOARDS->bits[whitePawns] | BITBOARDS->bits[blackPawns])) == 1;
    }

    // mobility and king safety
    // todo: make this less time consuming

    // int mobilityWhite = 0;
    // int mobilityBlack = 0;

    // int kingSafetyWhite = 0;
    // int kingSafetyBlack = 0;

    // int color = BITBOARDS->color;

    // BITBOARDS->color = WHITE;
    // getMobility(BITBOARDS, &mobilityWhite, &kingSafetyWhite, &kingSafetyBlack);

    // BITBOARDS->color = BLACK;
    // getMobility(BITBOARDS, &mobilityBlack, &kingSafetyBlack, &kingSafetyWhite);

    // BITBOARDS->color = color;

    // evalWhite += mobilityWhite;
    // evalBlack -= mobilityBlack;

    // evalWhite += 2 * kingSafetyWhite;
    // evalBlack -= 2 * kingSafetyBlack;

    // bishop pair

    evalWhite += (bitCount(BITBOARDS->bits[whiteBishops]) >= 2) * 15;
    evalBlack -= (bitCount(BITBOARDS->bits[blackBishops]) >= 2) * 15;

    evalWhite += rooksOnOpenFileWhite * 10;
    evalWhite += rooksOnSemiOpenFileWhite * 5;
    evalBlack -= rooksOnOpenFileBlack * 10;
    evalBlack -= rooksOnSemiOpenFileBlack * 5;

    // game phase dependent evaluation

    float endgameWeight = (
        
        ((7680 / (float)(BITBOARDS->whiteMaterial - BITBOARDS->blackMaterial)) - 1.08) * 0.5 + ((float)(fullMoveCount + ply) / 100) // maybe more than 100
        
    );

    endgameWeight = (endgameWeight > 0) * endgameWeight;
    if (endgameWeight > 1) {
        endgameWeight = 1;
    }

    evalWhite -= 10 * (doubledPawnsWhite + isolatedPawnsWhite + blockedPawnsWhite);
    evalWhite += (int)(100 * passedPawnsWhite * endgameWeight);
    evalBlack += 10 * (doubledPawnsBlack + isolatedPawnsBlack + blockedPawnsBlack);
    evalBlack -= (int)(100 * passedPawnsBlack * endgameWeight);

    // activity

    evalWhite += (int)(BITBOARDS->whitePositionOpening * (1 - endgameWeight) + BITBOARDS->whitePositionEndgame * endgameWeight);
    evalBlack += (int)(BITBOARDS->blackPositionOpening * (1 - endgameWeight) + BITBOARDS->blackPositionEndgame * endgameWeight);

    if (endgameWeight == 1) {
        int distBetweenKings = abs(lsb(BITBOARDS->bits[blackKing]) / 8 - lsb(BITBOARDS->bits[whiteKing]) / 8) + abs(lsb(BITBOARDS->bits[blackKing]) % 8 - lsb(BITBOARDS->bits[whiteKing]) % 8);
        // important for endgames
        if (evalWhite > -evalBlack) {

            evalWhite -= (distBetweenKings < 4) * 5 * distBetweenKings;
            evalWhite -= (distBetweenKings >= 4) * 10 * distBetweenKings;

            evalWhite += kingEvalWhiteEndgamePosition[lsb(BITBOARDS->bits[whiteKing])] / 2;

        } else if (-evalBlack > evalWhite) {
            
            evalBlack += (distBetweenKings < 4) * 5 * distBetweenKings;
            evalBlack += (distBetweenKings >= 4) * 10 * distBetweenKings;

            evalBlack -= kingEvalWhiteEndgamePosition[lsb(BITBOARDS->bits[blackKing]) ^ 56] / 2;
        }
    }

    return ((int)(((evalWhite + evalBlack) * 17680) / (evalWhite - evalBlack + 10000))) * (BITBOARDS->color);
}

bool equalBoards(struct bitboards_t *b1, struct bitboards_t *b2) {
    // was used for debugging the boards as pointers

    for (int i = 0; i < 64; i++) {
        if (b1->pieceList[i] != b2->pieceList[i]) {
            printf("piece list mismatch\n");
            printf("copy: [");
            for (int j = 63; j >= 0; j--) {
                printf("%d ", b1->pieceList[j]);
            }
            printf("]\n");
            printf("orig: [");
            for (int j = 63; j >= 0; j--) {
                printf("%d ", b2->pieceList[j]);
            }
            printf("]\n");
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
        b1->bits[whitePawns] == b2->bits[whitePawns] &&
        b1->bits[whiteKnights] == b2->bits[whiteKnights] &&
        b1->bits[whiteBishops] == b2->bits[whiteBishops] &&
        b1->bits[whiteRooks] == b2->bits[whiteRooks] &&
        b1->bits[whiteQueens] == b2->bits[whiteQueens] &&
        b1->bits[whiteKing] == b2->bits[whiteKing] &&
        b1->bits[blackPawns] == b2->bits[blackPawns] &&
        b1->bits[blackKnights] == b2->bits[blackKnights] &&
        b1->bits[blackBishops] == b2->bits[blackBishops] &&
        b1->bits[blackRooks] == b2->bits[blackRooks] &&
        b1->bits[blackQueens] == b2->bits[blackQueens] &&
        b1->bits[blackKing] == b2->bits[blackKing] &&
        b1->bits[allPieces] == b2->bits[allPieces] &&
        b1->bits[whitePieces] == b2->bits[whitePieces] &&
        b1->bits[blackPieces] == b2->bits[blackPieces] &&
        b1->hash == b2->hash &&
        b1->whiteMaterial == b2->whiteMaterial &&
        b1->blackMaterial == b2->blackMaterial &&
        b1->whitePositionOpening == b2->whitePositionOpening &&
        b1->whitePositionEndgame == b2->whitePositionEndgame &&
        b1->blackPositionOpening == b2->blackPositionOpening &&
        b1->blackPositionEndgame == b2->blackPositionEndgame

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
        if (b1->bits[whitePawns] != b2->bits[whitePawns]) {
            printf("bits[whitePawns]\n");
        }
        if (b1->bits[whiteKnights] != b2->bits[whiteKnights]) {
            printf("bits[whiteKnights]\n");
        }
        if (b1->bits[whiteBishops] != b2->bits[whiteBishops]) {
            printf("bits[whiteBishops]\n");
        }
        if (b1->bits[whiteRooks] != b2->bits[whiteRooks]) {
            printf("bits[whiteRooks]\n");
        }
        if (b1->bits[whiteQueens] != b2->bits[whiteQueens]) {
            printf("bits[whiteQueens]\n");
        }
        if (b1->bits[whiteKing] != b2->bits[whiteKing]) {
            printf("bits[whiteKing]\n");
        }
        if (b1->bits[blackPawns] != b2->bits[blackPawns]) {
            printf("bits[blackPawns]\n");
        }
        if (b1->bits[blackKnights] != b2->bits[blackKnights]) {
            printf("bits[blackKnights]\n");
        }
        if (b1->bits[blackBishops] != b2->bits[blackBishops]) {
            printf("bits[blackBishops]\n");
        }
        if (b1->bits[blackRooks] != b2->bits[blackRooks]) {
            printf("bits[blackRooks]\n");
        }
        if (b1->bits[blackQueens] != b2->bits[blackQueens]) {
            printf("bits[blackQueens]\n");
        }
        if (b1->bits[blackKing] != b2->bits[blackKing]) {
            printf("bits[blackKing]\n");
        }
        if (b1->bits[allPieces] != b2->bits[allPieces]) {
            printf("bits[allPieces]\n");
        }
        if (b1->bits[whitePieces] != b2->bits[whitePieces]) {
            printf("bits[whitePieces]\n");
        }
        if (b1->bits[blackPieces] != b2->bits[blackPieces]) {
            printf("bits[blackPieces]\n");
        }
        if (b1->hash != b2->hash) {
            printf("hash\n");
        }
        if (b1->whiteMaterial != b2->whiteMaterial) {
            printf("whiteMaterial\n");
        }
        if (b1->blackMaterial != b2->blackMaterial) {
            printf("blackMaterial\n");
        }
        if (b1->whitePositionOpening != b2->whitePositionOpening) {
            printf("whitePositionOpening\n");
            printf("%d %d\n", b1->whitePositionOpening, b2->whitePositionOpening);
        }
        if (b1->whitePositionEndgame != b2->whitePositionEndgame) {
            printf("whitePositionEndgame\n");
        }
        if (b1->blackPositionOpening != b2->blackPositionOpening) {
            printf("blackPositionOpening\n");
        }
        if (b1->blackPositionEndgame != b2->blackPositionEndgame) {
            printf("blackPositionEndgame\n");
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

// why have two different sorting algorithms?
// because a stable one (merge sort) is used for the 1st depth to sort the moves in order to get the best move and keep
// the principal variation, and a fast one that doesnt need to be stable for move ordering

void mergeSortArrayDec(move_t moves[], int values[], int left, int right) {
    // Sorts the moves decreasingly in order of their evaluation
    if (left < right) {
        int middle = left + (right - left) / 2;

        mergeSortArrayDec(moves, values, left, middle);
        mergeSortArrayDec(moves, values, middle + 1, right);

        int leftSize = middle - left + 1;
        int rightSize = right - middle;

        int leftValues[leftSize], rightValues[rightSize];
        move_t leftMoves[leftSize], rightMoves[rightSize];

        for (int i = 0; i < leftSize; i++) {
            leftValues[i] = values[left + i];
            leftMoves[i] = moves[left + i];
        }
        for (int i = 0; i < rightSize; i++) {
            rightValues[i] = values[middle + 1 + i];
            rightMoves[i] = moves[middle + 1 + i];
        }

        int i = 0, j = 0, k = left;
        while (i < leftSize && j < rightSize) {
            if (leftValues[i] >= rightValues[j]) {
                values[k] = leftValues[i];
                moves[k] = leftMoves[i];
                i++;
            } else {
                values[k] = rightValues[j];
                moves[k] = rightMoves[j];
                j++;
            }
            k++;
        }

        while (i < leftSize) {
            values[k] = leftValues[i];
            moves[k] = leftMoves[i];
            i++;
            k++;
        }

        while (j < rightSize) {
            values[k] = rightValues[j];
            moves[k] = rightMoves[j];
            j++;
            k++;
        }
    }
}

void mergeSortArrayInc(move_t moves[], int values[], int left, int right) {
    // Sorts the moves increasingly in order of their evaluation
    if (left < right) {
        int middle = left + (right - left) / 2;

        mergeSortArrayInc(moves, values, left, middle);
        mergeSortArrayInc(moves, values, middle + 1, right);

        int leftSize = middle - left + 1;
        int rightSize = right - middle;

        int leftValues[leftSize], rightValues[rightSize];
        move_t leftMoves[leftSize], rightMoves[rightSize];

        for (int i = 0; i < leftSize; i++) {
            leftValues[i] = values[left + i];
            leftMoves[i] = moves[left + i];
        }
        for (int i = 0; i < rightSize; i++) {
            rightValues[i] = values[middle + 1 + i];
            rightMoves[i] = moves[middle + 1 + i];
        }

        int i = 0, j = 0, k = left;
        while (i < leftSize && j < rightSize) {
            if (leftValues[i] <= rightValues[j]) {
                values[k] = leftValues[i];
                moves[k] = leftMoves[i];
                i++;
            } else {
                values[k] = rightValues[j];
                moves[k] = rightMoves[j];
                j++;
            }
            k++;
        }

        while (i < leftSize) {
            values[k] = leftValues[i];
            moves[k] = leftMoves[i];
            i++;
            k++;
        }

        while (j < rightSize) {
            values[k] = rightValues[j];
            moves[k] = rightMoves[j];
            j++;
            k++;
        }
    }
}

void orderMoves(move_t *moves, bitboards_t *BITBOARDS, int numMoves) {
    int values[numMoves];
    move_t tableMove = tableGetMove(transTable, BITBOARDS->hash);
    float endgameWeight = (
        
        ((7680 / (float)(BITBOARDS->whiteMaterial - BITBOARDS->blackMaterial)) - 1.08) * 0.5 + ((float)(fullMoveCount + ply) / 100)
        
    );

    endgameWeight = (endgameWeight > 0) * endgameWeight;
    if (endgameWeight > 1) {
        endgameWeight = 1;
    }
        
    if (BITBOARDS->color == WHITE) {
        for (int i = 0; i < numMoves; i++) {
            values[i] = 0;
            if (tableMove == moves[i]) {
                values[i] = INF;
                continue;
            }
            u8 t = mTo(moves[i]);
            u8 f = mFrom(moves[i]);
            // capture
            if (BITBOARDS->pieceList[t] || t == BITBOARDS->enPassantSquare) {
                if (t == BITBOARDS->enPassantSquare) {
                    values[i] += (int)(whitePositionTables[OPENING][0][(t - 8) ^ 56] * (1 - endgameWeight) + whitePositionTables[ENDGAME][0][(t - 8) ^ 56] * endgameWeight + PAWNVALUE);
                } else {
                    values[i] += (int)(whitePositionTables[OPENING][BITBOARDS->pieceList[t] * -1 - 1][t ^ 56] * (1 - endgameWeight) +
                                    whitePositionTables[ENDGAME][BITBOARDS->pieceList[t] * -1 - 1][t ^ 56] * endgameWeight + materialValues[BITBOARDS->pieceList[t] * -1 - 1]);
                }
                values[i] -= materialValues[BITBOARDS->pieceList[f] - 1];
            }
                values[i] += ((whitePositionTables[ENDGAME][BITBOARDS->pieceList[f] - 1][t] - 
                             whitePositionTables[ENDGAME][BITBOARDS->pieceList[f] - 1][f]) * (1 - endgameWeight) +
                             (whitePositionTables[OPENING][BITBOARDS->pieceList[f] - 1][t] -
                             whitePositionTables[OPENING][BITBOARDS->pieceList[f] - 1][f]) * endgameWeight);
        }

        quickSortArrayDec(moves, values, 0, numMoves-1);

    } else {
        for (int i = 0; i < numMoves; i++) {
            values[i] = 0;
            if (tableMove == moves[i]) {
                values[i] = -INF;
                continue;
            }
            u8 t = mTo(moves[i]);
            u8 f = mFrom(moves[i]);
            
            if (BITBOARDS->pieceList[t] || t == BITBOARDS->enPassantSquare) {
                if (t == BITBOARDS->enPassantSquare) {
                    values[i] -= (int)(whitePositionTables[OPENING][0][t + 8] * (1 - endgameWeight) + 
                                    whitePositionTables[ENDGAME][0][t + 8] * endgameWeight + PAWNVALUE);
                } else {
                    values[i] -= (int)(whitePositionTables[OPENING][BITBOARDS->pieceList[t] - 1][t] * (1 - endgameWeight) +
                                    whitePositionTables[ENDGAME][BITBOARDS->pieceList[t] - 1][t] * endgameWeight + materialValues[BITBOARDS->pieceList[t] - 1]);
                }
                values[i] += materialValues[BITBOARDS->pieceList[f] * -1 - 1];
            }
                values[i] -= ((whitePositionTables[ENDGAME][BITBOARDS->pieceList[f] * -1 - 1][t ^ 56] - 
                             whitePositionTables[ENDGAME][BITBOARDS->pieceList[f] * -1 - 1][f ^ 56]) * (1 - endgameWeight) +
                             (whitePositionTables[OPENING][BITBOARDS->pieceList[f] * -1 - 1][t ^ 56] -
                             whitePositionTables[OPENING][BITBOARDS->pieceList[f] * -1 - 1][f ^ 56]) * endgameWeight);
            
        }

        quickSortArrayInc(moves, values, 0, numMoves-1);
    }
}

void orderCaptures(move_t *moves, bitboards_t *BITBOARDS, int numMoves) {
    int values[numMoves];
    move_t tableMove = tableGetMove(quietTable, BITBOARDS->hash);
        
    if (BITBOARDS->color == WHITE) {
        for (int i = 0; i < numMoves; i++) {
            values[i] = 0;

            if (tableMove == moves[i]) {
                values[i] = INF;
                continue;
            }

            values[i] += (mTo(moves[i]) == BITBOARDS->enPassantSquare) * PAWNVALUE;
            values[i] += !(mTo(moves[i]) == BITBOARDS->enPassantSquare) * materialValues[BITBOARDS->pieceList[mTo(moves[i])] * -1 - 1];

            values[i] -= materialValues[BITBOARDS->pieceList[mFrom(moves[i])] - 1];
        }

        quickSortArrayDec(moves, values, 0, numMoves-1);

    } else {
        for (int i = 0; i < numMoves; i++) {
            values[i] = 0;

            if (tableMove == moves[i]) {
                values[i] = INF;
                continue;
            }

            values[i] -= (mTo(moves[i]) == BITBOARDS->enPassantSquare) * PAWNVALUE;
            values[i] -= !(mTo(moves[i]) == BITBOARDS->enPassantSquare) * materialValues[BITBOARDS->pieceList[mTo(moves[i])] - 1];

            values[i] += materialValues[BITBOARDS->pieceList[mFrom(moves[i])] * -1 - 1];
        }

        quickSortArrayInc(moves, values, 0, numMoves-1);
    }
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
    if (bitCount(BITBOARDS->bits[allPieces]) == 2) {
        // only kings left
        return true;

    } else if ((BITBOARDS->bits[whitePawns] | BITBOARDS->bits[blackPawns]) == 0) {
        // no pawns that could promote
        if ((BITBOARDS->bits[whiteRooks] | BITBOARDS->bits[blackRooks] | BITBOARDS->bits[whiteQueens] | BITBOARDS->bits[blackQueens]) == 0) {
            // no queens or rooks
            if (bitCount(BITBOARDS->bits[allPieces]) == 3) {
                // only kings and one minor piece left
                return true;
            } else if (bitCount(BITBOARDS->bits[allPieces]) == 4) {
                // both players have one minor piece left and if those are bishops they are on the same color
                // note that in theory it is possible to mate with two knights and also with bishop vs bishop on different colors
                if (bitCount(BITBOARDS->bits[whiteBishops]) == 1 && bitCount(BITBOARDS->bits[blackBishops]) == 1 && sameSquareColor(BITBOARDS->bits[whiteBishops], BITBOARDS->bits[blackBishops])) {
                    return true;
                }
            }
        }
    }

    return false;
}

int pvLen[64];

int pvTable[64][64];

void resetSearch() {
    ply = 0;
    evalCalls = 0;
    moveCalls = 0;
    possibleCalls = 0;
    quiescenceCalls = 0;
    nodes = 0;
    quietNodes = 0;
    transpositions = 0;
    quietTranspositions = 0;
    memset(searchHistory, 0, sizeof(searchHistory));
    memset(pvLen, 0, sizeof(pvLen));
    memset(pvTable, 0, sizeof(pvTable));
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

    move_t moves[MAX_NUM_MOVES];
    int moveCount = 0;
    
    u64 checkers;
    isInCheck(BITBOARDS, &checkers);
    u64 pinned = 0;
    u8 pinners[64] = {0};
    getPins(BITBOARDS, &pinned, &pinners[0]);
    u64 attacks = getEnemyAttackMask(BITBOARDS);
    moveCount = getCaptures(BITBOARDS, &moves[0], checkers, pinned, &pinners[0], attacks);

    // start with the normal evaluation since the players can not be forced to capture
    evalCalls++;

    value = evaluate(BITBOARDS);

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
        int delta = 890 + (mPromo(move)) * 800;

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

    for (int i = 0; i < ply; i++) {
        if (searchHistory[i] == BITBOARDS->hash) {
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

int negaMax(struct bitboards_t *BITBOARDS, int depth, int alpha, int beta, bool allowNullMove) { // , int checkExtensions, int promoExtensions
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

    pvLen[ply] = ply;

    nodes++;
    visits++;

    int value;
    bool futilityPruning = false;

    if (isRepetition(BITBOARDS)) {
        // as repetition mostly leads to repetition we can assume that the position is a draw
        return 0;
    }

    if (tableGetEntry(transTable, BITBOARDS->hash, depth, &value, alpha, beta)) {

        transpositions++;
        return value - (ply * BITBOARDS->color) * bitboards->endgameFlag;
    }

    if (nodes & 0b11111111111) {
        // every 2048 nodes we check if the time is up
        if (((double)(clock() - searchStartTime)) / (CLOCKS_PER_SEC / 1000) > maxTime*1000) {
            return 0;
        }
    }

    if (depth <= 0) {
        // when the depth limit is reached we use quiescence search to get a more accurate evaluation
        quiescenceCalls++;
        return quiescenceSearch(BITBOARDS, alpha, beta, 0);
    }

    struct undo_t undo;

    u64 checkers;
    bool inCheck = isInCheck(BITBOARDS, &checkers);

    move_t moves[MAX_NUM_MOVES];
    int moveCount;

    u64 pinned = 0;
    u8 pinners[64] = {0};
    getPins(BITBOARDS, &pinned, &pinners[0]);
    u64 attacks = getEnemyAttackMask(BITBOARDS);
    moveCount = getMoves(BITBOARDS, &moves[0], checkers, pinned, &pinners[0], attacks);

    // if ((aboutToPromoteWhite & BITBOARDS->bits[whitePawns]) || (aboutToPromoteBlack & BITBOARDS->bits[blackPawns])) {
    //     if (promoExtensions < 4) { // todo: test some numbers for this
    //         depth++;
    //         promoExtensions++;
    //     }
    // }

    if (inCheck) {
        // extend the search if the king is in check as when in check there are less moves that can be searched
        // and we dont want to miss a forced mate easily
        // if (checkExtensions < 8) {
        //     depth++;
        //     checkExtensions++;
        // }

    } else {
        int eval = evaluate(BITBOARDS);

        // evalution pruning
        if (depth < 3 && abs(beta - 1) > -INF + 100) {
            int evalMargin = 100 * depth;
            if (eval - evalMargin >= beta) {
                return eval - evalMargin;
            }
        }

        // null move pruning
        if (depth >= 3 && allowNullMove) {
        
            BITBOARDS->color = -BITBOARDS->color;
            u64 ep = BITBOARDS->enPassantSquare;
            if (BITBOARDS->enPassantSquare >= 0) {
                BITBOARDS->hash ^= ZOBRIST_TABLE[BITBOARDS->enPassantSquare][12];
            }
            BITBOARDS->enPassantSquare = -1;
            BITBOARDS->hash ^= whiteToMove;

            value = -negaMax(BITBOARDS, depth - 1 - 2, -beta, -beta + 1, false); // , checkExtensions, promoExtensions
            
            BITBOARDS->hash ^= whiteToMove;
            BITBOARDS->color = -BITBOARDS->color;
            BITBOARDS->enPassantSquare = ep;
            if (BITBOARDS->enPassantSquare >= 0) {
                BITBOARDS->hash ^= ZOBRIST_TABLE[BITBOARDS->enPassantSquare][12];
            }

            if (value >= beta) {
                return beta;
            }
        }

        // // razoring
        // value = eval + 125;
        // int newValue;
        
        // if (value < beta && depth == 1) {
        //     newValue = quiescenceSearch(BITBOARDS, alpha, beta, 0);
        //     quiescenceCalls++;
        //     return (newValue > value) ? newValue : value;
        // }
        
        // value += 175;

        // if (value < beta && depth < 4) {
        //     newValue = quiescenceSearch(BITBOARDS, alpha, beta, 0);
        //     quiescenceCalls++;
        //     if (newValue < beta) {return (newValue > value) ? newValue : value;}
        // }

        // futility pruning condition
        if (depth < 4 && abs(alpha) < INF && eval + futilityMargin[depth] <= alpha) {
            futilityPruning = true;
        }
    }

    orderMoves(&moves[0], BITBOARDS, moveCount);

    int flag = UPPERBOUND;
    int bestMove = 0;

    for (int i = 0; i < moveCount; i++) {

        move_t move = moves[i];

        doMove(move, BITBOARDS, &undo);

        // futility pruning
        if (futilityPruning && i > 0 && undo.capturedPiece == 0 && mPromo(move) == 0 && !isInCheckLight(BITBOARDS)) {
            undoMove(move, BITBOARDS, &undo);
            continue;
        }

        if (i == 0) {
            value = -negaMax(BITBOARDS, depth - 1, -beta, -alpha, true); // , checkExtensions, promoExtensions
        } else {
            // late move reductions
            if (i > 4 && depth > 2 && !inCheck && undo.capturedPiece == 0 && mPromo(move) == 0) {
                value = -negaMax(BITBOARDS, depth - 2, -alpha - 1, -alpha, true); // , checkExtensions, promoExtensions
            } else {
                value = alpha + 1;
            }

            if (value > alpha) {
                value = -negaMax(BITBOARDS, depth - 1, -alpha - 1, -alpha, true); // , checkExtensions, promoExtensions
                
                if (value > alpha && value < beta) {
                    value = -negaMax(BITBOARDS, depth - 1, -beta, -alpha, true); // , checkExtensions, promoExtensions
                }
            }
        }

        undoMove(move, BITBOARDS, &undo);

        if (value >= beta) {
            tableSetEntry(transTable, BITBOARDS->hash, depth, value, LOWERBOUND);
            
            tableSetMove(transTable, BITBOARDS->hash, depth, move);
            return beta;
        }

        if (value > alpha) {
            alpha = value;
            flag = EXACT;
            bestMove = move;
            // tableSetMove(transTable, BITBOARDS->hash, depth, move);

            pvTable[ply][ply] = move;
            for (int j = ply + 1; j < pvLen[ply + 1]; j++) {
                pvTable[ply][j] = pvTable[ply + 1][j];
            }
            pvLen[ply] = pvLen[ply + 1];
        }
    }

    if (moveCount == 0) {
        if (isInCheckLight(BITBOARDS)) {
            // mate
            return -INF + ply;
        } else {
            // stalemate
            return 0;
        }
    }

    tableSetEntry(transTable, BITBOARDS->hash, depth, alpha, flag);

    tableSetMove(transTable, BITBOARDS->hash, depth, bestMove);

    return alpha;
}

move_t iterDeepening(move_t *possible, struct bitboards_t *bitboards, int numMoves, bool inCheck) {
    // find the best move using iterative deepening
    // this also includes the principal variation search which could also be used in the negamax function
    move_t best;
    int values[numMoves];

    // ordering moves here is not necessary because at depth 0 the moves will be ordered by the orderMoves function
    searchStartTime = clock();
    
    // bool stopSearch = false;
    int s = -bitboards->color;
    int bestScore = 0;

    printf("move amount: %d\n", numMoves);

    if (numMoves == 1) {
        printf("forced move: %s%s\n", notation[mFrom(possible[0])], notation[mTo(possible[0])]);
        return possible[0];
    }

    pvLen[ply] = ply;

    for (int depth = 0; depth < maxDepth; depth++) {
        // aspiration window
        // int low = 25;
        // int high = 25;

        // while (1) {
        //     // try searching with the window until it fits
        //     int alpha = bestScore - low;
        //     int beta = bestScore + high;

            printf("depth: %d - \n", depth+1);

            int i = 0;
            int bestEval = s * INF;

            for (; i < numMoves; i++) {

                int moveEval;

                // if (stopSearch) {
                //     moveEval = 0;
                // } else {
                    visits = 0;
                    quietVisits = 0;

                    struct undo_t undo;
                    doMove(possible[i], bitboards, &undo);

                    printf("%s%s: ", notation[mFrom(possible[i])], notation[mTo(possible[i])]);
                    int t = transpositions;

                    moveEval = s * negaMax(bitboards, depth, -INF, INF, true); // , 0, 0 // start negamax search to evaluate the pv move
                    undoMove(possible[i], bitboards, &undo);

                    printf("%-10d quiet: %-10d eval: %d transpositions: %d\n", visits, quietVisits, moveEval, transpositions - t);
                // }

                values[i] = moveEval;

                if ((moveEval > bestEval && bitboards->color == WHITE) || (moveEval < bestEval && bitboards->color == BLACK)) {
                    bestEval = moveEval;

                    pvTable[ply][ply] = possible[i];
                    for (int j = ply + 1; j < pvLen[ply + 1]; j++) {
                        pvTable[ply][j] = pvTable[ply + 1][j];
                    }
                    pvLen[ply] = pvLen[ply + 1];
                }

                if (((double)(clock() - searchStartTime)) / (CLOCKS_PER_SEC / 1000) > maxTime*1000) {
                    printf("time out\n");
                    break;

                } // else if ((bitboards->color && (moveEval >= 99000)) || (!bitboards->color && (moveEval <= -99000))) {
                //     printf("checkmate found\n");
                //     return possible[i];
                //     stopSearch = true;
                // }
            }

            if (bitboards->color == WHITE) {
                mergeSortArrayDec(&possible[0], values, 0, i-1);
            } else {
                mergeSortArrayInc(&possible[0], values, 0, i-1);
            }

            bestScore = values[0];
            tableSetMove(transTable, bitboards->hash, depth, possible[0]);

            if (((double)(clock() - searchStartTime)) / (CLOCKS_PER_SEC / 1000) > maxTime*1000) {
                break;
            }

            printf("\n>>> best move: %s%s value: %d\n", notation[mFrom(possible[0])], notation[mTo(possible[0])], values[0]);
            printf(">>> pv: ");

            for (int j = 0; j < pvLen[0]; j++) {
                printf("%s%s ", notation[mFrom(pvTable[0][j])], notation[mTo(pvTable[0][j])]);
            }

            printf("\n\n");

            // int x = 0;
            // int y = 0;
            // int z = 0;
            // while (pvTable[x][0] != 0) {
            //     while (pvTable[x][y] != 0) {
            //         printf("%s%s ", notation[mFrom(pvTable[x][y])], notation[mTo(pvTable[x][y])]);
            //         y++;
            //     }
            //     printf("\n");
            //     x++;
            // }

            // while (pvLen[z]) {
            //     printf("%d: %d\n", z, pvLen[z]);
            //     z++;
            // }

            memset(pvTable, 0, sizeof(pvTable));
            memset(pvLen, 0, sizeof(pvLen));

            // // aspiration window fail
            // if (bestScore == alpha) {
            //     low *= 4;
            //     printf("repeating search...\n");
            // }
            // else if (bestScore == beta) {
            //     printf("repeating search...\n");
            //     high *= 4;
            // }
            // else {
            //     // aspiration window success, break out of the loop
            //     break;
            // }
        //}
        if (((double)(clock() - searchStartTime)) / (CLOCKS_PER_SEC / 1000) > maxTime*1000) {
            break;
        }
    }

    printf("\ntranspositions found: %d (%d quiet)\n", transpositions, quietTranspositions);

    return possible[0];
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
    resetSearch();
    float endgameWeight = (
            
        ((7680 / (float)(bitboards->whiteMaterial - bitboards->blackMaterial)) - 1.08) * 0.5 + ((float)(fullMoveCount) / 100) // maybe more than 100
        
    );
    if (endgameWeight > 1) {
        bitboards->endgameFlag = 1;
    }
    // find and make the engine move
    clock_t startTime = clock();
    move_t best = 0;
    // probe the book
    move_t bookMove = getBookMove(bitboards->hash);
    int numMoves = 0;
    if (bookMove != 0) {
        best = bookMove;

    } else {
        move_t possible[MAX_NUM_MOVES];
        u64 checkers;
        bool inCheck = isInCheck(bitboards, &checkers);
        u64 pinned = 0;
        u8 pinners[64] = {0};
        getPins(bitboards, &pinned, &pinners[0]);
        u64 attacks = getEnemyAttackMask(bitboards);
        numMoves = getMoves(bitboards, &possible[0], checkers, pinned, &pinners[0], attacks);
        if (numMoves > 0) {
            best = iterDeepening(possible, bitboards, numMoves, inCheck);
        } 
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
        int s = bitboards->color;
        printf("quick evaluation: %d\n", s * quiescenceSearch(bitboards, -INF, INF, 0));
        printf("board after move\n");
        printBoard(bitboards);

        printf("fullMoveCount: %d\n", fullMoveCount);

        float endgameWeight = (
            
            ((7680 / (float)(bitboards->whiteMaterial - bitboards->blackMaterial)) - 1.08) * 0.5 + ((float)(fullMoveCount) / 100) // maybe more than 100
            
        );

        endgameWeight = (endgameWeight > 0) * endgameWeight;
        if (endgameWeight > 1) {
            endgameWeight = 1;
        }

        printf("game phase: %.2f\n", endgameWeight);

        printf("nodes searched: %d (%d quiet)\n", nodes, quietNodes);
    } else {
        bitboards->color = -bitboards->color;
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
    int numOtherMoves;

    u64 checkers;
    bool inCheck = isInCheck(bitboards, &checkers);
    u64 pinned = 0;
    u8 pinners[64] = {0};
    getPins(bitboards, &pinned, &pinners[0]);
    u64 attacks = getEnemyAttackMask(bitboards);
    numOtherMoves = getMoves(bitboards, &othermoves[0], checkers, pinned, &pinners[0], attacks);

    if (numOtherMoves == 0) {
        if (inCheck) {
            printf("player is mate\n");
        } else {
            printf("draw\n");
        }
        exit(0);
    }

    printf("--------------------function calls-------------------\n");
    printf("eval: %d moves: %d possible: %d quiet: %d\n", evalCalls, moveCalls, possibleCalls, quiescenceCalls);
}

u64 perft(int depth, struct bitboards_t *BITBOARDS, int originalDepth) {
    // perft function for debugging

    if (depth == 0) {
        return 1;
    }

    move_t possible[MAX_NUM_MOVES];
    int numMoves = 0;

    u64 checkers;
    bool inCheck = isInCheck(BITBOARDS, &checkers);
    u64 pinned = 0;
    u8 pinners[64] = {0};
    getPins(BITBOARDS, &pinned, &pinners[0]);
    u64 attacks = getEnemyAttackMask(BITBOARDS);
    numMoves = getMoves(BITBOARDS, &possible[0], checkers, pinned, &pinners[0], attacks);
    
    int nodes = 0;
    // bitboards_t copy = *BITBOARDS;

    for (int i = 0; i < numMoves; i++) {

        struct undo_t undo;
        doMove(possible[i], BITBOARDS, &undo);

        int nodesbefore = nodes;
        nodes += perft(depth-1, BITBOARDS, originalDepth);
        undoMove(possible[i], BITBOARDS, &undo);

        // if (!equalBoards(&copy, BITBOARDS)) {

        //     printBoard(&copy);
        //     printBoard(BITBOARDS);

        //     printf("move %s %s\n", notation[mFrom(possible[i])], notation[mTo(possible[i])]);
        //     printf("undo: ");
        //     printf("%d %d %d %d %d %d\n", undo.capturedPiece, undo.enPassantSquare, undo.whiteCastleQueenSide, undo.whiteCastleKingSide, undo.blackCastleQueenSide, undo.blackCastleKingSide);
        //     exit(0);
        // }

        if (depth == originalDepth && nodes != nodesbefore) {
            printf("%s%s:", notation[mFrom(possible[i])], notation[mTo(possible[i])]);
            printf(" %d\n", nodes-nodesbefore);
        }
        
    }

    return nodes;
}