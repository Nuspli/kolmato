#include "moveGeneration.h"

u64 notRightmostFileMask = 0xFEFEFEFEFEFEFEFE;
u64 notLeftmostFileMask = 0x7F7F7F7F7F7F7F7F;

u64 pawnPromotionMask = 0xFF000000000000FF;

u64 rank[8] = {
    0x00000000000000FF,
    0x000000000000FF00,
    0x0000000000FF0000,
    0x00000000FF000000,
    0x000000FF00000000,
    0x0000FF0000000000,
    0x00FF000000000000,
    0xFF00000000000000
};

u64 file[8] = {
    0x0101010101010101,
    0x0202020202020202,
    0x0404040404040404,
    0x0808080808080808,
    0x1010101010101010,
    0x2020202020202020,
    0x4040404040404040,
    0x8080808080808080
};

u64 adjFiles[8] = {
    0x0202020202020202,
    0x0505050505050505,
    0x0A0A0A0A0A0A0A0A,
    0x1414141414141414,
    0x2828282828282828,
    0x5050505050505050,
    0xA0A0A0A0A0A0A0A0,
    0x4040404040404040
};

u64 knightAttacks[64];
u64 kingAttacks[64];
u64 bishopAttacks[64][512];
u64 rookAttacks[64][4096];

u64 squaresBetween[64][64];

int BLTR[64] = {
    7, 8, 9,10,11,12,13,14,
    6, 7, 8, 9,10,11,12,13,
    5, 6, 7, 8, 9,10,11,12,
    4, 5, 6, 7, 8, 9,10,11,
    3, 4, 5, 6, 7, 8, 9,10,
    2, 3, 4, 5, 6, 7, 8, 9,
    1, 2, 3, 4, 5, 6, 7, 8,
    0, 1, 2, 3, 4, 5, 6, 7
};

int TLBR[64] = {
    14,13,12,11,10,9, 8, 7,
    13,12,11,10,9, 8, 7, 6,
    12,11,10,9, 8, 7, 6, 5,
    11,10,9, 8, 7, 6, 5, 4,
    10,9, 8, 7, 6, 5, 4, 3,
    9, 8, 7, 6, 5, 4, 3, 2,
    8, 7, 6, 5, 4, 3, 2, 1,
    7, 6, 5, 4, 3, 2, 1, 0
};

bool sameDiagonal(int a, int b) {
    return (BLTR[a] == BLTR[b] || TLBR[a] == TLBR[b]);
}

void initSquaresBetween() {
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 64; j++) {
            u64 mask = 0;
            if (i != j) {
                int a, b;
                if (i < j) {
                    a = i;
                    b = j;
                } else {
                    a = j;
                    b = i;
                }
                if (sameFile(a, b)) {
                    for (int k = a + 8; k < b; k += 8) {
                        setBit(mask, k);
                    }
                } else if (sameRank(a, b)) {
                    for (int k = a + 1; k < b; k++) {
                        setBit(mask, k);
                    }
                } else if (BLTR[a] == BLTR[b]) {
                    for (int k = a + 9; k < b; k += 9) {
                        setBit(mask, k);
                    }
                } else if (TLBR[a] == TLBR[b]) {
                    for (int k = a + 7; k < b; k += 7) {
                        setBit(mask, k);
                    }
                }
            }
            squaresBetween[i][j] = mask;
        }
    }

    // printf("\n");
    // displayBoard(squaresBetween[0][63]);
    // displayBoard(squaresBetween[31][26]);
    // displayBoard(squaresBetween[26][2]);
    // displayBoard(squaresBetween[60][33]);
    // displayBoard(squaresBetween[28][26]);
    // displayBoard(squaresBetween[28][27]);
    // displayBoard(squaresBetween[28][28]);
}

u64 getSquaresBetween(int a, int b) {
    if (sameFile(a, b) || sameRank(a, b) || BLTR[a] == BLTR[b] || TLBR[a] == TLBR[b]) {
        return squaresBetween[a][b];
    } else {
        printf("Error: getSquaresBetween called on non-aligned squares\n");
        exit(1);
    }
}

void initKnightAttacks() {
    for (int i = 0; i < 64; i++) {
        // knight attacks from each of the 64 squares
        u64 knightboard = 0;
        if ((i % 8) < 6 && (i / 8) != 7) {setBit(knightboard, i+10);} // conditions are to make sure the knight does not go off the board
        if ((i % 8) > 1 && (i / 8) != 0) {setBit(knightboard, i-10);}
        if ((i % 8) != 7 && (i / 8) < 6) {setBit(knightboard, i+17);}
        if ((i % 8) != 0 && (i / 8) > 1) {setBit(knightboard, i-17);}
        if ((i % 8) != 0 && (i / 8) < 6) {setBit(knightboard, i+15);}
        if ((i % 8) != 7 && (i / 8) > 1) {setBit(knightboard, i-15);}
        if ((i % 8) > 1 && (i / 8) != 7) {setBit(knightboard, i+6);}
        if ((i % 8) < 6 && (i / 8) != 0) {setBit(knightboard, i-6);}

        knightAttacks[i] = knightboard;
    }
}

void initKingAttacks() {
    for (int i = 0; i < 64; i++) {
        u64 kingboard = 0;
        if (i % 8 != 7) {setBit(kingboard, i+1);}
        if (i % 8 != 0 && i / 8 != 7) {setBit(kingboard, i+7);}
        if (i / 8 != 7) {setBit(kingboard, i+8);}
        if (i % 8 != 7 && i / 8 != 7) {setBit(kingboard, i+9);}
        if (i % 8 != 0) {setBit(kingboard, i-1);}
        if (i % 8 != 7 && i / 8 != 0) {setBit(kingboard, i-7);}
        if (i / 8 != 0) {setBit(kingboard, i-8);}
        if (i % 8 != 0 && i / 8 != 0) {setBit(kingboard, i-9);}

        kingAttacks[i] = kingboard;
    }
}

void initSlidingPieceAttacks(bool isRook) {
    for (int square = 0; square < 64; square++) {
        u64 mask = isRook ? rookAttackMasksNoEdges[square] : bishopAttackMasksNoEdges[square];
        
        int bitCnt = bitCount(mask);
        
        int occupancyVariations = 1 << bitCnt;
        
        for (int count = 0; count < occupancyVariations; count++) {
            if (isRook) {
                u64 occupancy = setOccupancy(count, bitCnt, mask);
                u64 magicIndex = (occupancy * rookMagics[square]) >> (64 - rookRelevantOccupancyAmount[square]);
                rookAttacks[square][magicIndex] = normRookAttacks(square, occupancy);
            } else {
                u64 occupancy = setOccupancy(count, bitCnt, mask);
                u64 magicIndex = (occupancy * bishopMagics[square]) >> (64 - bishopRelevantOccupancyAmount[square]);
                bishopAttacks[square][magicIndex] = normBishopAttacks(square, occupancy);
            }
        }
    }
}

u64 generateRookMoves(int square, u64 occupancy) {
	
	occupancy &= rookAttackMasksNoEdges[square];
	occupancy *= rookMagics[square];
	occupancy >>= 64 - rookRelevantOccupancyAmount[square];
	
	return rookAttacks[square][occupancy];
}

u64 generateBishopMoves(int square, u64 occupancy) {
	
	occupancy &= bishopAttackMasksNoEdges[square];
	occupancy *= bishopMagics[square];
	occupancy >>= 64 - bishopRelevantOccupancyAmount[square];
	
	return bishopAttacks[square][occupancy];
}

void getPins(bitboards_t *BB, u64 *pinned, u8 *pinners) {

    u8 colorOffsetSideToMove = (BB->color == BLACK) * 6;
    u8 colorOffsetOpponent = (BB->color == WHITE) * 6;


    u64 enemyPieces = BB->bits[whitePieces + (BB->color == WHITE)];
    u64 myPieces = BB->bits[whitePieces + (BB->color == BLACK)];
    u64 myKing = BB->bits[whiteKing + colorOffsetSideToMove];

    u8 kingIndex = lsb(myKing);

    for (u64 enemyRooks = BB->bits[whiteRooks + colorOffsetOpponent] | BB->bits[whiteQueens + colorOffsetOpponent]; enemyRooks; enemyRooks &= enemyRooks - 1) {
        u8 b = lsb(enemyRooks);
        if ((rookAttackMasksWithEdges[b] & myKing) && (bitCount(squaresBetween[b][kingIndex] & myPieces) == 1) && (bitCount(squaresBetween[b][kingIndex] & enemyPieces) == 0)) {
            
            u64 pinnedBit = squaresBetween[b][kingIndex] & myPieces;
            *pinned |= pinnedBit;
            pinners[lsb(pinnedBit)] = b;
        }
    }

    for (u64 enemyBishops = BB->bits[whiteBishops + colorOffsetOpponent] | BB->bits[whiteQueens + colorOffsetOpponent]; enemyBishops; enemyBishops &= enemyBishops - 1) {
        
        u8 b = lsb(enemyBishops);

        if ((bishopAttackMasksWithEdges[b] & myKing) && (bitCount(squaresBetween[b][kingIndex] & myPieces) == 1) && (bitCount(squaresBetween[b][kingIndex] & enemyPieces) == 0)) {
            
            u64 pinnedBit = squaresBetween[b][kingIndex] & myPieces;
            *pinned |= pinnedBit;
            pinners[lsb(pinnedBit)] = b;
        }
    }

    // displayBoard(*pinned);
}

