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
                        
int possiblemoves(
        bool isWhiteToMove,
        u64 occupied, int epSquare, u64 myPieces, u64 enemyPieces, 
        u64 pawns, u64 knights, u64 bishops, u64 rooks, u64 queens, u64 king, 
        bool castleQueen, bool castleKing, move_t *MOVES
    ) {

    u64 epBit = epSquare < 0 ? 0 : bit(epSquare);

    // function to generate all possible moves for a given player and board state
    possibleCalls++;

    int count = 0;

    u64 step1, step2, capture1, capture2;
    int sign;

    if (isWhiteToMove) {
        step1 = (pawns << 8) & ~occupied;
        step2 = ((step1 & whitePawnStartRank) << 8) & ~occupied;
        capture1 = ((pawns << 7) & ~leftmostFileMask) & (enemyPieces | epBit);
        capture2 = ((pawns << 9) & ~rightmostFileMask) & (enemyPieces | epBit);
        sign = -1;

    } else {
        step1 = (pawns >> 8) & ~occupied;
        step2 = ((step1 & blackPawnStartRank) >> 8) & ~occupied;
        capture1 = ((pawns >> 7) & ~rightmostFileMask) & (enemyPieces | epBit);
        capture2 = ((pawns >> 9) & ~leftmostFileMask) & (enemyPieces | epBit);
        sign = 1;
    }
        
    while (step1) {
        u8 b = lsb(step1);
        flipBit(step1, b);
        if ((1ULL << b) & pawnPromotionMask) {
            MOVES[count++] = MOVE(b + (8 * sign), b, 4, 0, 0, 0);
            MOVES[count++] = MOVE(b + (8 * sign), b, 3, 0, 0, 0);
            MOVES[count++] = MOVE(b + (8 * sign), b, 2, 0, 0, 0);
            MOVES[count++] = MOVE(b + (8 * sign), b, 1, 0, 0, 0);
        } else {
            MOVES[count++] = MOVE(b + (8 * sign), b, 0, 0, 0, 0);
        }
    }

    while (step2) {
        u8 b = lsb(step2);
        flipBit(step2, b);
        MOVES[count++] = MOVE(b + (16 * sign), b, 0, 0, 0, 1);
    }

    while (capture1) {
        u8 b = lsb(capture1);
        flipBit(capture1, b);
        if (bit(b) == epBit) {
            MOVES[count++] = MOVE(b + (7 * sign), b, 0, 0, 1, 0);
        } else if ((1ULL << b) & pawnPromotionMask) {
            MOVES[count++] = MOVE(b + (7 * sign), b, 4, 0, 0, 0);
            MOVES[count++] = MOVE(b + (7 * sign), b, 3, 0, 0, 0);
            MOVES[count++] = MOVE(b + (7 * sign), b, 2, 0, 0, 0);
            MOVES[count++] = MOVE(b + (7 * sign), b, 1, 0, 0, 0);
        } else {
            MOVES[count++] = MOVE(b + (7 * sign), b, 0, 0, 0, 0);
        }
    }

    while (capture2) {
        u8 b = lsb(capture2);
        flipBit(capture2, b);
        if (bit(b) == epBit) {
            MOVES[count++] = MOVE(b + (9 * sign), b, 0, 0, 1, 0);
        } else if ((1ULL << b) & pawnPromotionMask) {
            MOVES[count++] = MOVE(b + (9 * sign), b, 4, 0, 0, 0);
            MOVES[count++] = MOVE(b + (9 * sign), b, 3, 0, 0, 0);
            MOVES[count++] = MOVE(b + (9 * sign), b, 2, 0, 0, 0);
            MOVES[count++] = MOVE(b + (9 * sign), b, 1, 0, 0, 0);
        } else {
            MOVES[count++] = MOVE(b + (9 * sign), b, 0, 0, 0, 0);
        }
    }

    u64 knightMoves;

    while (knights) {
        // for each knight on the board, generate all its MOVES moves
        u8 knightIndex = lsb(knights);
        flipBit(knights, knightIndex);
        knightMoves = knightAttacks[knightIndex] & ~myPieces;
        while (knightMoves) {
            // loop through all MOVES knight move bits and add them to the move list
            u8 b = lsb(knightMoves);
            flipBit(knightMoves, b);
            MOVES[count++] = MOVE(knightIndex, b, 0, 0, 0, 0);
        }
    }

    u64 bishopMoves;

    while (bishops) {
        u8 bishopIndex = lsb(bishops);
        flipBit(bishops, bishopIndex);
        bishopMoves = generateBishopMoves(bishopIndex, occupied) & ~myPieces;
        while (bishopMoves) {
            u8 b = lsb(bishopMoves);
            flipBit(bishopMoves, b);
            MOVES[count++] = MOVE(bishopIndex, b, 0, 0, 0, 0);
        }
    }

    u64 kingMoves;
    u8 kingIndex = 0;

    if (king) {
        kingIndex = lsb(king);
        kingMoves = kingAttacks[kingIndex] & ~myPieces; // exclude own pieces
    } else {
        // without the king some of the other functions will break
        printf("\n");
        displayBoard(occupied);
        printf("\nError: king not found\n");
        exit(1);
    }

    while (kingMoves) {
        // loop through all MOVES king move bits and add them to the move list
        u8 b = lsb(kingMoves);
        flipBit(kingMoves, b);
        MOVES[count++] = MOVE(kingIndex, b, 0, 0, 0, 0);
    }
    // castling is only available when the flags are set and the squares are empty, check detection is done in the search tree
    if (castleKing && ((!(checkBit(occupied, kingIndex-1))) && (!(checkBit(occupied, kingIndex-2))) && (checkBit(rooks, kingIndex-3)))) {

        MOVES[count++] = MOVE(kingIndex, kingIndex-2, 0, KINGSIDE, 0, 0);
    }

    if (castleQueen && (!(checkBit(occupied, kingIndex+1)) && (!(checkBit(occupied, kingIndex+2))) && (!(checkBit(occupied, kingIndex+3))) && (checkBit(rooks, kingIndex+4)))) {

        MOVES[count++] = MOVE(kingIndex, kingIndex+2, 0, QUEENSIDE, 0, 0);
    }

    u64 rookMoves;

    while (rooks) {
        u8 rookIndex = lsb(rooks);
        flipBit(rooks, rookIndex);
        rookMoves = generateRookMoves(rookIndex, occupied) & ~myPieces;
        while (rookMoves) {
            u8 b = lsb(rookMoves);
            flipBit(rookMoves, b);
            MOVES[count++] = MOVE(rookIndex, b, 0, 0, 0, 0);
        }
    }

    u64 queenMoves;
    // queens are a combination of rooks and bishops
    while (queens) {
        int queenIndex = lsb(queens);
        flipBit(queens, queenIndex);
        queenMoves = (generateBishopMoves(queenIndex, occupied) & ~myPieces) | (generateRookMoves(queenIndex, occupied) & ~myPieces);
        while (queenMoves) {
            u8 b = lsb(queenMoves);
            flipBit(queenMoves, b);
            MOVES[count++] = MOVE(queenIndex, b, 0, 0, 0, 0);
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
    u64 allPieces = BB->allPieces;

    u64 whitePawns = BB->whitePawns;
    u64 capture1, capture2;

    capture1 = ((whitePawns << 7) & ~leftmostFileMask);
    capture2 = ((whitePawns << 9) & ~rightmostFileMask);

    while (capture1) {
        u8 b = lsb(capture1);
        flipBit(capture1, b);
        whiteAttacks[b]++;
        if (BB->color && ((1ULL << b) & (BB->blackPieces | epSquare))) {
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
        if (BB->color && ((1ULL << b) & (BB->blackPieces | epSquare))) {
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

    u64 blackPawns = BB->blackPawns;

    capture1 = ((blackPawns >> 7) & ~rightmostFileMask);
    capture2 = ((blackPawns >> 9) & ~leftmostFileMask);
    
    while (capture1) {
        u8 b = lsb(capture1);
        flipBit(capture1, b);
        blackAttacks[b]++;
        if (!BB->color && ((1ULL << b) & (BB->whitePieces | epSquare))) {
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
        if (!BB->color && ((1ULL << b) & (BB->whitePieces | epSquare))) {
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

    u64 whiteKnights = BB->whiteKnights;
    u64 knightAtkWhite;

    while (whiteKnights) {
        u8 knightIndex = lsb(whiteKnights);
        flipBit(whiteKnights, knightIndex);
        knightAtkWhite = knightAttacks[knightIndex];
        while (knightAtkWhite) {
            u8 b = lsb(knightAtkWhite);
            flipBit(knightAtkWhite, b);
            whiteAttacks[b]++;
            if (BB->color && ((1ULL << b) & BB->blackPieces)) {
                CAPTURES[count++] = MOVE(knightIndex, b, 0, 0, 0, 0);
            }
        }
    }

    u64 blackKnights = BB->blackKnights;
    u64 knightAtkBlack;

    while (blackKnights) {
        u8 knightIndex = lsb(blackKnights);
        flipBit(blackKnights, knightIndex);
        knightAtkBlack = knightAttacks[knightIndex];
        while (knightAtkBlack) {
            u8 b = lsb(knightAtkBlack);
            flipBit(knightAtkBlack, b);
            blackAttacks[b]++;
            if (!BB->color && ((1ULL << b) & BB->whitePieces)) {
                CAPTURES[count++] = MOVE(knightIndex, b, 0, 0, 0, 0);
            }
        }
    }
    
    u64 whiteBishops = BB->whiteBishops;
    u64 bishopAtkWhite;

    while (whiteBishops) {
        u8 bishopIndex = lsb(whiteBishops);
        flipBit(whiteBishops, bishopIndex);
        bishopAtkWhite = generateBishopMoves(bishopIndex, allPieces);
        while (bishopAtkWhite) {
            u8 b = lsb(bishopAtkWhite);
            flipBit(bishopAtkWhite, b);
            whiteAttacks[b]++;
            if (BB->color && ((1ULL << b) & BB->blackPieces)) {
                CAPTURES[count++] = MOVE(bishopIndex, b, 0, 0, 0, 0);
            }
        }
    }

    u64 blackBishops = BB->blackBishops;
    u64 bishopAtkBlack;

    while (blackBishops) {
        u8 bishopIndex = lsb(blackBishops);
        flipBit(blackBishops, bishopIndex);
        bishopAtkBlack = generateBishopMoves(bishopIndex, allPieces);
        while (bishopAtkBlack) {
            u8 b = lsb(bishopAtkBlack);
            flipBit(bishopAtkBlack, b);
            blackAttacks[b]++;
            if (!BB->color && ((1ULL << b) & BB->whitePieces)) {
                CAPTURES[count++] = MOVE(bishopIndex, b, 0, 0, 0, 0);
            }
        }
    }

    u64 whiteKing = BB->whiteKing;
    u64 kingAtkWhite;
    u8 kingIndex;

    if (whiteKing) {
        kingIndex = lsb(whiteKing);
        kingAtkWhite = kingAttacks[kingIndex];
    } else {
        printf("\n");
        displayBoard(allPieces);
        printf("\nError: king not found\n");
        exit(1);
    }

    while (kingAtkWhite) {
        u8 b = lsb(kingAtkWhite);
        flipBit(kingAtkWhite, b);
        whiteAttacks[b]++;
        if (BB->color && ((1ULL << b) & BB->blackPieces)) {
            CAPTURES[count++] = MOVE(kingIndex, b, 0, 0, 0, 0);
        }
    }

    u64 blackKing = BB->blackKing;
    u64 kingAtkBlack;

    if (blackKing) {
        kingIndex = lsb(blackKing);
        kingAtkBlack = kingAttacks[kingIndex];
    } else {
        printf("\n");
        displayBoard(allPieces);
        printf("\nError: king not found\n");
        exit(1);
    }

    while (kingAtkBlack) {
        u8 b = lsb(kingAtkBlack);
        flipBit(kingAtkBlack, b);
        blackAttacks[b]++;
        if (!BB->color && ((1ULL << b) & BB->whitePieces)) {
            CAPTURES[count++] = MOVE(kingIndex, b, 0, 0, 0, 0);
        }
    }

    u64 whiteRooks = BB->whiteRooks;
    u64 rookAtkWhite;

    while (whiteRooks) {
        u8 rookIndex = lsb(whiteRooks);
        flipBit(whiteRooks, rookIndex);
        rookAtkWhite = generateRookMoves(rookIndex, allPieces);
        while (rookAtkWhite) {
            u8 b = lsb(rookAtkWhite);
            flipBit(rookAtkWhite, b);
            whiteAttacks[b]++;
            if (BB->color && ((1ULL << b) & BB->blackPieces)) {
                CAPTURES[count++] = MOVE(rookIndex, b, 0, 0, 0, 0);
            }
        }
    }

    u64 blackRooks = BB->blackRooks;
    u64 rookAtkBlack;

    while (blackRooks) {
        u8 rookIndex = lsb(blackRooks);
        flipBit(blackRooks, rookIndex);
        rookAtkBlack = generateRookMoves(rookIndex, allPieces);
        while (rookAtkBlack) {
            u8 b = lsb(rookAtkBlack);
            flipBit(rookAtkBlack, b);
            blackAttacks[b]++;
            if (!BB->color && ((1ULL << b) & BB->whitePieces)) {
                CAPTURES[count++] = MOVE(rookIndex, b, 0, 0, 0, 0);
            }
        }
    }

    u64 whiteQueens = BB->whiteQueens;
    u64 queenAtkWhite;

    while (whiteQueens) {
        u8 queenIndex = lsb(whiteQueens);
        flipBit(whiteQueens, queenIndex);
        queenAtkWhite = (generateBishopMoves(queenIndex, allPieces)) | (generateRookMoves(queenIndex, allPieces));
        while (queenAtkWhite) {
            u8 b = lsb(queenAtkWhite);
            flipBit(queenAtkWhite, b);
            whiteAttacks[b]++;
            if (BB->color && ((1ULL << b) & BB->blackPieces)) {
                CAPTURES[count++] = MOVE(queenIndex, b, 0, 0, 0, 0);
            }
        }
    }

    u64 blackQueens = BB->blackQueens;
    u64 queenAtkBlack;

    while (blackQueens) {
        u8 queenIndex = lsb(blackQueens);
        flipBit(blackQueens, queenIndex);
        queenAtkBlack = (generateBishopMoves(queenIndex, allPieces)) | (generateRookMoves(queenIndex, allPieces));
        while (queenAtkBlack) {
            u8 b = lsb(queenAtkBlack);
            flipBit(queenAtkBlack, b);
            blackAttacks[b]++;
            if (!BB->color && ((1ULL << b) & BB->whitePieces)) {
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