#ifndef BITBOARDS_H
#define BITBOARDS_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "counters.h"
#include "bitOperations.h"
#include "random.h"
#include "eval.h"

#define u64 uint64_t
#define u8 uint8_t

typedef struct bitboards_t {
    u64 whitePawns;
    u64 whiteKnights;
    u64 whiteBishops;
    u64 whiteRooks;
    u64 whiteQueens;
    u64 whiteKing;

    u64 blackPawns;
    u64 blackKnights;
    u64 blackBishops;
    u64 blackRooks;
    u64 blackQueens;
    u64 blackKing;
    
    u64 whitePieces;
    u64 blackPieces;
    u64 allPieces;
    
    bool whiteCastleKingSide;
    bool whiteCastleQueenSide;
    
    bool blackCastleKingSide;
    bool blackCastleQueenSide;
    
    u64 enPassantSquare;
    
    u64 hash;
    bool color;
    int pieceList[64];
    int whiteEval;
    int blackEval;

} bitboards_t;

extern struct bitboards_t bitboards;

extern u64 ZOBRIST_TABLE[64][13]; 
extern u64 whiteToMove;
extern u64 castlingRights[4];

void initZobrist();

void initBoards(int startPosition[64], bool isWhiteToMove, char* castle, char* enPas, int fiftyMove, int moveNum);

void resetBoards();

#endif