u64 getEnemyAttackMask(bitboards_t *BB) {
    
        u64 attacks = 0;
        u64 occupied, enemyKingIndex, capture1, capture2;

        u8 colorOffsetOpponent = (BB->color == WHITE) * 6;

        // the king has to be removed from the occupied squares because it would be seen as a blocker and 
        // could just step away from the check without being inside the attack mask anymore
        occupied = BB->bits[allPieces] ^ BB->bits[whiteKing + (BB->color == BLACK) * 6];

        enemyKingIndex = lsb(BB->bits[whiteKing + colorOffsetOpponent]);
    
        capture1 = (((BB->bits[blackPawns] >> 7) & notRightmostFileMask) * (BB->color == WHITE)) +
                    (((BB->bits[whitePawns] << 7) & notLeftmostFileMask) * (BB->color == BLACK));
        capture2 = (((BB->bits[blackPawns] >> 9) & notLeftmostFileMask) * (BB->color == WHITE)) +
                    (((BB->bits[whitePawns] << 9) & notRightmostFileMask) * (BB->color == BLACK));

        // is branchless like this faster?

        attacks |= kingAttacks[enemyKingIndex] | capture1 | capture2;

        for (u64 enemyKnights = BB->bits[whiteKnights + colorOffsetOpponent]; enemyKnights; enemyKnights &= enemyKnights - 1) {
            u8 b = lsb(enemyKnights);
            attacks |= knightAttacks[b];
        }

        for (u64 enemyBishops = BB->bits[whiteBishops + colorOffsetOpponent] | BB->bits[whiteQueens + colorOffsetOpponent]; enemyBishops; enemyBishops &= enemyBishops - 1) {
            u8 b = lsb(enemyBishops);
            attacks |= generateBishopMoves(b, occupied);
        }

        for (u64 enemyRooks = BB->bits[whiteRooks + colorOffsetOpponent] | BB->bits[whiteQueens + colorOffsetOpponent]; enemyRooks; enemyRooks &= enemyRooks - 1) {
            u8 b = lsb(enemyRooks);
            attacks |= generateRookMoves(b, occupied);
        }
    
        return attacks;
}
                        
