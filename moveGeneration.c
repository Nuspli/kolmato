#include "moveGeneration.h"

u64 rightmostFileMask = 0x0101010101010101;
u64 leftmostFileMask = 0x8080808080808080;
u64 whitePawnStartRank = 0x0000000000FF0000; // or 1 below actually
u64 blackPawnStartRank = 0x0000FF0000000000;
u64 pawnPromotionMask = 0xFF000000000000FF;

u64 knightAttacks[64];
u64 kingAttacks[64];
u64 bishopAttacks[64][512];
u64 rookAttacks[64][4096];

// precomputed move lookup tables
void initKnightAttacks() {
    for (int i = 0; i < 64; i++) {
        // knight attacks from each of the 64 squares
        u64 knightboard = 0;
        if (i % 8 < 6 && i / 8 != 7) {setBit(knightboard, i+10);} // conditions are to make sure the knight does not go off the board
        if (i % 8 > 1 && i / 8 != 0) {setBit(knightboard, i-10);}
        if (i % 8 != 7 && i / 8 < 6) {setBit(knightboard, i+17);}
        if (i % 8 != 0 && i / 8 > 1) {setBit(knightboard, i-17);}
        if (i % 8 != 0 && i / 8 < 6) {setBit(knightboard, i+15);}
        if (i % 8 != 7 && i / 8 > 1) {setBit(knightboard, i-15);}
        if (i % 8 > 1 && i / 8 != 7) {setBit(knightboard, i+6);}
        if (i % 8 < 6 && i / 8 != 0) {setBit(knightboard, i-6);}

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
        u64 mask = isRook ? rookAttackMasks[square] : bishopAttackMasks[square];
        
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
	
	occupancy &= rookAttackMasks[square];
	occupancy *= rookMagics[square];
	occupancy >>= 64 - rookRelevantOccupancyAmount[square];
	
	return rookAttacks[square][occupancy];
}

u64 generateBishopMoves(int square, u64 occupancy) {
	
	occupancy &= bishopAttackMasks[square];
	occupancy *= bishopMagics[square];
	occupancy >>= 64 - bishopRelevantOccupancyAmount[square];
	
	return bishopAttacks[square][occupancy];
}

u64 generateRookAttacks(int rookIndex, u64 occupancy, u64 enemyPieces) {
    
    occupancy &= rookAttackMasks[rookIndex];
    occupancy *= rookMagics[rookIndex];
    occupancy >>= 64 - rookRelevantOccupancyAmount[rookIndex];

    return rookAttacks[rookIndex][occupancy] & enemyPieces;
}

u64 generateBishopAttacks(int bishopIndex, u64 occupancy, u64 enemyPieces) {
    
    occupancy &= bishopAttackMasks[bishopIndex];
    occupancy *= bishopMagics[bishopIndex];
    occupancy >>= 64 - bishopRelevantOccupancyAmount[bishopIndex];

    return bishopAttacks[bishopIndex][occupancy] & enemyPieces;
}
                        
int getMoves(bitboards_t *BB, move_t *MOVES, int *whiteAttacks, int *blackAttacks) {

    u64 epBit = BB->enPassantSquare < 0 ? 0 : bit(BB->enPassantSquare);

    // function to generate all possible moves for a given player and board state
    possibleCalls++;

    int count = 0;

    if (BB->color) {
        u64 step1 = (BB->bits[whitePawns] << 8) & ~BB->bits[allPieces];
        u64 step2 = ((step1 & whitePawnStartRank) << 8) & ~BB->bits[allPieces];

        while (step1) {
            u8 b = lsb(step1);
            flipBit(step1, b);
            if ((1ULL << b) & pawnPromotionMask) {
                MOVES[count++] = MOVE(b - 8, b, 4, 0, 0, 0);
                MOVES[count++] = MOVE(b - 8, b, 3, 0, 0, 0);
                MOVES[count++] = MOVE(b - 8, b, 2, 0, 0, 0);
                MOVES[count++] = MOVE(b - 8, b, 1, 0, 0, 0);
            } else {
                MOVES[count++] = MOVE(b - 8, b, 0, 0, 0, 0);
            }
        }

        while (step2) {
            u8 b = lsb(step2);
            flipBit(step2, b);
            MOVES[count++] = MOVE(b - 16, b, 0, 0, 0, 1);
        }

    } else {

        u64 step1 = (BB->bits[blackPawns] >> 8) & ~BB->bits[allPieces];
        u64 step2 = ((step1 & blackPawnStartRank) >> 8) & ~BB->bits[allPieces];

        while (step1) {
            u8 b = lsb(step1);
            flipBit(step1, b);
            if ((1ULL << b) & pawnPromotionMask) {
                MOVES[count++] = MOVE(b + 8, b, 4, 0, 0, 0);
                MOVES[count++] = MOVE(b + 8, b, 3, 0, 0, 0);
                MOVES[count++] = MOVE(b + 8, b, 2, 0, 0, 0);
                MOVES[count++] = MOVE(b + 8, b, 1, 0, 0, 0);
            } else {
                MOVES[count++] = MOVE(b + 8, b, 0, 0, 0, 0);
            }
        }

        while (step2) {
            u8 b = lsb(step2);
            flipBit(step2, b);
            MOVES[count++] = MOVE(b + 16, b, 0, 0, 0, 1);
        }
    }

    u64 capture1, capture2;

    capture1 = ((BB->bits[whitePawns] << 7) & ~leftmostFileMask);
    capture2 = ((BB->bits[whitePawns] << 9) & ~rightmostFileMask);

    while (capture1) {
        u8 b = lsb(capture1);
        flipBit(capture1, b);
        whiteAttacks[b]++;
        if (BB->color && (bit(b) & (BB->bits[blackPieces] | epBit))) {
            if (bit(b) == epBit) {
                MOVES[count++] = MOVE(b - 7, b, 0, 0, 1, 0);
            } else if ((1ULL << b) & pawnPromotionMask) {
                MOVES[count++] = MOVE(b - 7, b, 4, 0, 0, 0);
                MOVES[count++] = MOVE(b - 7, b, 3, 0, 0, 0);
                MOVES[count++] = MOVE(b - 7, b, 2, 0, 0, 0);
                MOVES[count++] = MOVE(b - 7, b, 1, 0, 0, 0);
            } else {
                MOVES[count++] = MOVE(b - 7, b, 0, 0, 0, 0);
            }
        }
    }

    while (capture2) {
        u8 b = lsb(capture2);
        flipBit(capture2, b);
        whiteAttacks[b]++;
        if (BB->color && (bit(b) & (BB->bits[blackPieces] | epBit))) {
            if (bit(b) == epBit) {
                MOVES[count++] = MOVE(b - 9, b, 0, 0, 1, 0);
            } else if ((1ULL << b) & pawnPromotionMask) {
                MOVES[count++] = MOVE(b - 9, b, 4, 0, 0, 0);
                MOVES[count++] = MOVE(b - 9, b, 3, 0, 0, 0);
                MOVES[count++] = MOVE(b - 9, b, 2, 0, 0, 0);
                MOVES[count++] = MOVE(b - 9, b, 1, 0, 0, 0);
            } else {
                MOVES[count++] = MOVE(b - 9, b, 0, 0, 0, 0);
            }
        }
    }

    capture1 = ((BB->bits[blackPawns] >> 7) & ~rightmostFileMask);
    capture2 = ((BB->bits[blackPawns] >> 9) & ~leftmostFileMask);

    while (capture1) {
        u8 b = lsb(capture1);
        flipBit(capture1, b);
        blackAttacks[b]++;
        if (!BB->color && (bit(b) & (BB->bits[whitePieces] | epBit))) {
            if (bit(b) == epBit) {
                MOVES[count++] = MOVE(b + 7, b, 0, 0, 1, 0);
            } else if ((1ULL << b) & pawnPromotionMask) {
                MOVES[count++] = MOVE(b + 7, b, 4, 0, 0, 0);
                MOVES[count++] = MOVE(b + 7, b, 3, 0, 0, 0);
                MOVES[count++] = MOVE(b + 7, b, 2, 0, 0, 0);
                MOVES[count++] = MOVE(b + 7, b, 1, 0, 0, 0);
            } else {
                MOVES[count++] = MOVE(b + 7, b, 0, 0, 0, 0);
            }
        }
    }

    while (capture2) {
        u8 b = lsb(capture2);
        flipBit(capture2, b);
        blackAttacks[b]++;
        if (!BB->color && (bit(b) & (BB->bits[whitePieces] | epBit))) {
            if (bit(b) == epBit) {
                MOVES[count++] = MOVE(b + 9, b, 0, 0, 1, 0);
            } else if ((1ULL << b) & pawnPromotionMask) {
                MOVES[count++] = MOVE(b + 9, b, 4, 0, 0, 0);
                MOVES[count++] = MOVE(b + 9, b, 3, 0, 0, 0);
                MOVES[count++] = MOVE(b + 9, b, 2, 0, 0, 0);
                MOVES[count++] = MOVE(b + 9, b, 1, 0, 0, 0);
            } else {
                MOVES[count++] = MOVE(b + 9, b, 0, 0, 0, 0);
            }
        }
    }

    u64 knightMoves;
    u64 knights = BB->bits[whiteKnights];

    while (knights) {
        u8 knightIndex = lsb(knights);
        flipBit(knights, knightIndex);
        knightMoves = knightAttacks[knightIndex];
        while (knightMoves) {
            u8 b = lsb(knightMoves);
            flipBit(knightMoves, b);
            whiteAttacks[b]++;
            if (BB->color && (bit(b) & ~BB->bits[whitePieces])) {
                MOVES[count++] = MOVE(knightIndex, b, 0, 0, 0, 0);
            }
        }
    }

    knights = BB->bits[blackKnights];

    while (knights) {
        u8 knightIndex = lsb(knights);
        flipBit(knights, knightIndex);
        knightMoves = knightAttacks[knightIndex];
        while (knightMoves) {
            u8 b = lsb(knightMoves);
            flipBit(knightMoves, b);
            blackAttacks[b]++;
            if (!BB->color && (bit(b) & ~BB->bits[blackPieces])) {
                MOVES[count++] = MOVE(knightIndex, b, 0, 0, 0, 0);
            }
        }
    }

    u64 bishopMoves;
    u64 bishops = BB->bits[whiteBishops];

    while (bishops) {
        u8 bishopIndex = lsb(bishops);
        flipBit(bishops, bishopIndex);
        bishopMoves = generateBishopMoves(bishopIndex, BB->bits[allPieces]);
        while (bishopMoves) {
            u8 b = lsb(bishopMoves);
            flipBit(bishopMoves, b);
            whiteAttacks[b]++;
            if (BB->color && (bit(b) & ~BB->bits[whitePieces])) {
                MOVES[count++] = MOVE(bishopIndex, b, 0, 0, 0, 0);
            }
        }
    }

    bishops = BB->bits[blackBishops];

    while (bishops) {
        u8 bishopIndex = lsb(bishops);
        flipBit(bishops, bishopIndex);
        bishopMoves = generateBishopMoves(bishopIndex, BB->bits[allPieces]);
        while (bishopMoves) {
            u8 b = lsb(bishopMoves);
            flipBit(bishopMoves, b);
            blackAttacks[b]++;
            if (!BB->color && (bit(b) & ~BB->bits[blackPieces])) {
                MOVES[count++] = MOVE(bishopIndex, b, 0, 0, 0, 0);
            }
        }
    }

    u64 kingMoves;
    u8 kingIndex;

    u64 king = BB->bits[whiteKing];

    if (king) {
        kingIndex = lsb(king);
        kingMoves = kingAttacks[kingIndex];
    } else {
        // without the king some of the other functions will break
        printf("\n");
        printBoard(BB);
        printf("\nError: king not found\n");
        exit(1);
    }

    while (kingMoves) {
        // loop through all MOVES king move bits and add them to the move list
        u8 b = lsb(kingMoves);
        flipBit(kingMoves, b);
        whiteAttacks[b]++;
        if (BB->color && (bit(b) & ~BB->bits[whitePieces])) { // exclude own pieces
            MOVES[count++] = MOVE(kingIndex, b, 0, 0, 0, 0);
        }
    }

    // castling is only available when the flags are set and the squares are empty, check detection is done in the search tree
    if (BB->color) {
        bool castleKing = BB->whiteCastleKingSide;
        bool castleQueen = BB->whiteCastleQueenSide;

        if (castleKing && ((!(checkBit(BB->bits[allPieces], kingIndex-1))) && (!(checkBit(BB->bits[allPieces], kingIndex-2))) && (checkBit(BB->bits[whiteRooks], kingIndex-3)))) {

            MOVES[count++] = MOVE(kingIndex, kingIndex-2, 0, KINGSIDE, 0, 0);
        }

        if (castleQueen && (!(checkBit(BB->bits[allPieces], kingIndex+1)) && (!(checkBit(BB->bits[allPieces], kingIndex+2))) && (!(checkBit(BB->bits[allPieces], kingIndex+3))) && (checkBit(BB->bits[whiteRooks], kingIndex+4)))) {

            MOVES[count++] = MOVE(kingIndex, kingIndex+2, 0, QUEENSIDE, 0, 0);
        }
    }

    king = BB->bits[blackKing];

    if (king) {
        kingIndex = lsb(king);
        kingMoves = kingAttacks[kingIndex];
    } else {
        printf("\n");
        printBoard(BB);
        printf("\nError: king not found\n");
        exit(1);
    }

    while (kingMoves) {
        u8 b = lsb(kingMoves);
        flipBit(kingMoves, b);
        blackAttacks[b]++;
        if (!BB->color && (bit(b) & ~BB->bits[blackPieces])) {
            MOVES[count++] = MOVE(kingIndex, b, 0, 0, 0, 0);
        }
    }

    if (!BB->color) {
        bool castleKing = BB->blackCastleKingSide;
        bool castleQueen = BB->blackCastleQueenSide;

        if (castleKing && ((!(checkBit(BB->bits[allPieces], kingIndex-1))) && (!(checkBit(BB->bits[allPieces], kingIndex-2))) && (checkBit(BB->bits[blackRooks], kingIndex-3)))) {

            MOVES[count++] = MOVE(kingIndex, kingIndex-2, 0, KINGSIDE, 0, 0);
        }

        if (castleQueen && (!(checkBit(BB->bits[allPieces], kingIndex+1)) && (!(checkBit(BB->bits[allPieces], kingIndex+2))) && (!(checkBit(BB->bits[allPieces], kingIndex+3))) && (checkBit(BB->bits[blackRooks], kingIndex+4)))) {

            MOVES[count++] = MOVE(kingIndex, kingIndex+2, 0, QUEENSIDE, 0, 0);
        }
    } 

    u64 rookMoves;
    u64 rooks = BB->bits[whiteRooks];

    while (rooks) {
        u8 rookIndex = lsb(rooks);
        flipBit(rooks, rookIndex);
        rookMoves = generateRookMoves(rookIndex, BB->bits[allPieces]);
        while (rookMoves) {
            u8 b = lsb(rookMoves);
            flipBit(rookMoves, b);
            whiteAttacks[b]++;
            if (BB->color && (bit(b) & ~BB->bits[whitePieces])) {
                MOVES[count++] = MOVE(rookIndex, b, 0, 0, 0, 0);
            }
        }
    }

    rooks = BB->bits[blackRooks];

    while (rooks) {
        u8 rookIndex = lsb(rooks);
        flipBit(rooks, rookIndex);
        rookMoves = generateRookMoves(rookIndex, BB->bits[allPieces]);
        while (rookMoves) {
            u8 b = lsb(rookMoves);
            flipBit(rookMoves, b);
            blackAttacks[b]++;
            if (!BB->color && (bit(b) & ~BB->bits[blackPieces])) {
                MOVES[count++] = MOVE(rookIndex, b, 0, 0, 0, 0);
            }
        }
    }

    u64 queenMoves;
    u64 queens = BB->bits[whiteQueens];
    // queens are a combination of rooks and bishops
    while (queens) {
        int queenIndex = lsb(queens);
        flipBit(queens, queenIndex);
        queenMoves = (generateBishopMoves(queenIndex, BB->bits[allPieces])) | (generateRookMoves(queenIndex, BB->bits[allPieces]));
        while (queenMoves) {
            u8 b = lsb(queenMoves);
            flipBit(queenMoves, b);
            whiteAttacks[b]++;
            if (BB->color && (bit(b) & ~BB->bits[whitePieces])) {
                MOVES[count++] = MOVE(queenIndex, b, 0, 0, 0, 0);
            }
        }
    }

    queens = BB->bits[blackQueens];

    while (queens) {
        int queenIndex = lsb(queens);
        flipBit(queens, queenIndex);
        queenMoves = (generateBishopMoves(queenIndex, BB->bits[allPieces])) | (generateRookMoves(queenIndex, BB->bits[allPieces]));
        while (queenMoves) {
            u8 b = lsb(queenMoves);
            flipBit(queenMoves, b);
            blackAttacks[b]++;
            if (!BB->color && (bit(b) & ~BB->bits[blackPieces])) {
                MOVES[count++] = MOVE(queenIndex, b, 0, 0, 0, 0);
            }
        }
    }

    return count;
}

int getAttacks(bitboards_t *BB, move_t *CAPTURES, int *whiteAttacks, int *blackAttacks) {
    // this function is used to generate capture moves for the quiescence search and get the number of attacks on each square for both colors
    // it is also used to evaluate the king safety

    /*
        whiteAttacks for startpos:

        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        2, 2, 3, 2, 2, 3, 2, 2,
        1, 1, 1, 4, 4, 1, 1, 1,
        0, 1, 1, 1, 1, 1, 1, 0

        and for a dangerous position for black:

        r4r2/ppp1qpk1/1b1p1n2/n2Pp1B1/3PN3/2P2Q2/PP3PPP/R4RK1 b - - 2 16

        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, k, 0,
        0, 0, 1, 0, 1, 3, 0, 1,
        0, 0, 2, 0, 1, 1, 0, 1,
        0, 1, 0, 1, 1, 2, 1, 1,
        1, 1, 3, 1, 3, 1, 4, 2,
        1, 0, 0, 0, 1, 4, 2, 1,
        1, 2, 3, 3, 2, 2, 1, 1

        here the danger level is:

        0  0  0  0  1  1  1  1         0  0  0  0  0  0  0  0
        0  0  0  0  1  1  1  1         0  0  0  0  0  0  0  0
        0  0  0  0  1  1  1  1         0  0  0  0  1  3  0  1
        0  0  0  0  0  1  1  1   -->   0  0  0  0  0  1  0  1   =   7 + king ring = 11
        0  0  0  0  0  0  0  0         0  0  0  0  0  0  0  0
        0  0  0  0  0  0  0  0         0  0  0  0  0  0  0  0
        0  0  0  0  0  0  0  0         0  0  0  0  0  0  0  0
        0  0  0  0  0  0  0  0         0  0  0  0  0  0  0  0

        attacks int the king ring will get extra weight

    */

    u64 epSquare = BB->enPassantSquare < 0 ? 0 : bit(BB->enPassantSquare);

    int count = 0;

    u64 wPawns = BB->bits[whitePawns];
    u64 capture1, capture2;

    capture1 = ((wPawns << 7) & ~leftmostFileMask);
    capture2 = ((wPawns << 9) & ~rightmostFileMask);

    while (capture1) {
        u8 b = lsb(capture1);
        flipBit(capture1, b);
        whiteAttacks[b]++;
        if (BB->color && ((1ULL << b) & (BB->bits[blackPieces] | epSquare))) {
            if (bit(b) == epSquare) {
                CAPTURES[count++] = MOVE(b - 7, b, 0, 0, 1, 0);
            } else if ((1ULL << b) & pawnPromotionMask) {
                CAPTURES[count++] = MOVE(b - 7, b, 4, 0, 0, 0);
                CAPTURES[count++] = MOVE(b - 7, b, 3, 0, 0, 0);
                CAPTURES[count++] = MOVE(b - 7, b, 2, 0, 0, 0);
                CAPTURES[count++] = MOVE(b - 7, b, 1, 0, 0, 0);
            } else {
                CAPTURES[count++] = MOVE(b - 7, b, 0, 0, 0, 0);
            }
        }
    }

    while (capture2) {
        u8 b = lsb(capture2);
        flipBit(capture2, b);
        whiteAttacks[b]++;
        if (BB->color && ((1ULL << b) & (BB->bits[blackPieces] | epSquare))) {
            if (bit(b) == epSquare) {
                CAPTURES[count++] = MOVE(b - 9, b, 0, 0, 1, 0);
            } else if ((1ULL << b) & pawnPromotionMask) {
                CAPTURES[count++] = MOVE(b - 9, b, 4, 0, 0, 0);
                CAPTURES[count++] = MOVE(b - 9, b, 3, 0, 0, 0);
                CAPTURES[count++] = MOVE(b - 9, b, 2, 0, 0, 0);
                CAPTURES[count++] = MOVE(b - 9, b, 1, 0, 0, 0);
            } else {
                CAPTURES[count++] = MOVE(b - 9, b, 0, 0, 0, 0);
            }
        }
    }

    u64 bPawns = BB->bits[blackPawns];

    capture1 = ((bPawns >> 7) & ~rightmostFileMask);
    capture2 = ((bPawns >> 9) & ~leftmostFileMask);
    
    while (capture1) {
        u8 b = lsb(capture1);
        flipBit(capture1, b);
        blackAttacks[b]++;
        if (!BB->color && ((1ULL << b) & (BB->bits[whitePieces] | epSquare))) {
            if (bit(b) == epSquare) {
                CAPTURES[count++] = MOVE(b + 7, b, 0, 0, 1, 0);
            } else if ((1ULL << b) & pawnPromotionMask) {
                CAPTURES[count++] = MOVE(b + 7, b, 4, 0, 0, 0);
                CAPTURES[count++] = MOVE(b + 7, b, 3, 0, 0, 0);
                CAPTURES[count++] = MOVE(b + 7, b, 2, 0, 0, 0);
                CAPTURES[count++] = MOVE(b + 7, b, 1, 0, 0, 0);
            } else {
                CAPTURES[count++] = MOVE(b + 7, b, 0, 0, 0, 0);
            }
        }
    }

    while (capture2) {
        u8 b = lsb(capture2);
        flipBit(capture2, b);
        blackAttacks[b]++;
        if (!BB->color && ((1ULL << b) & (BB->bits[whitePieces] | epSquare))) {
            if (bit(b) == epSquare) {
                CAPTURES[count++] = MOVE(b + 9, b, 0, 0, 1, 0);
            } else if ((1ULL << b) & pawnPromotionMask) {
                CAPTURES[count++] = MOVE(b + 9, b, 4, 0, 0, 0);
                CAPTURES[count++] = MOVE(b + 9, b, 3, 0, 0, 0);
                CAPTURES[count++] = MOVE(b + 9, b, 2, 0, 0, 0);
                CAPTURES[count++] = MOVE(b + 9, b, 1, 0, 0, 0);
            } else {
                CAPTURES[count++] = MOVE(b + 9, b, 0, 0, 0, 0);
            }
        }
    }

    u64 wKnights = BB->bits[whiteKnights];
    u64 knightAtkWhite;

    while (wKnights) {
        u8 knightIndex = lsb(wKnights);
        flipBit(wKnights, knightIndex);
        knightAtkWhite = knightAttacks[knightIndex];
        while (knightAtkWhite) {
            u8 b = lsb(knightAtkWhite);
            flipBit(knightAtkWhite, b);
            whiteAttacks[b]++;
            if (BB->color && ((1ULL << b) & BB->bits[blackPieces])) {
                CAPTURES[count++] = MOVE(knightIndex, b, 0, 0, 0, 0);
            }
        }
    }

    u64 bKnights = BB->bits[blackKnights];
    u64 knightAtkBlack;

    while (bKnights) {
        u8 knightIndex = lsb(bKnights);
        flipBit(bKnights, knightIndex);
        knightAtkBlack = knightAttacks[knightIndex];
        while (knightAtkBlack) {
            u8 b = lsb(knightAtkBlack);
            flipBit(knightAtkBlack, b);
            blackAttacks[b]++;
            if (!BB->color && ((1ULL << b) & BB->bits[whitePieces])) {
                CAPTURES[count++] = MOVE(knightIndex, b, 0, 0, 0, 0);
            }
        }
    }
    
    u64 wBishops = BB->bits[whiteBishops];
    u64 bishopAtkWhite;

    while (wBishops) {
        u8 bishopIndex = lsb(wBishops);
        flipBit(wBishops, bishopIndex);
        bishopAtkWhite = generateBishopMoves(bishopIndex, BB->bits[allPieces]);
        while (bishopAtkWhite) {
            u8 b = lsb(bishopAtkWhite);
            flipBit(bishopAtkWhite, b);
            whiteAttacks[b]++;
            if (BB->color && ((1ULL << b) & BB->bits[blackPieces])) {
                CAPTURES[count++] = MOVE(bishopIndex, b, 0, 0, 0, 0);
            }
        }
    }

    u64 bBishops = BB->bits[blackBishops];
    u64 bishopAtkBlack;

    while (bBishops) {
        u8 bishopIndex = lsb(bBishops);
        flipBit(bBishops, bishopIndex);
        bishopAtkBlack = generateBishopMoves(bishopIndex, BB->bits[allPieces]);
        while (bishopAtkBlack) {
            u8 b = lsb(bishopAtkBlack);
            flipBit(bishopAtkBlack, b);
            blackAttacks[b]++;
            if (!BB->color && ((1ULL << b) & BB->bits[whitePieces])) {
                CAPTURES[count++] = MOVE(bishopIndex, b, 0, 0, 0, 0);
            }
        }
    }

    u64 wKing = BB->bits[whiteKing];
    u64 kingAtkWhite;
    u8 kingIndex;

    if (wKing) {
        kingIndex = lsb(wKing);
        kingAtkWhite = kingAttacks[kingIndex];
    } else {
        printf("\n");
        printBoard(BB);
        printf("\nError: king not found\n");
        exit(1);
    }

    while (kingAtkWhite) {
        u8 b = lsb(kingAtkWhite);
        flipBit(kingAtkWhite, b);
        whiteAttacks[b]++;
        if (BB->color && ((1ULL << b) & BB->bits[blackPieces])) {
            CAPTURES[count++] = MOVE(kingIndex, b, 0, 0, 0, 0);
        }
    }

    u64 bKing = BB->bits[blackKing];
    u64 kingAtkBlack;

    if (bKing) {
        kingIndex = lsb(bKing);
        kingAtkBlack = kingAttacks[kingIndex];
    } else {
        printf("\n");
        printBoard(BB);
        printf("\nError: king not found\n");
        exit(1);
    }

    while (kingAtkBlack) {
        u8 b = lsb(kingAtkBlack);
        flipBit(kingAtkBlack, b);
        blackAttacks[b]++;
        if (!BB->color && ((1ULL << b) & BB->bits[whitePieces])) {
            CAPTURES[count++] = MOVE(kingIndex, b, 0, 0, 0, 0);
        }
    }

    u64 wRooks = BB->bits[whiteRooks];
    u64 rookAtkWhite;

    while (wRooks) {
        u8 rookIndex = lsb(wRooks);
        flipBit(wRooks, rookIndex);
        rookAtkWhite = generateRookMoves(rookIndex, BB->bits[allPieces]);
        while (rookAtkWhite) {
            u8 b = lsb(rookAtkWhite);
            flipBit(rookAtkWhite, b);
            whiteAttacks[b]++;
            if (BB->color && ((1ULL << b) & BB->bits[blackPieces])) {
                CAPTURES[count++] = MOVE(rookIndex, b, 0, 0, 0, 0);
            }
        }
    }

    u64 bRooks = BB->bits[blackRooks];
    u64 rookAtkBlack;

    while (bRooks) {
        u8 rookIndex = lsb(bRooks);
        flipBit(bRooks, rookIndex);
        rookAtkBlack = generateRookMoves(rookIndex, BB->bits[allPieces]);
        while (rookAtkBlack) {
            u8 b = lsb(rookAtkBlack);
            flipBit(rookAtkBlack, b);
            blackAttacks[b]++;
            if (!BB->color && ((1ULL << b) & BB->bits[whitePieces])) {
                CAPTURES[count++] = MOVE(rookIndex, b, 0, 0, 0, 0);
            }
        }
    }

    u64 wQueens = BB->bits[whiteQueens];
    u64 queenAtkWhite;

    while (wQueens) {
        u8 queenIndex = lsb(wQueens);
        flipBit(wQueens, queenIndex);
        queenAtkWhite = (generateBishopMoves(queenIndex, BB->bits[allPieces])) | (generateRookMoves(queenIndex, BB->bits[allPieces]));
        while (queenAtkWhite) {
            u8 b = lsb(queenAtkWhite);
            flipBit(queenAtkWhite, b);
            whiteAttacks[b]++;
            if (BB->color && ((1ULL << b) & BB->bits[blackPieces])) {
                CAPTURES[count++] = MOVE(queenIndex, b, 0, 0, 0, 0);
            }
        }
    }

    u64 bQueens = BB->bits[blackQueens];
    u64 queenAtkBlack;

    while (bQueens) {
        u8 queenIndex = lsb(bQueens);
        flipBit(bQueens, queenIndex);
        queenAtkBlack = (generateBishopMoves(queenIndex, BB->bits[allPieces])) | (generateRookMoves(queenIndex, BB->bits[allPieces]));
        while (queenAtkBlack) {
            u8 b = lsb(queenAtkBlack);
            flipBit(queenAtkBlack, b);
            blackAttacks[b]++;
            if (!BB->color && ((1ULL << b) & BB->bits[whitePieces])) {
                CAPTURES[count++] = MOVE(queenIndex, b, 0, 0, 0, 0);
            }
        }
    }

    // printBoard(BB);
    // printf("White Attacks:\n");
    // for (int i = 63; i >= 0; i--) {
    //     printf("%d ", whiteAttacks[i]);
    //     if (i % 8 == 0) printf("\n");
    // }
    // printf("Black Attacks:\n");
    // for (int i = 63; i >= 0; i--) {
    //     printf("%d ", blackAttacks[i]);
    //     if (i % 8 == 0) printf("\n");
    // }
    // printf("Count: %d\n", count);
    // printf("Captures:\n");
    // for (int i = 0; i < count; i++) {
    //     printf("%s%s\n", notation[mFrom(CAPTURES[i])], notation[mTo(CAPTURES[i])]);
    // }
    // exit(0);
    return count;
}