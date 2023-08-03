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

#define whitePawns 0
#define whiteKnights 1
#define whiteBishops 2
#define whiteRooks 3
#define whiteQueens 4
#define whiteKing 5
#define blackPawns 6
#define blackKnights 7
#define blackBishops 8
#define blackRooks 9
#define blackQueens 10
#define blackKing 11
#define whitePieces 12
#define blackPieces 13
#define allPieces 14

#define WHITE 1
#define BLACK -1

typedef struct bitboards_t {

    u64 bits[15];

    u8 pinnerForSquare[64];
    
    bool whiteCastleKingSide;
    bool whiteCastleQueenSide;
    
    bool blackCastleKingSide;
    bool blackCastleQueenSide;
    
    int enPassantSquare : 7;
    
    u64 hash;
    signed int color : 2;
    int pieceList[64];

    int whitePositionOpening;
    int blackPositionOpening;
    int whitePositionEndgame;
    int blackPositionEndgame;

    int whiteMaterial;
    int blackMaterial;

    int endgameFlag : 1;

} bitboards_t;

extern struct bitboards_t *bitboards;

extern u64 ZOBRIST_TABLE[64][13]; 
extern u64 whiteToMove;
extern u64 castlingRights[4];

void allocateBoards();

void freeBoards();

void initZobrist();

void initBoards(struct bitboards_t *BITBOARDS, bool isWhiteToMove, char* castle, char* enPas, int fiftyMove, int moveNum);
void initBoardsLight(struct bitboards_t *BITBOARDS, bool isWhiteToMove, char* castle, char* enPas);

void resetBoards(struct bitboards_t *BITBOARDS);
void resetBoardsLight(struct bitboards_t *BITBOARDS);

#endif