int getMoves(bitboards_t *BB, move_t *MOVES, u64 checkers, u64 pinned, u8 *pinners, u64 attacked) {

    bool inCheck = checkers != 0;
    u64 epBit = BB->enPassantSquare < 0 ? 0 : bit(BB->enPassantSquare);
    u8 kingIndex;

    possibleCalls++;

    int count = 0;

    u64 occupied, notMyPieces, step1, step2, capture1, capture2, knights, bishops, rooks, king, pinnedStep1, pinnedStep2, pinnedCapture1, pinnedCapture2, pinnedBishops, pinnedRooks;

    pinnedStep1 = 0;
    pinnedStep2 = 0;
    pinnedCapture1 = 0;
    pinnedCapture2 = 0;

    pinnedBishops = 0;
    pinnedRooks = 0;

    occupied = BB->bits[allPieces];

    bool castleKing;
    bool castleQueen;

    if (BB->color == WHITE) {

        castleKing = BB->whiteCastleKingSide;
        castleQueen = BB->whiteCastleQueenSide;

        notMyPieces = ~BB->bits[whitePieces];
        king = BB->bits[whiteKing];

        if (king == 0) {
            // without the king some of the other functions will break
            printf("\n");
            printBoard(BB);
            printf("\nError: king not found\n");
            exit(1);
        }

        kingIndex = lsb(king);

        if (bitCount(checkers) >= 2) {
            // double check, only king moves are legal
            step1 = 0;
            step2 = 0;
            capture1 = 0;
            capture2 = 0;

            knights = 0;
            bishops = 0;
            rooks = 0;

        } else if (checkers & (BB->bits[blackKnights] | BB->bits[blackPawns] | BB->bits[blackKing])) {
            // single check by non-sliders, only king moves and captures are legal
            step1 = 0;
            step2 = 0;
            capture1 = (((BB->bits[whitePawns] & ~pinned) << 7) & notLeftmostFileMask) & (checkers | epBit); // epBit for en passant check evasion
            capture2 = (((BB->bits[whitePawns] & ~pinned) << 9) & notRightmostFileMask) & (checkers | epBit);

            knights = BB->bits[whiteKnights] & ~pinned;
            bishops = (BB->bits[whiteBishops] | BB->bits[whiteQueens]) & ~pinned;
            rooks = (BB->bits[whiteRooks] | BB->bits[whiteQueens]) & ~pinned;

        } else if (checkers) {
            // single check by sliders, only king moves, captures and blocking moves are legal
            knights = BB->bits[whiteKnights] & ~pinned;
            bishops = (BB->bits[whiteBishops] | BB->bits[whiteQueens]) & ~pinned;
            rooks = (BB->bits[whiteRooks] | BB->bits[whiteQueens]) & ~pinned;

            checkers |= squaresBetween[lsb(checkers)][kingIndex];

            step1 = ((BB->bits[whitePawns] & ~pinned) << 8) & ~occupied;
            step2 = ((step1 & rank[2]) << 8) & ~occupied & checkers;
            step1 &= checkers;
            capture1 = (((BB->bits[whitePawns] & ~pinned) << 7) & notLeftmostFileMask) & (BB->bits[blackPieces] | epBit) & checkers;
            capture2 = (((BB->bits[whitePawns] & ~pinned) << 9) & notRightmostFileMask) & (BB->bits[blackPieces] | epBit) & checkers;

        } else {
            // no checks, all moves are legal (todo: except pinned pieces or castling through check)
            step1 = ((BB->bits[whitePawns] & ~pinned) << 8) & ~occupied;
            step2 = ((step1 & rank[2]) << 8) & ~occupied;
            capture1 = (((BB->bits[whitePawns] & ~pinned) << 7) & notLeftmostFileMask) & (BB->bits[blackPieces] | epBit);
            capture2 = (((BB->bits[whitePawns] & ~pinned) << 9) & notRightmostFileMask) & (BB->bits[blackPieces] | epBit);

            knights = BB->bits[whiteKnights] & ~pinned;
            bishops = (BB->bits[whiteBishops] | BB->bits[whiteQueens]) & ~pinned;
            rooks = (BB->bits[whiteRooks] | BB->bits[whiteQueens]) & ~pinned;
        
            checkers = 0xFFFFFFFFFFFFFFFFULL;

            // some pinned pieces still have moves so they need to be generated here
            pinnedStep1 = ((BB->bits[whitePawns] & pinned) << 8) & ~occupied;
            pinnedStep2 = ((pinnedStep1 & rank[2]) << 8) & ~occupied;
            pinnedCapture1 = (((BB->bits[whitePawns] & pinned) << 7) & notLeftmostFileMask) & (BB->bits[blackPieces] | epBit);
            pinnedCapture2 = (((BB->bits[whitePawns] & pinned) << 9) & notRightmostFileMask) & (BB->bits[blackPieces] | epBit);

            pinnedBishops = (BB->bits[whiteBishops] | BB->bits[whiteQueens]) & pinned;
            pinnedRooks = (BB->bits[whiteRooks] | BB->bits[whiteQueens]) & pinned;
        }

        for (; step1; step1 &= step1 - 1) {
            u8 b = lsb(step1);
            if ((1ULL << b) & pawnPromotionMask) {
                MOVES[count++] = MOVE(b - 8, b, 4, 0, 0);
                MOVES[count++] = MOVE(b - 8, b, 3, 0, 0);
                MOVES[count++] = MOVE(b - 8, b, 2, 0, 0);
                MOVES[count++] = MOVE(b - 8, b, 1, 0, 0);
            } else {
                MOVES[count++] = MOVE(b - 8, b, 0, 0, 0);
            }
        }

        for (; step2; step2 &= step2 - 1) {
            u8 b = lsb(step2);
            MOVES[count++] = MOVE(b - 16, b, 0, 0, 1);
        }

        for (; capture1; capture1 &= capture1 - 1) {
            u8 b = lsb(capture1);
            // watch out for en passant discovered check
            if ((bit(b) == epBit)) {
                if (!((rank[4] & king) && (rank[4] & BB->bits[blackRooks]))) {
                    MOVES[count++] = MOVE(b - 7, b, 0, 0, 0);
                } else if ((generateRookMoves(kingIndex, (occupied ^ (bit(b - 7) | bit(b - 8)))) & (BB->bits[blackRooks] | BB->bits[blackQueens])) == 0) {
                    MOVES[count++] = MOVE(b - 7, b, 0, 0, 0);
                }
            } else if ((1ULL << b) & pawnPromotionMask) {
                MOVES[count++] = MOVE(b - 7, b, 4, 0, 0);
                MOVES[count++] = MOVE(b - 7, b, 3, 0, 0);
                MOVES[count++] = MOVE(b - 7, b, 2, 0, 0);
                MOVES[count++] = MOVE(b - 7, b, 1, 0, 0);
            } else {
                MOVES[count++] = MOVE(b - 7, b, 0, 0, 0);
            }
        }

        for (; capture2; capture2 &= capture2 - 1) {
            u8 b = lsb(capture2);
            if (bit(b) == epBit) {
                if (!((rank[4] & king) && (rank[4] & BB->bits[blackRooks]))) {
                    MOVES[count++] = MOVE(b - 9, b, 0, 0, 0);
                } else if ((generateRookMoves(kingIndex, (occupied ^ (bit(b - 9) | bit(b - 8)))) & (BB->bits[blackRooks] | BB->bits[blackQueens])) == 0) {
                    MOVES[count++] = MOVE(b - 9, b, 0, 0, 0);
                }
            } else if ((1ULL << b) & pawnPromotionMask) {
                MOVES[count++] = MOVE(b - 9, b, 4, 0, 0);
                MOVES[count++] = MOVE(b - 9, b, 3, 0, 0);
                MOVES[count++] = MOVE(b - 9, b, 2, 0, 0);
                MOVES[count++] = MOVE(b - 9, b, 1, 0, 0);
            } else {
                MOVES[count++] = MOVE(b - 9, b, 0, 0, 0);
            }
        }

        for (; pinnedStep1; pinnedStep1 &= pinnedStep1 - 1) {
            u8 b = lsb(pinnedStep1);
            if (bit(b) & squaresBetween[kingIndex][pinners[b - 8]]) {
                MOVES[count++] = MOVE(b - 8, b, 0, 0, 0);
            }
            // there is no promoting here since there is either a rook in front of the pawn or your own king
        }

        for (; pinnedStep2; pinnedStep2 &= pinnedStep2 - 1) {
            u8 b = lsb(pinnedStep2);
            if (bit(b) & squaresBetween[kingIndex][pinners[b - 16]]) {
                MOVES[count++] = MOVE(b - 16, b, 0, 0, 1);
            }
        }

        for (; pinnedCapture1; pinnedCapture1 &= pinnedCapture1 - 1) {
            u8 b = lsb(pinnedCapture1);
            // for captures, promoting and en passant are still possible if the pawn captures in the direction of the bishop pin
            if (bit(b) & (squaresBetween[kingIndex][pinners[b - 7]] | bit(pinners[b - 7]))) {
                // we also dont need to worry about discovered checks here since the pinned capture can only work if the pinning piece is a bishop
                if (bit(b) == epBit) {
                    MOVES[count++] = MOVE(b - 7, b, 0, 0, 0);
                } else if ((1ULL << b) & pawnPromotionMask) {
                    MOVES[count++] = MOVE(b - 7, b, 4, 0, 0);
                    MOVES[count++] = MOVE(b - 7, b, 3, 0, 0);
                    MOVES[count++] = MOVE(b - 7, b, 2, 0, 0);
                    MOVES[count++] = MOVE(b - 7, b, 1, 0, 0);
                } else {
                    MOVES[count++] = MOVE(b - 7, b, 0, 0, 0);
                }
            }
        }

        for (; pinnedCapture2; pinnedCapture2 &= pinnedCapture2 - 1) {
            u8 b = lsb(pinnedCapture2);
            if (bit(b) & (squaresBetween[kingIndex][pinners[b - 9]] | bit(pinners[b - 9]))) {
                if (bit(b) == epBit) {
                    MOVES[count++] = MOVE(b - 9, b, 0, 0, 0);
                } else if ((1ULL << b) & pawnPromotionMask) {
                    MOVES[count++] = MOVE(b - 9, b, 4, 0, 0);
                    MOVES[count++] = MOVE(b - 9, b, 3, 0, 0);
                    MOVES[count++] = MOVE(b - 9, b, 2, 0, 0);
                    MOVES[count++] = MOVE(b - 9, b, 1, 0, 0);
                } else {
                    MOVES[count++] = MOVE(b - 9, b, 0, 0, 0);
                }
            }
        }

    } else {

        castleKing = BB->blackCastleKingSide;
        castleQueen = BB->blackCastleQueenSide;

        notMyPieces = ~BB->bits[blackPieces];
        king = BB->bits[blackKing];

        if (king == 0) {
            // without the king some of the other functions will break
            printf("\n");
            printBoard(BB);
            printf("\nError: king not found\n");
            exit(1);
        }

        kingIndex = lsb(king);

        if (bitCount(checkers) >= 2) {
            step1 = 0;
            step2 = 0;
            capture1 = 0;
            capture2 = 0;

            knights = 0;
            bishops = 0;
            rooks = 0;

        } else if ((checkers & BB->bits[whiteKnights]) || (checkers & BB->bits[whitePawns]) || (checkers & BB->bits[whiteKing])) {
            step1 = 0;
            step2 = 0;
            capture1 = (((BB->bits[blackPawns] & ~pinned) >> 7) & notRightmostFileMask) & (checkers | epBit);
            capture2 = (((BB->bits[blackPawns] & ~pinned) >> 9) & notLeftmostFileMask) & (checkers | epBit);

            knights = BB->bits[blackKnights] & ~pinned;
            bishops = (BB->bits[blackBishops] | BB->bits[blackQueens]) & ~pinned;
            rooks = (BB->bits[blackRooks] | BB->bits[blackQueens]) & ~pinned;
            
        } else if (checkers) {

            knights = BB->bits[blackKnights] & ~pinned;
            bishops = (BB->bits[blackBishops] | BB->bits[blackQueens]) & ~pinned;
            rooks = (BB->bits[blackRooks] | BB->bits[blackQueens]) & ~pinned;

            checkers |= squaresBetween[lsb(checkers)][kingIndex];

            step1 = ((BB->bits[blackPawns] & ~pinned) >> 8) & ~occupied;
            step2 = ((step1 & rank[5]) >> 8) & ~occupied & checkers;
            step1 &= checkers;
            capture1 = (((BB->bits[blackPawns] & ~pinned) >> 7) & notRightmostFileMask) & (BB->bits[whitePieces] | epBit) & checkers;
            capture2 = (((BB->bits[blackPawns] & ~pinned) >> 9) & notLeftmostFileMask) & (BB->bits[whitePieces] | epBit) & checkers;

        } else {

            step1 = ((BB->bits[blackPawns] & ~pinned) >> 8) & ~occupied;
            step2 = ((step1 & rank[5]) >> 8) & ~occupied;
            capture1 = (((BB->bits[blackPawns] & ~pinned) >> 7) & notRightmostFileMask) & (BB->bits[whitePieces] | epBit);
            capture2 = (((BB->bits[blackPawns] & ~pinned) >> 9) & notLeftmostFileMask) & (BB->bits[whitePieces] | epBit);

            knights = BB->bits[blackKnights] & ~pinned;
            bishops = (BB->bits[blackBishops] | BB->bits[blackQueens]) & ~pinned;
            rooks = (BB->bits[blackRooks] | BB->bits[blackQueens]) & ~pinned;

            checkers = 0xFFFFFFFFFFFFFFFFULL;

            pinnedStep1 = ((BB->bits[blackPawns] & pinned) >> 8) & ~occupied;
            pinnedStep2 = ((pinnedStep1 & rank[5]) >> 8) & ~occupied;
            pinnedCapture1 = (((BB->bits[blackPawns] & pinned) >> 7) & notRightmostFileMask) & (BB->bits[whitePieces] | epBit);
            pinnedCapture2 = (((BB->bits[blackPawns] & pinned) >> 9) & notLeftmostFileMask) & (BB->bits[whitePieces] | epBit);

            pinnedBishops = (BB->bits[blackBishops] | BB->bits[blackQueens]) & pinned;
            pinnedRooks = (BB->bits[blackRooks] | BB->bits[blackQueens]) & pinned;
        }

        for (; step1; step1 &= step1 - 1) {
            u8 b = lsb(step1);
            
            if ((1ULL << b) & pawnPromotionMask) {
                MOVES[count++] = MOVE(b + 8, b, 4, 0, 0);
                MOVES[count++] = MOVE(b + 8, b, 3, 0, 0);
                MOVES[count++] = MOVE(b + 8, b, 2, 0, 0);
                MOVES[count++] = MOVE(b + 8, b, 1, 0, 0);
            } else {
                MOVES[count++] = MOVE(b + 8, b, 0, 0, 0);
            }
        }

        for (; step2; step2 &= step2 - 1) {
            u8 b = lsb(step2);
            
            MOVES[count++] = MOVE(b + 16, b, 0, 0, 1);
        }

        for (; capture1; capture1 &= capture1 - 1) {
            u8 b = lsb(capture1);
            
            if (bit(b) == epBit) {
                if (!((rank[3] & king) && (rank[3] & BB->bits[whiteRooks]))) {
                    MOVES[count++] = MOVE(b + 7, b, 0, 0, 0);
                } else if ((generateRookMoves(kingIndex, (occupied ^ (bit(b + 7) | bit(b + 8)))) & (BB->bits[whiteRooks] | BB->bits[whiteQueens])) == 0) {
                    MOVES[count++] = MOVE(b + 7, b, 0, 0, 0);
                }
            } else if ((1ULL << b) & pawnPromotionMask) {
                MOVES[count++] = MOVE(b + 7, b, 4, 0, 0);
                MOVES[count++] = MOVE(b + 7, b, 3, 0, 0);
                MOVES[count++] = MOVE(b + 7, b, 2, 0, 0);
                MOVES[count++] = MOVE(b + 7, b, 1, 0, 0);
            } else {
                MOVES[count++] = MOVE(b + 7, b, 0, 0, 0);
            }
        }

        for (; capture2; capture2 &= capture2 - 1) {
            u8 b = lsb(capture2);
            
            if (bit(b) == epBit) {
                if (!((rank[3] & king) && (rank[3] & BB->bits[whiteRooks]))) {
                    MOVES[count++] = MOVE(b + 9, b, 0, 0, 0);
                } else if ((generateRookMoves(kingIndex, (occupied ^ (bit(b + 9) | bit(b + 8)))) & (BB->bits[whiteRooks] | BB->bits[whiteQueens])) == 0) {
                    MOVES[count++] = MOVE(b + 9, b, 0, 0, 0);
                }
            } else if ((1ULL << b) & pawnPromotionMask) {
                MOVES[count++] = MOVE(b + 9, b, 4, 0, 0);
                MOVES[count++] = MOVE(b + 9, b, 3, 0, 0);
                MOVES[count++] = MOVE(b + 9, b, 2, 0, 0);
                MOVES[count++] = MOVE(b + 9, b, 1, 0, 0);
            } else {
                MOVES[count++] = MOVE(b + 9, b, 0, 0, 0);
            }
        }

        for (; pinnedStep1; pinnedStep1 &= pinnedStep1 - 1) {
            u8 b = lsb(pinnedStep1);
            
            if (bit(b) & squaresBetween[kingIndex][pinners[b + 8]]) {
                MOVES[count++] = MOVE(b + 8, b, 0, 0, 0);
            }
        }

        for (; pinnedStep2; pinnedStep2 &= pinnedStep2 - 1) {
            u8 b = lsb(pinnedStep2);
            
            if (bit(b) & squaresBetween[kingIndex][pinners[b + 16]]) {
                MOVES[count++] = MOVE(b + 16, b, 0, 0, 1);
            }
        }

        for (; pinnedCapture1; pinnedCapture1 &= pinnedCapture1 - 1) {
            u8 b = lsb(pinnedCapture1);
            
            if (bit(b) & (squaresBetween[kingIndex][pinners[b + 7]] | bit(pinners[b + 7]))) {
                if (bit(b) == epBit) {
                    MOVES[count++] = MOVE(b + 7, b, 0, 0, 0);
                } else if ((1ULL << b) & pawnPromotionMask) {
                    MOVES[count++] = MOVE(b + 7, b, 4, 0, 0);
                    MOVES[count++] = MOVE(b + 7, b, 3, 0, 0);
                    MOVES[count++] = MOVE(b + 7, b, 2, 0, 0);
                    MOVES[count++] = MOVE(b + 7, b, 1, 0, 0);
                } else {
                    MOVES[count++] = MOVE(b + 7, b, 0, 0, 0);
                }
            }
        }

        for (; pinnedCapture2; pinnedCapture2 &= pinnedCapture2 - 1) {
            u8 b = lsb(pinnedCapture2);
            
            if (bit(b) & (squaresBetween[kingIndex][pinners[b + 9]] | bit(pinners[b + 9]))) {
                if (bit(b) == epBit) {
                    MOVES[count++] = MOVE(b + 9, b, 0, 0, 0);
                } else if ((1ULL << b) & pawnPromotionMask) {
                    MOVES[count++] = MOVE(b + 9, b, 4, 0, 0);
                    MOVES[count++] = MOVE(b + 9, b, 3, 0, 0);
                    MOVES[count++] = MOVE(b + 9, b, 2, 0, 0);
                    MOVES[count++] = MOVE(b + 9, b, 1, 0, 0);
                } else {
                    MOVES[count++] = MOVE(b + 9, b, 0, 0, 0);
                }
            }
        }
    }

    u64 kingMoves = kingAttacks[kingIndex] & notMyPieces & ~attacked;

    for (; kingMoves; kingMoves &= kingMoves - 1) {
        u8 b = lsb(kingMoves);
        
        MOVES[count++] = MOVE(kingIndex, b, 0, 0, 0);
    }

    // castling

    if (!inCheck) {
        if (castleKing && (((occupied | attacked) & ((bit(kingIndex-1)) | (bit(kingIndex-2)))) == 0)) {

            MOVES[count++] = MOVE(kingIndex, kingIndex-2, 0, KINGSIDE, 0);
        }

        if (castleQueen && (((occupied | attacked) & ((bit(kingIndex+1)) | (bit(kingIndex+2)))) == 0) && ((occupied & (bit(kingIndex+3))) == 0)) {

            MOVES[count++] = MOVE(kingIndex, kingIndex+2, 0, QUEENSIDE, 0);
        }
    }

    for (; knights; knights &= knights - 1) {
        u8 knightIndex = lsb(knights);

        for (u64 knightMoves = knightAttacks[knightIndex] & notMyPieces & checkers; knightMoves; knightMoves &= knightMoves - 1) {
            u8 b = lsb(knightMoves);
            MOVES[count++] = MOVE(knightIndex, b, 0, 0, 0);
        }
    }

    for (; bishops; bishops &= bishops - 1) {
        u8 bishopIndex = lsb(bishops);

        for (u64 bishopMoves = generateBishopMoves(bishopIndex, occupied) & notMyPieces & checkers; bishopMoves; bishopMoves &= bishopMoves - 1) {
            u8 b = lsb(bishopMoves);
            MOVES[count++] = MOVE(bishopIndex, b, 0, 0, 0);
        }
    }

    for (; rooks; rooks &= rooks - 1) {
        u8 rookIndex = lsb(rooks);
        
        for (u64 rookMoves = generateRookMoves(rookIndex, occupied) & notMyPieces & checkers; rookMoves; rookMoves &= rookMoves - 1) {
            u8 b = lsb(rookMoves);
            MOVES[count++] = MOVE(rookIndex, b, 0, 0, 0);
        }
    }

    // move generation for all pinned sliders

    // a pinned knight can never move, so no moves for that

    for (; pinnedBishops; pinnedBishops &= pinnedBishops - 1) {
        u8 bishopIndex = lsb(pinnedBishops);

        for (
            u64 bishopMoves = generateBishopMoves(bishopIndex, occupied) & notMyPieces & (squaresBetween[kingIndex][pinners[bishopIndex]] | bit(pinners[bishopIndex]));
            bishopMoves; 
            bishopMoves &= bishopMoves - 1
            ) {
            u8 b = lsb(bishopMoves);
            
            MOVES[count++] = MOVE(bishopIndex, b, 0, 0, 0);
        }
    }

    for (; pinnedRooks; pinnedRooks &= pinnedRooks - 1) {
        u8 rookIndex = lsb(pinnedRooks);

        for (
            u64 rookMoves = generateRookMoves(rookIndex, occupied) & notMyPieces & (squaresBetween[kingIndex][pinners[rookIndex]] | bit(pinners[rookIndex])); 
            rookMoves; 
            rookMoves &= rookMoves - 1
            ) {
            u8 b = lsb(rookMoves);
            
            MOVES[count++] = MOVE(rookIndex, b, 0, 0, 0);
        }
    }

    return count;
}

