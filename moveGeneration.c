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
        u64 occupied, u64 epSquare, u64 myPieces, u64 enemyPieces, 
        u64 pawns, u64 knights, u64 bishops, u64 rooks, u64 queens, u64 king, 
        bool castleQueen, bool castleKing, move_t *MOVES
    ) {

    // function to generate all possible moves for a given player and board state
    possibleCalls++;

    int count = 0;

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
        struct move_t m = {kingIndex, b, 5, 0, 0, 0, 0};
        MOVES[count++] = m;
    }
    // castling is only available when the flags are set and the squares are empty, check detection is done in the search tree
    if (castleKing && ((!(checkBit(occupied, kingIndex-1))) && (!(checkBit(occupied, kingIndex-2))) && (checkBit(rooks, kingIndex-3)))) {
        struct move_t m = {kingIndex, kingIndex-2, 5, KINGSIDE, 0, 0, 0};
        MOVES[count++] = m;
    }
    if (castleQueen && (!(checkBit(occupied, kingIndex+1)) && (!(checkBit(occupied, kingIndex+2))) && (!(checkBit(occupied, kingIndex+3))) && (checkBit(rooks, kingIndex+4)))) {
        struct move_t m = {kingIndex, kingIndex+2, 5, QUEENSIDE, 0, 0, 0};
        MOVES[count++] = m;
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
            struct move_t m = {knightIndex, b, 1, 0, 0, 0, 0};
            MOVES[count++] = m;
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
            struct move_t m = {bishopIndex, b, 2, 0, 0, 0, 0};
            MOVES[count++] = m;
        }
    }

    u64 rookMoves;

    while (rooks) {
        u8 rookIndex = lsb(rooks);
        flipBit(rooks, rookIndex);
        rookMoves = generateRookMoves(rookIndex, occupied) & ~myPieces;
        while (rookMoves) {
            u8 b = lsb(rookMoves);
            flipBit(rookMoves, b);
            struct move_t m = {rookIndex, b, 3, 0, 0, 0, 0};
            MOVES[count++] = m;
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
            struct move_t m = {queenIndex, b, 4, 0, 0, 0, 0};
            MOVES[count++] = m;
        }
    }

    u64 step1, step2, capture1, capture2;
    int sign;

    if (isWhiteToMove) {
        step1 = (pawns << 8) & ~occupied;
        step2 = ((step1 & whitePawnStartRank) << 8) & ~occupied;
        capture1 = ((pawns << 7) & ~leftmostFileMask) & (enemyPieces | epSquare);
        capture2 = ((pawns << 9) & ~rightmostFileMask) & (enemyPieces | epSquare);
        sign = -1;

    } else {
        step1 = (pawns >> 8) & ~occupied;
        step2 = ((step1 & blackPawnStartRank) >> 8) & ~occupied;
        capture1 = ((pawns >> 7) & ~rightmostFileMask) & (enemyPieces | epSquare);
        capture2 = ((pawns >> 9) & ~leftmostFileMask) & (enemyPieces | epSquare);
        sign = 1;
    }
        
    while (step1) {
        u8 b = lsb(step1);
        flipBit(step1, b);
        if ((1ULL << b) & pawnPromotionMask) {
            struct move_t m = {b + (8 * sign), b, 0, 0, 0, 0, 4};
            MOVES[count++] = m;
            m.promotesTo--;
            MOVES[count++] = m;
            m.promotesTo--;
            MOVES[count++] = m;
            m.promotesTo--;
            MOVES[count++] = m;
        } else {
            struct move_t m = {b + (8 * sign), b, 0, 0, 0, 0, 0};
            MOVES[count++] = m;
        }
    }

    while (step2) {
        u8 b = lsb(step2);
        flipBit(step2, b);
        struct move_t m = {b + (16 * sign), b, 0, 0, 0, 1, 0};
        MOVES[count++] = m;
    }

    while (capture1) {
        u8 b = lsb(capture1);
        flipBit(capture1, b);
        if ((1ULL << b) & epSquare) {
            struct move_t m = {b + (7 * sign), b, 0, 0, 1, 0, 0};
            MOVES[count++] = m;
        } else if ((1ULL << b) & pawnPromotionMask) {
            struct move_t m = {b + (7 * sign), b, 0, 0, 0, 0, 4};
            MOVES[count++] = m;
            m.promotesTo--;
            MOVES[count++] = m;
            m.promotesTo--;
            MOVES[count++] = m;
            m.promotesTo--;
            MOVES[count++] = m;
        } else {
            struct move_t m = {b + (7 * sign), b, 0, 0, 0, 0, 0};
            MOVES[count++] = m;
        }
    }

    while (capture2) {
        u8 b = lsb(capture2);
        flipBit(capture2, b);
        if ((1ULL << b) & epSquare) {
            struct move_t m = {b + (9 * sign), b, 0, 0, 1, 0, 0};
            MOVES[count++] = m;
        } else if ((1ULL << b) & pawnPromotionMask) {
            struct move_t m = {b + (9 * sign), b, 0, 0, 0, 0, 4};
            MOVES[count++] = m;
            m.promotesTo--;
            MOVES[count++] = m;
            m.promotesTo--;
            MOVES[count++] = m;
            m.promotesTo--;
            MOVES[count++] = m;
        } else {
            struct move_t m = {b + (9 * sign), b, 0, 0, 0, 0, 0};
            MOVES[count++] = m;
        }
    }

    return count;
}