int getCaptures(bitboards_t *BB, move_t *CAPTURES, u64 checkers, u64 pinned, u8 *pinners, u64 attacked) {

    bool inCheck = checkers != 0;
    u64 epBit = BB->enPassantSquare < 0 ? 0 : bit(BB->enPassantSquare);
    u8 kingIndex;

    possibleCalls++;

    int count = 0;

    u64 occupied, enemyPieces, capture1, capture2, knights, bishops, rooks, king, pinnedCapture1, pinnedCapture2, pinnedBishops, pinnedRooks;

    pinnedCapture1 = 0;
    pinnedCapture2 = 0;

    pinnedBishops = 0;
    pinnedRooks = 0;

    occupied = BB->bits[allPieces];

    if (BB->color == WHITE) {

        enemyPieces = BB->bits[blackPieces];
        king = BB->bits[whiteKing];

        if (king == 0) {
            // without the king some of the other functions will break
            printf("\n");
            printBoard(BB);
            printf("\nError: king not found\n");
            exit(1);
        }

        kingIndex = lsb(king);

        if (bitCount(checkers) >= 2) {
            capture1 = 0;
            capture2 = 0;

            knights = 0;
            bishops = 0;
            rooks = 0;

        } else if (checkers & (BB->bits[blackKnights] | BB->bits[blackPawns] | BB->bits[blackKing])) {

            capture1 = (((BB->bits[whitePawns] & ~pinned) << 7) & notLeftmostFileMask) & (checkers | epBit);
            capture2 = (((BB->bits[whitePawns] & ~pinned) << 9) & notRightmostFileMask) & (checkers | epBit);

            knights = BB->bits[whiteKnights] & ~pinned;
            bishops = (BB->bits[whiteBishops] | BB->bits[whiteQueens]) & ~pinned;
            rooks = (BB->bits[whiteRooks] | BB->bits[whiteQueens]) & ~pinned;

        } else if (checkers) {
            
            knights = BB->bits[whiteKnights] & ~pinned;
            bishops = (BB->bits[whiteBishops] | BB->bits[whiteQueens]) & ~pinned;
            rooks = (BB->bits[whiteRooks] | BB->bits[whiteQueens]) & ~pinned;

            checkers |= squaresBetween[lsb(checkers)][kingIndex];

            capture1 = (((BB->bits[whitePawns] & ~pinned) << 7) & notLeftmostFileMask) & (BB->bits[blackPieces] | epBit) & checkers;
            capture2 = (((BB->bits[whitePawns] & ~pinned) << 9) & notRightmostFileMask) & (BB->bits[blackPieces] | epBit) & checkers;

        } else {

            capture1 = (((BB->bits[whitePawns] & ~pinned) << 7) & notLeftmostFileMask) & (BB->bits[blackPieces] | epBit);
            capture2 = (((BB->bits[whitePawns] & ~pinned) << 9) & notRightmostFileMask) & (BB->bits[blackPieces] | epBit);

            knights = BB->bits[whiteKnights] & ~pinned;
            bishops = (BB->bits[whiteBishops] | BB->bits[whiteQueens]) & ~pinned;
            rooks = (BB->bits[whiteRooks] | BB->bits[whiteQueens]) & ~pinned;
        
            checkers = 0xFFFFFFFFFFFFFFFFULL;

            // some pinned pieces still have moves so they need to be generated here
            pinnedCapture1 = (((BB->bits[whitePawns] & pinned) << 7) & notLeftmostFileMask) & (BB->bits[blackPieces] | epBit);
            pinnedCapture2 = (((BB->bits[whitePawns] & pinned) << 9) & notRightmostFileMask) & (BB->bits[blackPieces] | epBit);

            pinnedBishops = (BB->bits[whiteBishops] | BB->bits[whiteQueens]) & pinned;
            pinnedRooks = (BB->bits[whiteRooks] | BB->bits[whiteQueens]) & pinned;
        }

        while (capture1) {
            u8 b = lsb(capture1);
            capture1 &= capture1 - 1;
            // watch out for en passant discovered check
            if ((bit(b) == epBit)) {
                if (!((rank[4] & king) && (rank[4] & BB->bits[blackRooks]))) {
                    CAPTURES[count++] = MOVE(b - 7, b, 0, 0, 0);
                } else if ((generateRookMoves(kingIndex, (occupied ^ (bit(b - 7) | bit(b - 8)))) & (BB->bits[blackRooks] | BB->bits[blackQueens])) == 0) {
                    CAPTURES[count++] = MOVE(b - 7, b, 0, 0, 0);
                }
            } else if ((1ULL << b) & pawnPromotionMask) {
                CAPTURES[count++] = MOVE(b - 7, b, 4, 0, 0);
                CAPTURES[count++] = MOVE(b - 7, b, 3, 0, 0);
                CAPTURES[count++] = MOVE(b - 7, b, 2, 0, 0);
                CAPTURES[count++] = MOVE(b - 7, b, 1, 0, 0);
            } else {
                CAPTURES[count++] = MOVE(b - 7, b, 0, 0, 0);
            }
        }

        while (capture2) {
            u8 b = lsb(capture2);
            capture2 &= capture2 - 1;
            if (bit(b) == epBit) {
                if (!((rank[4] & king) && (rank[4] & BB->bits[blackRooks]))) {
                    CAPTURES[count++] = MOVE(b - 9, b, 0, 0, 0);
                } else if ((generateRookMoves(kingIndex, (occupied ^ (bit(b - 9) | bit(b - 8)))) & (BB->bits[blackRooks] | BB->bits[blackQueens])) == 0) {
                    CAPTURES[count++] = MOVE(b - 9, b, 0, 0, 0);
                }
            } else if ((1ULL << b) & pawnPromotionMask) {
                CAPTURES[count++] = MOVE(b - 9, b, 4, 0, 0);
                CAPTURES[count++] = MOVE(b - 9, b, 3, 0, 0);
                CAPTURES[count++] = MOVE(b - 9, b, 2, 0, 0);
                CAPTURES[count++] = MOVE(b - 9, b, 1, 0, 0);
            } else {
                CAPTURES[count++] = MOVE(b - 9, b, 0, 0, 0);
            }
        }

        while (pinnedCapture1) {
            u8 b = lsb(pinnedCapture1);
            pinnedCapture1 &= pinnedCapture1 - 1;
            // for captures, promoting and en passant are still possible if the pawn captures in the direction of the bishop pin
            if (bit(b) & (squaresBetween[kingIndex][pinners[b - 7]] | bit(pinners[b - 7]))) {
                // we also dont need to worry about discovered checks here since the pinned capture can only work if the pinning piece is a bishop
                if (bit(b) == epBit) {
                    CAPTURES[count++] = MOVE(b - 7, b, 0, 0, 0);
                } else if ((1ULL << b) & pawnPromotionMask) {
                    CAPTURES[count++] = MOVE(b - 7, b, 4, 0, 0);
                    CAPTURES[count++] = MOVE(b - 7, b, 3, 0, 0);
                    CAPTURES[count++] = MOVE(b - 7, b, 2, 0, 0);
                    CAPTURES[count++] = MOVE(b - 7, b, 1, 0, 0);
                } else {
                    CAPTURES[count++] = MOVE(b - 7, b, 0, 0, 0);
                }
            }
        }

        while (pinnedCapture2) {
            u8 b = lsb(pinnedCapture2);
            pinnedCapture2 &= pinnedCapture2 - 1;
            if (bit(b) & (squaresBetween[kingIndex][pinners[b - 9]] | bit(pinners[b - 9]))) {
                if (bit(b) == epBit) {
                    CAPTURES[count++] = MOVE(b - 9, b, 0, 0, 0);
                } else if ((1ULL << b) & pawnPromotionMask) {
                    CAPTURES[count++] = MOVE(b - 9, b, 4, 0, 0);
                    CAPTURES[count++] = MOVE(b - 9, b, 3, 0, 0);
                    CAPTURES[count++] = MOVE(b - 9, b, 2, 0, 0);
                    CAPTURES[count++] = MOVE(b - 9, b, 1, 0, 0);
                } else {
                    CAPTURES[count++] = MOVE(b - 9, b, 0, 0, 0);
                }
            }
        }

    } else {

        enemyPieces = BB->bits[whitePieces];
        king = BB->bits[blackKing];

        if (king == 0) {
            // without the king some of the other functions will break
            printf("\n");
            printBoard(BB);
            printf("\nError: king not found\n");
            exit(1);
        }

        kingIndex = lsb(king);

        if (bitCount(checkers) >= 2) {

            capture1 = 0;
            capture2 = 0;

            knights = 0;
            bishops = 0;
            rooks = 0;

        } else if ((checkers & BB->bits[whiteKnights]) || (checkers & BB->bits[whitePawns]) || (checkers & BB->bits[whiteKing])) {

            capture1 = (((BB->bits[blackPawns] & ~pinned) >> 7) & notRightmostFileMask) & (checkers | epBit);
            capture2 = (((BB->bits[blackPawns] & ~pinned) >> 9) & notLeftmostFileMask) & (checkers | epBit);

            knights = BB->bits[blackKnights] & ~pinned;
            bishops = (BB->bits[blackBishops] | BB->bits[blackQueens]) & ~pinned;
            rooks = (BB->bits[blackRooks] | BB->bits[blackQueens]) & ~pinned;
            
        } else if (checkers) {

            knights = BB->bits[blackKnights] & ~pinned;
            bishops = (BB->bits[blackBishops] | BB->bits[blackQueens]) & ~pinned;
            rooks = (BB->bits[blackRooks] | BB->bits[blackQueens]) & ~pinned;

            checkers |= squaresBetween[lsb(checkers)][kingIndex];

            capture1 = (((BB->bits[blackPawns] & ~pinned) >> 7) & notRightmostFileMask) & (BB->bits[whitePieces] | epBit) & checkers;
            capture2 = (((BB->bits[blackPawns] & ~pinned) >> 9) & notLeftmostFileMask) & (BB->bits[whitePieces] | epBit) & checkers;

        } else {

            capture1 = (((BB->bits[blackPawns] & ~pinned) >> 7) & notRightmostFileMask) & (BB->bits[whitePieces] | epBit);
            capture2 = (((BB->bits[blackPawns] & ~pinned) >> 9) & notLeftmostFileMask) & (BB->bits[whitePieces] | epBit);

            knights = BB->bits[blackKnights] & ~pinned;
            bishops = (BB->bits[blackBishops] | BB->bits[blackQueens]) & ~pinned;
            rooks = (BB->bits[blackRooks] | BB->bits[blackQueens]) & ~pinned;

            checkers = 0xFFFFFFFFFFFFFFFFULL;

            pinnedCapture1 = (((BB->bits[blackPawns] & pinned) >> 7) & notRightmostFileMask) & (BB->bits[whitePieces] | epBit);
            pinnedCapture2 = (((BB->bits[blackPawns] & pinned) >> 9) & notLeftmostFileMask) & (BB->bits[whitePieces] | epBit);

            pinnedBishops = (BB->bits[blackBishops] | BB->bits[blackQueens]) & pinned;
            pinnedRooks = (BB->bits[blackRooks] | BB->bits[blackQueens]) & pinned;
        }

        while (capture1) {
            u8 b = lsb(capture1);
            capture1 &= capture1 - 1;
            if (bit(b) == epBit) {
                if (!((rank[3] & king) && (rank[3] & BB->bits[whiteRooks]))) {
                    CAPTURES[count++] = MOVE(b + 7, b, 0, 0, 0);
                } else if ((generateRookMoves(kingIndex, (occupied ^ (bit(b + 7) | bit(b + 8)))) & (BB->bits[whiteRooks] | BB->bits[whiteQueens])) == 0) {
                    CAPTURES[count++] = MOVE(b + 7, b, 0, 0, 0);
                }
            } else if ((1ULL << b) & pawnPromotionMask) {
                CAPTURES[count++] = MOVE(b + 7, b, 4, 0, 0);
                CAPTURES[count++] = MOVE(b + 7, b, 3, 0, 0);
                CAPTURES[count++] = MOVE(b + 7, b, 2, 0, 0);
                CAPTURES[count++] = MOVE(b + 7, b, 1, 0, 0);
            } else {
                CAPTURES[count++] = MOVE(b + 7, b, 0, 0, 0);
            }
        }

        while (capture2) {
            u8 b = lsb(capture2);
            capture2 &= capture2 - 1;
            if (bit(b) == epBit) {
                if (!((rank[3] & king) && (rank[3] & BB->bits[whiteRooks]))) {
                    CAPTURES[count++] = MOVE(b + 9, b, 0, 0, 0);
                } else if ((generateRookMoves(kingIndex, (occupied ^ (bit(b + 9) | bit(b + 8)))) & (BB->bits[whiteRooks] | BB->bits[whiteQueens])) == 0) {
                    CAPTURES[count++] = MOVE(b + 9, b, 0, 0, 0);
                }
            } else if ((1ULL << b) & pawnPromotionMask) {
                CAPTURES[count++] = MOVE(b + 9, b, 4, 0, 0);
                CAPTURES[count++] = MOVE(b + 9, b, 3, 0, 0);
                CAPTURES[count++] = MOVE(b + 9, b, 2, 0, 0);
                CAPTURES[count++] = MOVE(b + 9, b, 1, 0, 0);
            } else {
                CAPTURES[count++] = MOVE(b + 9, b, 0, 0, 0);
            }
        }

        while (pinnedCapture1) {
            u8 b = lsb(pinnedCapture1);
            pinnedCapture1 &= pinnedCapture1 - 1;
            if (bit(b) & (squaresBetween[kingIndex][pinners[b + 7]] | bit(pinners[b + 7]))) {
                if (bit(b) == epBit) {
                    CAPTURES[count++] = MOVE(b + 7, b, 0, 0, 0);
                } else if ((1ULL << b) & pawnPromotionMask) {
                    CAPTURES[count++] = MOVE(b + 7, b, 4, 0, 0);
                    CAPTURES[count++] = MOVE(b + 7, b, 3, 0, 0);
                    CAPTURES[count++] = MOVE(b + 7, b, 2, 0, 0);
                    CAPTURES[count++] = MOVE(b + 7, b, 1, 0, 0);
                } else {
                    CAPTURES[count++] = MOVE(b + 7, b, 0, 0, 0);
                }
            }
        }

        while (pinnedCapture2) {
            u8 b = lsb(pinnedCapture2);
            pinnedCapture2 &= pinnedCapture2 - 1;
            if (bit(b) & (squaresBetween[kingIndex][pinners[b + 9]] | bit(pinners[b + 9]))) {
                if (bit(b) == epBit) {
                    CAPTURES[count++] = MOVE(b + 9, b, 0, 0, 0);
                } else if ((1ULL << b) & pawnPromotionMask) {
                    CAPTURES[count++] = MOVE(b + 9, b, 4, 0, 0);
                    CAPTURES[count++] = MOVE(b + 9, b, 3, 0, 0);
                    CAPTURES[count++] = MOVE(b + 9, b, 2, 0, 0);
                    CAPTURES[count++] = MOVE(b + 9, b, 1, 0, 0);
                } else {
                    CAPTURES[count++] = MOVE(b + 9, b, 0, 0, 0);
                }
            }
        }
    }

    u64 kingMoves = kingAttacks[kingIndex] & enemyPieces & ~attacked;

    while (kingMoves) {
        u8 b = lsb(kingMoves);
        kingMoves &= kingMoves - 1;
        CAPTURES[count++] = MOVE(kingIndex, b, 0, 0, 0);
    }

    while (knights) {
        u8 knightIndex = lsb(knights);
        knights &= knights - 1;
        u64 knightMoves = knightAttacks[knightIndex] & enemyPieces & checkers;

        while (knightMoves) {
            u8 b = lsb(knightMoves);
            knightMoves &= knightMoves - 1;
            CAPTURES[count++] = MOVE(knightIndex, b, 0, 0, 0);
        }
    }

    while (bishops) {
        u8 bishopIndex = lsb(bishops);
        bishops &= bishops - 1;
        u64 bishopMoves = generateBishopMoves(bishopIndex, occupied) & enemyPieces & checkers;

        while (bishopMoves) {
            u8 b = lsb(bishopMoves);
            bishopMoves &= bishopMoves - 1;
            CAPTURES[count++] = MOVE(bishopIndex, b, 0, 0, 0);
        }
    }

    while (rooks) {
        u8 rookIndex = lsb(rooks);
        rooks &= rooks - 1;
        u64 rookMoves = generateRookMoves(rookIndex, occupied) & enemyPieces & checkers;

        while (rookMoves) {
            u8 b = lsb(rookMoves);
            rookMoves &= rookMoves - 1;
            CAPTURES[count++] = MOVE(rookIndex, b, 0, 0, 0);
        }
    }

    // move generation for all pinned sliders

    // a pinned knight can never move, so no moves for that

    while (pinnedBishops) {
        u8 bishopIndex = lsb(pinnedBishops);
        pinnedBishops &= pinnedBishops - 1;
        u64 bishopMoves = generateBishopMoves(bishopIndex, occupied) & enemyPieces & 
                          (squaresBetween[kingIndex][pinners[bishopIndex]] | bit(pinners[bishopIndex]));

        while (bishopMoves) {
            u8 b = lsb(bishopMoves);
            bishopMoves &= bishopMoves - 1;
            CAPTURES[count++] = MOVE(bishopIndex, b, 0, 0, 0);
        }
    }

    while (pinnedRooks) {
        u8 rookIndex = lsb(pinnedRooks);
        pinnedRooks &= pinnedRooks - 1;
        u64 rookMoves = generateRookMoves(rookIndex, occupied) & enemyPieces & 
                        (squaresBetween[kingIndex][pinners[rookIndex]] | bit(pinners[rookIndex]));

        while (rookMoves) {
            u8 b = lsb(rookMoves);
            rookMoves &= rookMoves - 1;
            CAPTURES[count++] = MOVE(rookIndex, b, 0, 0, 0);
        }
    }

    return count;
}

u64 dangerMasks[64] = {0};

void initDangerMasks() {
    for (int i = 0; i < 64; i++) {
        dangerMasks[i] |= kingAttacks[i];
        dangerMasks[i] |= knightAttacks[i];

        if (i % 8 >= 2) {
            dangerMasks[i] |= (1ULL << (i - 2));
        }
        if (i % 8 <= 5) {
            dangerMasks[i] |= (1ULL << (i + 2));
        }
        if (i / 8 >= 2) {
            dangerMasks[i] |= (1ULL << (i - 16));
        }
        if (i / 8 <= 5) {
            dangerMasks[i] |= (1ULL << (i + 16));
        }
    }
}

bool isInCheck2(struct bitboards_t *BITBOARDS, u64 *checkers) {
    int colorOffsetSideToMove = ((BITBOARDS->color == BLACK) * 6);
    int colorOffsetOpponent = ((BITBOARDS->color == WHITE) * 6);

    int playerKingIndex = lsb(BITBOARDS->bits[whiteKing + colorOffsetSideToMove]);

    // same as before only this time we check if the side to move is in check and used on the board before making a move
    // mainly to see if a player can castle or not

    *checkers = (

        (kingAttacks[playerKingIndex] & BITBOARDS->bits[whiteKing + colorOffsetOpponent]) | 
        (knightAttacks[playerKingIndex] & BITBOARDS->bits[whiteKnights + colorOffsetOpponent]) |
        (generateBishopMoves(playerKingIndex, BITBOARDS->bits[allPieces]) & (BITBOARDS->bits[whiteBishops + colorOffsetOpponent] | BITBOARDS->bits[whiteQueens + colorOffsetOpponent])) |
        (generateRookMoves(playerKingIndex, BITBOARDS->bits[allPieces]) & (BITBOARDS->bits[whiteRooks + colorOffsetOpponent] | BITBOARDS->bits[whiteQueens + colorOffsetOpponent])) |
        (
            (BITBOARDS->color == BLACK) *
            (((BITBOARDS->bits[whiteKing + colorOffsetSideToMove] >> 7) & notRightmostFileMask) | ((BITBOARDS->bits[whiteKing + colorOffsetSideToMove] >> 9) & notLeftmostFileMask)) & BITBOARDS->bits[whitePawns + colorOffsetOpponent]
        ) | 
        (
            (BITBOARDS->color == WHITE) *
            (((BITBOARDS->bits[whiteKing + colorOffsetSideToMove] << 7) & notLeftmostFileMask) | ((BITBOARDS->bits[whiteKing + colorOffsetSideToMove] << 9) & notRightmostFileMask)) & BITBOARDS->bits[whitePawns + colorOffsetOpponent]
        )

    );

    return *checkers != 0;
}