int possiblecaptures(
    bool isWhiteToMove,
    u64 occupied, u64 epSquare, u64 myPieces, u64 enemyPieces, 
    u64 pawns, u64 knights, u64 bishops, u64 rooks, u64 queens, u64 king, 
    bool castleQueen, bool castleKing, struct move_t *CAPTURES
    ) {

    // just like possiblemoves, but only returns captures
    // this is used to generate the quiescence search tree
    possibleCalls++;
    int count = 0;

    u64 kingCaptures;
    u8 kingIndex = 0;

    if (king) {
        kingIndex = lsb(king);
        kingCaptures = kingAttacks[kingIndex] & enemyPieces; // this time we only want to look at moves that capture enemy pieces
    } else {
        printf("\n");
        displayBoard(occupied);
        printf("\nError: king not found\n");
        exit(1);
    }

    while (kingCaptures) {
        u8 b = lsb(kingCaptures);
        flipBit(kingCaptures, b);
        struct move_t m = {kingIndex, b, 5, 0, 0, 0, 0};
        CAPTURES[count++] = m;
    }

    u64 knightCaptures;

    while (knights) {
        u8 knightIndex = lsb(knights);
        flipBit(knights, knightIndex);
        knightCaptures = knightAttacks[knightIndex] & enemyPieces;
        while (knightCaptures) {
            u8 b = lsb(knightCaptures);
            flipBit(knightCaptures, b);
            struct move_t m = {knightIndex, b, 1, 0, 0, 0, 0};
            CAPTURES[count++] = m;
        }
    }
    
    u64 bishopCaptures;

    while (bishops) {
        u8 bishopIndex = lsb(bishops);
        flipBit(bishops, bishopIndex);
        bishopCaptures = generateBishopAttacks(bishopIndex, occupied, enemyPieces);
        while (bishopCaptures) {
            u8 b = lsb(bishopCaptures);
            flipBit(bishopCaptures, b);
            struct move_t m = {bishopIndex, b, 2, 0, 0, 0, 0};
            CAPTURES[count++] = m;
        }
    }

    u64 rookCaptures;

    while (rooks) {
        u8 rookIndex = lsb(rooks);
        flipBit(rooks, rookIndex);
        rookCaptures = generateRookAttacks(rookIndex, occupied, enemyPieces);
        while (rookCaptures) {
            u8 b = lsb(rookCaptures);
            flipBit(rookCaptures, b);
            struct move_t m = {rookIndex, b, 3, 0, 0, 0, 0};
            CAPTURES[count++] = m;
        }
    }

    u64 queenCaptures;

    while (queens) {
        u8 queenIndex = lsb(queens);
        flipBit(queens, queenIndex);
        queenCaptures = (generateBishopAttacks(queenIndex, occupied, enemyPieces)) | (generateRookAttacks(queenIndex, occupied, enemyPieces));
        while (queenCaptures) {
            u8 b = lsb(queenCaptures);
            flipBit(queenCaptures, b);
            struct move_t m = {queenIndex, b, 4, 0, 0, 0, 0};
            CAPTURES[count++] = m;
        }
    }

    u64 capture1, capture2;
    int sign;

    if (isWhiteToMove) {
        capture1 = ((pawns << 7) & ~leftmostFileMask) & (enemyPieces | epSquare);
        capture2 = ((pawns << 9) & ~rightmostFileMask) & (enemyPieces | epSquare);
        sign = -1;

    } else {
        capture1 = ((pawns >> 7) & ~rightmostFileMask) & (enemyPieces | epSquare);
        capture2 = ((pawns >> 9) & ~leftmostFileMask) & (enemyPieces | epSquare);
        sign = 1;
    }

        while (capture1) {
        u8 b = lsb(capture1);
        flipBit(capture1, b);
        if ((1ULL << b) & epSquare) {
            struct move_t m = {b + (7 * sign), b, 0, 0, 1, 0, 0};
            CAPTURES[count++] = m;
        } else if ((1ULL << b) & pawnPromotionMask) {
            struct move_t m = {b + (7 * sign), b, 0, 0, 0, 0, 4};
            CAPTURES[count++] = m;
            m.promotesTo--;
            CAPTURES[count++] = m;
            m.promotesTo--;
            CAPTURES[count++] = m;
            m.promotesTo--;
            CAPTURES[count++] = m;
        } else {
            struct move_t m = {b + (7 * sign), b, 0, 0, 0, 0, 0};
            CAPTURES[count++] = m;
        }
    }

    while (capture2) {
        u8 b = lsb(capture2);
        flipBit(capture2, b);
        if ((1ULL << b) & epSquare) {
            struct move_t m = {b + (9 * sign), b, 0, 0, 1, 0, 0};
            CAPTURES[count++] = m;
        } else if ((1ULL << b) & pawnPromotionMask) {
            struct move_t m = {b + (9 * sign), b, 0, 0, 0, 0, 4};
            CAPTURES[count++] = m;
            m.promotesTo--;
            CAPTURES[count++] = m;
            m.promotesTo--;
            CAPTURES[count++] = m;
            m.promotesTo--;
            CAPTURES[count++] = m;
        } else {
            struct move_t m = {b + (9 * sign), b, 0, 0, 0, 0, 0};
            CAPTURES[count++] = m;
        }
    }

    return count;
}