void getMobility(bitboards_t *BB, int *mobility, int *kingSafety, int *kingUnSafety) {

    u64 checkers;
    bool inCheck = isInCheck2(BB, &checkers);
    u64 pinned = 0;
    u8 pinners[64] = {0};
    getPins(BB, &pinned, &pinners[0]);
    u64 attacked = getEnemyAttackMask(BB);

    u64 epBit = BB->enPassantSquare < 0 ? 0 : bit(BB->enPassantSquare);
    u8 kingIndex;

    u64 occupied, notMyPieces, step1, step2, capture1, capture2, knights, bishops, rooks,
        king, pinnedStep1, pinnedStep2, pinnedCapture1, pinnedCapture2, pinnedBishops, pinnedRooks;

    pinnedStep1 = 0;
    pinnedStep2 = 0;
    pinnedCapture1 = 0;
    pinnedCapture2 = 0;

    pinnedBishops = 0;
    pinnedRooks = 0;

    occupied = BB->bits[allPieces];

    bool castleKing;
    bool castleQueen;

    u64 dangerMyKing;
    u64 dangerEnemyKing;
    u64 kingRingMyKing;
    u64 kingRingEnemyKing;

    if (BB->color == WHITE) {

        castleKing = BB->whiteCastleKingSide;
        castleQueen = BB->whiteCastleQueenSide;

        notMyPieces = ~BB->bits[whitePieces];
        king = BB->bits[whiteKing];

        if (king == 0) {
            // without the king some of the other functions will break
            printf("\n");
            printBoard(BB);
            printf("\nError: king not found\n");
            exit(1);
        }

        kingIndex = lsb(king);

        dangerMyKing = dangerMasks[kingIndex];
        dangerEnemyKing = dangerMasks[lsb(BB->bits[blackKing])];
        kingRingMyKing = kingAttacks[kingIndex];
        kingRingEnemyKing = kingAttacks[lsb(BB->bits[blackKing])];

        if (bitCount(checkers) >= 2) {
            // double check, only king moves are legal
            step1 = 0;
            step2 = 0;
            capture1 = 0;
            capture2 = 0;

            knights = 0;
            bishops = 0;
            rooks = 0;

        } else if (checkers & (BB->bits[blackKnights] | BB->bits[blackPawns] | BB->bits[blackKing])) {
            // single check by non-sliders, only king moves and captures are legal
            step1 = 0;
            step2 = 0;
            capture1 = (((BB->bits[whitePawns] & ~pinned) << 7) & notLeftmostFileMask) & (checkers | epBit); // epBit for en passant check evasion
            capture2 = (((BB->bits[whitePawns] & ~pinned) << 9) & notRightmostFileMask) & (checkers | epBit);

            knights = BB->bits[whiteKnights] & ~pinned;
            bishops = (BB->bits[whiteBishops] | BB->bits[whiteQueens]) & ~pinned;
            rooks = (BB->bits[whiteRooks] | BB->bits[whiteQueens]) & ~pinned;

        } else if (checkers) {
            // single check by sliders, only king moves, captures and blocking moves are legal
            knights = BB->bits[whiteKnights] & ~pinned;
            bishops = (BB->bits[whiteBishops] | BB->bits[whiteQueens]) & ~pinned;
            rooks = (BB->bits[whiteRooks] | BB->bits[whiteQueens]) & ~pinned;

            checkers |= squaresBetween[lsb(checkers)][kingIndex];

            step1 = ((BB->bits[whitePawns] & ~pinned) << 8) & ~occupied;
            step2 = ((step1 & rank[2]) << 8) & ~occupied & checkers;
            step1 &= checkers;
            capture1 = (((BB->bits[whitePawns] & ~pinned) << 7) & notLeftmostFileMask) & (BB->bits[blackPieces] | epBit) & checkers;
            capture2 = (((BB->bits[whitePawns] & ~pinned) << 9) & notRightmostFileMask) & (BB->bits[blackPieces] | epBit) & checkers;

        } else {
            // no checks, all moves are legal (todo: except pinned pieces or castling through check)
            step1 = ((BB->bits[whitePawns] & ~pinned) << 8) & ~occupied;
            step2 = ((step1 & rank[2]) << 8) & ~occupied;
            capture1 = (((BB->bits[whitePawns] & ~pinned) << 7) & notLeftmostFileMask) & (BB->bits[blackPieces] | epBit);
            capture2 = (((BB->bits[whitePawns] & ~pinned) << 9) & notRightmostFileMask) & (BB->bits[blackPieces] | epBit);

            knights = BB->bits[whiteKnights] & ~pinned;
            bishops = (BB->bits[whiteBishops] | BB->bits[whiteQueens]) & ~pinned;
            rooks = (BB->bits[whiteRooks] | BB->bits[whiteQueens]) & ~pinned;
        
            checkers = 0xFFFFFFFFFFFFFFFFULL;

            // some pinned pieces still have moves so they need to be generated here
            pinnedStep1 = ((BB->bits[whitePawns] & pinned) << 8) & ~occupied;
            pinnedStep2 = ((pinnedStep1 & rank[2]) << 8) & ~occupied;
            pinnedCapture1 = (((BB->bits[whitePawns] & pinned) << 7) & notLeftmostFileMask) & (BB->bits[blackPieces] | epBit);
            pinnedCapture2 = (((BB->bits[whitePawns] & pinned) << 9) & notRightmostFileMask) & (BB->bits[blackPieces] | epBit);

            pinnedBishops = (BB->bits[whiteBishops] | BB->bits[whiteQueens]) & pinned;
            pinnedRooks = (BB->bits[whiteRooks] | BB->bits[whiteQueens]) & pinned;
        }

        while (step1) {
            u8 b = lsb(step1);
            step1 &= step1 - 1;
            if ((1ULL << b) & pawnPromotionMask) {
                *mobility += 4;
                *kingUnSafety -= 4 * ((dangerEnemyKing & bit(b)) > 0);
                *kingUnSafety -= (kingRingEnemyKing & bit(b)) > 0;
            } else {
                *mobility += 1;
                *kingUnSafety -= (dangerEnemyKing & bit(b)) > 0;
                *kingUnSafety -= (kingRingEnemyKing & bit(b)) > 0;
            }
        }

        *mobility += bitCount(step2);
        *kingUnSafety -= bitCount(dangerEnemyKing & step2);
        *kingUnSafety -= bitCount(kingRingEnemyKing & step2);

        while (capture1) {
            u8 b = lsb(capture1);
            capture1 &= capture1 - 1;
            // watch out for en passant discovered check
            if ((bit(b) == epBit)) {
                if (!((rank[4] & king) && (rank[4] & BB->bits[blackRooks]))) {
                    *mobility += 1;
                    *kingUnSafety -= (dangerEnemyKing & bit(b)) > 0;
                    *kingUnSafety -= (kingRingEnemyKing & bit(b)) > 0;
                } else if ((generateRookMoves(kingIndex, (occupied ^ (bit(b - 7) | bit(b - 8)))) & (BB->bits[blackRooks] | BB->bits[blackQueens])) == 0) {
                    *mobility += 1;
                    *kingUnSafety -= (dangerEnemyKing & bit(b)) > 0;
                    *kingUnSafety -= (kingRingEnemyKing & bit(b)) > 0;
                }
            } else if ((1ULL << b) & pawnPromotionMask) {
                *mobility += 4;
                *kingUnSafety -= 4 * ((dangerEnemyKing & bit(b)) > 0);
                *kingUnSafety -= (kingRingEnemyKing & bit(b)) > 0;
            } else {
                *mobility += 1;
                *kingUnSafety -= (dangerEnemyKing & bit(b)) > 0;
                *kingUnSafety -= (kingRingEnemyKing & bit(b)) > 0;
            }
        }

        while (capture2) {
            u8 b = lsb(capture2);
            capture2 &= capture2 - 1;
            if (bit(b) == epBit) {
                if (!((rank[4] & king) && (rank[4] & BB->bits[blackRooks]))) {
                    *mobility += 1;
                    *kingUnSafety -= (dangerEnemyKing & bit(b)) > 0;
                    *kingUnSafety -= (kingRingEnemyKing & bit(b)) > 0;
                } else if ((generateRookMoves(kingIndex, (occupied ^ (bit(b - 9) | bit(b - 8)))) & (BB->bits[blackRooks] | BB->bits[blackQueens])) == 0) {
                    *mobility += 1;
                    *kingUnSafety -= (dangerEnemyKing & bit(b)) > 0;
                    *kingUnSafety -= (kingRingEnemyKing & bit(b)) > 0;
                }
            } else if ((1ULL << b) & pawnPromotionMask) {
                *mobility += 4;
                *kingUnSafety -= 4 * ((dangerEnemyKing & bit(b)) > 0);
                *kingUnSafety -= (kingRingEnemyKing & bit(b)) > 0;
            } else {
                *mobility += 1;
                *kingUnSafety -= (dangerEnemyKing & bit(b)) > 0;
                *kingUnSafety -= (kingRingEnemyKing & bit(b)) > 0;
            }
        }

        while (pinnedStep1) {
            u8 b = lsb(pinnedStep1);
            pinnedStep1 &= pinnedStep1 - 1;
            if (bit(b) & squaresBetween[kingIndex][pinners[b - 8]]) {
                *mobility += 1;
                *kingUnSafety -= (dangerEnemyKing & bit(b)) > 0;
                *kingUnSafety -= (kingRingEnemyKing & bit(b)) > 0;
            }
        }

        while (pinnedStep2) {
            u8 b = lsb(pinnedStep2);
            pinnedStep2 &= pinnedStep2 - 1;
            if (bit(b) & squaresBetween[kingIndex][pinners[b - 16]]) {
                *mobility += 1;
                *kingUnSafety -= (dangerEnemyKing & bit(b)) > 0;
                *kingUnSafety -= (kingRingEnemyKing & bit(b)) > 0;
            }
        }

        while (pinnedCapture1) {
            u8 b = lsb(pinnedCapture1);
            pinnedCapture1 &= pinnedCapture1 - 1;
            if (bit(b) & (squaresBetween[kingIndex][pinners[b - 7]] | bit(pinners[b - 7]))) {
                if ((1ULL << b) & pawnPromotionMask) {
                    *mobility += 4;
                    *kingUnSafety -= 4 * ((dangerEnemyKing & bit(b)) > 0);
                    *kingUnSafety -= (kingRingEnemyKing & bit(b)) > 0;
                } else {
                    *mobility += 1;
                    *kingUnSafety -= (dangerEnemyKing & bit(b)) > 0;
                    *kingUnSafety -= (kingRingEnemyKing & bit(b)) > 0;
                }
            }
        }

        while (pinnedCapture2) {
            u8 b = lsb(pinnedCapture2);
            pinnedCapture2 &= pinnedCapture2 - 1;
            if (bit(b) & (squaresBetween[kingIndex][pinners[b - 9]] | bit(pinners[b - 9]))) {
                if ((1ULL << b) & pawnPromotionMask) {
                    *mobility += 4;
                    *kingUnSafety -= 4 * ((dangerEnemyKing & bit(b)) > 0);
                    *kingUnSafety -= (kingRingEnemyKing & bit(b)) > 0;
                } else {
                    *mobility += 1;
                    *kingUnSafety -= (dangerEnemyKing & bit(b)) > 0;
                    *kingUnSafety -= (kingRingEnemyKing & bit(b)) > 0;
                }
            }
        }

        *kingSafety += bitCount(BB->bits[whitePieces] & dangerMyKing);
        *kingSafety += bitCount(BB->bits[whitePieces] & kingRingMyKing);

    } else {

        castleKing = BB->blackCastleKingSide;
        castleQueen = BB->blackCastleQueenSide;

        notMyPieces = ~BB->bits[blackPieces];
        king = BB->bits[blackKing];

        if (king == 0) {
            // without the king some of the other functions will break
            printf("\n");
            printBoard(BB);
            printf("\nError: king not found\n");
            exit(1);
        }

        kingIndex = lsb(king);

        dangerMyKing = dangerMasks[kingIndex];
        dangerEnemyKing = dangerMasks[lsb(BB->bits[whiteKing])];
        kingRingMyKing = kingAttacks[kingIndex];
        kingRingEnemyKing = kingAttacks[lsb(BB->bits[whiteKing])];

        if (bitCount(checkers) >= 2) {
            step1 = 0;
            step2 = 0;
            capture1 = 0;
            capture2 = 0;

            knights = 0;
            bishops = 0;
            rooks = 0;

        } else if ((checkers & BB->bits[whiteKnights]) || (checkers & BB->bits[whitePawns]) || (checkers & BB->bits[whiteKing])) {
            step1 = 0;
            step2 = 0;
            capture1 = (((BB->bits[blackPawns] & ~pinned) >> 7) & notRightmostFileMask) & (checkers | epBit);
            capture2 = (((BB->bits[blackPawns] & ~pinned) >> 9) & notLeftmostFileMask) & (checkers | epBit);

            knights = BB->bits[blackKnights] & ~pinned;
            bishops = (BB->bits[blackBishops] | BB->bits[blackQueens]) & ~pinned;
            rooks = (BB->bits[blackRooks] | BB->bits[blackQueens]) & ~pinned;
            
        } else if (checkers) {

            knights = BB->bits[blackKnights] & ~pinned;
            bishops = (BB->bits[blackBishops] | BB->bits[blackQueens]) & ~pinned;
            rooks = (BB->bits[blackRooks] | BB->bits[blackQueens]) & ~pinned;

            checkers |= squaresBetween[lsb(checkers)][kingIndex];

            step1 = ((BB->bits[blackPawns] & ~pinned) >> 8) & ~occupied;
            step2 = ((step1 & rank[5]) >> 8) & ~occupied & checkers;
            step1 &= checkers;
            capture1 = (((BB->bits[blackPawns] & ~pinned) >> 7) & notRightmostFileMask) & (BB->bits[whitePieces] | epBit) & checkers;
            capture2 = (((BB->bits[blackPawns] & ~pinned) >> 9) & notLeftmostFileMask) & (BB->bits[whitePieces] | epBit) & checkers;

        } else {

            step1 = ((BB->bits[blackPawns] & ~pinned) >> 8) & ~occupied;
            step2 = ((step1 & rank[5]) >> 8) & ~occupied;
            capture1 = (((BB->bits[blackPawns] & ~pinned) >> 7) & notRightmostFileMask) & (BB->bits[whitePieces] | epBit);
            capture2 = (((BB->bits[blackPawns] & ~pinned) >> 9) & notLeftmostFileMask) & (BB->bits[whitePieces] | epBit);

            knights = BB->bits[blackKnights] & ~pinned;
            bishops = (BB->bits[blackBishops] | BB->bits[blackQueens]) & ~pinned;
            rooks = (BB->bits[blackRooks] | BB->bits[blackQueens]) & ~pinned;

            checkers = 0xFFFFFFFFFFFFFFFFULL;

            pinnedStep1 = ((BB->bits[blackPawns] & pinned) >> 8) & ~occupied;
            pinnedStep2 = ((pinnedStep1 & rank[5]) >> 8) & ~occupied;
            pinnedCapture1 = (((BB->bits[blackPawns] & pinned) >> 7) & notRightmostFileMask) & (BB->bits[whitePieces] | epBit);
            pinnedCapture2 = (((BB->bits[blackPawns] & pinned) >> 9) & notLeftmostFileMask) & (BB->bits[whitePieces] | epBit);

            pinnedBishops = (BB->bits[blackBishops] | BB->bits[blackQueens]) & pinned;
            pinnedRooks = (BB->bits[blackRooks] | BB->bits[blackQueens]) & pinned;
        }

        while (step1) {
            u8 b = lsb(step1);
            step1 &= step1 - 1;
            if ((1ULL << b) & pawnPromotionMask) {
                *mobility += 4;
                *kingUnSafety -= 4 * ((dangerEnemyKing & bit(b)) > 0);
                *kingUnSafety -= (kingRingEnemyKing & bit(b)) > 0;
            } else {
                *mobility += 1;
                *kingUnSafety -= (dangerEnemyKing & bit(b)) > 0;
                *kingUnSafety -= (kingRingEnemyKing & bit(b)) > 0;
            }
        }

        *mobility += bitCount(step2);
        *kingUnSafety -= bitCount(dangerEnemyKing & step2);
        *kingUnSafety -= bitCount(kingRingEnemyKing & step2);

        while (capture1) {
            u8 b = lsb(capture1);
            capture1 &= capture1 - 1;
            if (bit(b) == epBit) {
                if (!((rank[3] & king) && (rank[3] & BB->bits[whiteRooks]))) {
                    *mobility += 1;
                    *kingUnSafety -= (dangerEnemyKing & bit(b)) > 0;
                    *kingUnSafety -= (kingRingEnemyKing & bit(b)) > 0;
                } else if ((generateRookMoves(kingIndex, (occupied ^ (bit(b + 7) | bit(b + 8)))) & (BB->bits[whiteRooks] | BB->bits[whiteQueens])) == 0) {
                    *mobility += 1;
                    *kingUnSafety -= (dangerEnemyKing & bit(b)) > 0;
                    *kingUnSafety -= (kingRingEnemyKing & bit(b)) > 0;
                }
            } else if ((1ULL << b) & pawnPromotionMask) {
                *mobility += 4;
                *kingUnSafety -= 4 * ((dangerEnemyKing & bit(b)) > 0);
                *kingUnSafety -= (kingRingEnemyKing & bit(b)) > 0;
            } else {
                *mobility += 1;
                *kingUnSafety -= (dangerEnemyKing & bit(b)) > 0;
                *kingUnSafety -= (kingRingEnemyKing & bit(b)) > 0;
            }
        }

        while (capture2) {
            u8 b = lsb(capture2);
            capture2 &= capture2 - 1;
            if (bit(b) == epBit) {
                if (!((rank[3] & king) && (rank[3] & BB->bits[whiteRooks]))) {
                    *mobility += 1;
                    *kingUnSafety -= (dangerEnemyKing & bit(b)) > 0;
                    *kingUnSafety -= (kingRingEnemyKing & bit(b)) > 0;
                } else if ((generateRookMoves(kingIndex, (occupied ^ (bit(b + 9) | bit(b + 8)))) & (BB->bits[whiteRooks] | BB->bits[whiteQueens])) == 0) {
                    *mobility += 1;
                    *kingUnSafety -= (dangerEnemyKing & bit(b)) > 0;
                    *kingUnSafety -= (kingRingEnemyKing & bit(b)) > 0;
                }
            } else if ((1ULL << b) & pawnPromotionMask) {
                *mobility += 4;
                *kingUnSafety -= 4 * ((dangerEnemyKing & bit(b)) > 0);
                *kingUnSafety -= (kingRingEnemyKing & bit(b)) > 0;
            } else {
                *mobility += 1;
                *kingUnSafety -= (dangerEnemyKing & bit(b)) > 0;
                *kingUnSafety -= (kingRingEnemyKing & bit(b)) > 0;
            }
        }

        while (pinnedStep1) {
            u8 b = lsb(pinnedStep1);
            pinnedStep1 &= pinnedStep1 - 1;
            if (bit(b) & squaresBetween[kingIndex][pinners[b + 8]]) {
                *mobility += 1;
                *kingUnSafety -= (dangerEnemyKing & bit(b)) > 0;
                *kingUnSafety -= (kingRingEnemyKing & bit(b)) > 0;
            }
        }

        while (pinnedStep2) {
            u8 b = lsb(pinnedStep2);
            pinnedStep2 &= pinnedStep2 - 1;
            if (bit(b) & squaresBetween[kingIndex][pinners[b + 16]]) {
                *mobility += 1;
                *kingUnSafety -= (dangerEnemyKing & bit(b)) > 0;
                *kingUnSafety -= (kingRingEnemyKing & bit(b)) > 0;
            }
        }

        while (pinnedCapture1) {
            u8 b = lsb(pinnedCapture1);
            pinnedCapture1 &= pinnedCapture1 - 1;
            if (bit(b) & (squaresBetween[kingIndex][pinners[b + 7]] | bit(pinners[b + 7]))) {
                if ((1ULL << b) & pawnPromotionMask) {
                    *mobility += 4;
                    *kingUnSafety -= 4 * ((dangerEnemyKing & bit(b)) > 0);
                    *kingUnSafety -= (kingRingEnemyKing & bit(b)) > 0;
                } else {
                    *mobility += 1;
                    *kingUnSafety -= (dangerEnemyKing & bit(b)) > 0;
                    *kingUnSafety -= (kingRingEnemyKing & bit(b)) > 0;
                }
            }
        }

        while (pinnedCapture2) {
            u8 b = lsb(pinnedCapture2);
            pinnedCapture2 &= pinnedCapture2 - 1;
            if (bit(b) & (squaresBetween[kingIndex][pinners[b + 9]] | bit(pinners[b + 9]))) {
                if ((1ULL << b) & pawnPromotionMask) {
                    *mobility += 4;
                    *kingUnSafety -= 4 * ((dangerEnemyKing & bit(b)) > 0);
                    *kingUnSafety -= (kingRingEnemyKing & bit(b)) > 0;
                } else {
                    *mobility += 1;
                    *kingUnSafety -= (dangerEnemyKing & bit(b)) > 0;
                    *kingUnSafety -= (kingRingEnemyKing & bit(b)) > 0;
                }
            }
        }
    
        *kingSafety += bitCount(BB->bits[blackPieces] & dangerMyKing);
        *kingSafety += bitCount(BB->bits[blackPieces] & kingRingMyKing);
    }

    *mobility += bitCount(kingAttacks[kingIndex] & notMyPieces & ~attacked);
    *kingUnSafety -= bitCount(kingAttacks[kingIndex] & dangerEnemyKing);
    *kingUnSafety -= bitCount(kingAttacks[kingIndex] & kingRingEnemyKing);

    // castling is only available when the flags are set and the squares are empty, check detection is done in the search tree

    if (!inCheck && castleKing && (((occupied | attacked) & ((bit(kingIndex-1)) | (bit(kingIndex-2)))) == 0)) {
        *mobility += 1;
        *kingSafety += 2;
        
        u64 newRookAtk = generateRookMoves(kingIndex-1, (occupied ^ (bit(kingIndex) | bit(kingIndex-2))));
        *kingUnSafety -= bitCount(newRookAtk & dangerEnemyKing);
        *kingUnSafety -= bitCount(newRookAtk & kingRingEnemyKing);
    }

    if (!inCheck && castleQueen && (((occupied | attacked) & ((bit(kingIndex+1)) | (bit(kingIndex+2)))) == 0) && ((occupied & (bit(kingIndex+3))) == 0)) {
        *mobility += 1;
        *kingSafety += 2;

        u64 newRookAtk = generateRookMoves(kingIndex+1, (occupied ^ (bit(kingIndex) | bit(kingIndex+2))));
        *kingUnSafety -= bitCount(newRookAtk & dangerEnemyKing);
        *kingUnSafety -= bitCount(newRookAtk & kingRingEnemyKing);
    }

    while (knights) {
        u8 knightIndex = lsb(knights);
        knights &= knights - 1;
        u64 knightAtk = knightAttacks[knightIndex] & notMyPieces & checkers;
        *mobility += bitCount(knightAtk);
        *kingUnSafety -= bitCount(knightAtk & dangerEnemyKing);
        *kingUnSafety -= bitCount(knightAtk & kingRingEnemyKing);
    }

    while (bishops) {
        u8 bishopIndex = lsb(bishops);
        bishops &= bishops - 1;
        u64 bishopAtk = generateBishopMoves(bishopIndex, occupied) & notMyPieces & checkers;
        *mobility += bitCount(bishopAtk);
        *kingUnSafety -= bitCount(bishopAtk & dangerEnemyKing);
        *kingUnSafety -= bitCount(bishopAtk & kingRingEnemyKing);
    }

    while (rooks) {
        u8 rookIndex = lsb(rooks);
        rooks &= rooks - 1;
        u64 rookAtk = generateRookMoves(rookIndex, occupied) & notMyPieces & checkers;
        *mobility += bitCount(rookAtk);
        *kingUnSafety -= bitCount(rookAtk & dangerEnemyKing);
        *kingUnSafety -= bitCount(rookAtk & kingRingEnemyKing);
    }

    while (pinnedBishops) {
        u8 bishopIndex = lsb(pinnedBishops);
        pinnedBishops &= pinnedBishops - 1;
        u64 bishopAtk = generateBishopMoves(bishopIndex, occupied) & notMyPieces & 
                                           (squaresBetween[kingIndex][pinners[bishopIndex]] | bit(pinners[bishopIndex]));
        *mobility += bitCount(bishopAtk);
        *kingUnSafety -= bitCount(bishopAtk & dangerEnemyKing);
        *kingUnSafety -= bitCount(bishopAtk & kingRingEnemyKing);

    }

    while (pinnedRooks) {
        u8 rookIndex = lsb(pinnedRooks);
        pinnedRooks &= pinnedRooks - 1;
        u64 rookAtk = generateRookMoves(rookIndex, occupied) & notMyPieces & 
                                       (squaresBetween[kingIndex][pinners[rookIndex]] | bit(pinners[rookIndex]));
        *mobility += bitCount(rookAtk);
        *kingUnSafety -= bitCount(rookAtk & dangerEnemyKing);
        *kingUnSafety -= bitCount(rookAtk & kingRingEnemyKing);
    }
}
