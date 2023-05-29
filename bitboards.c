#include "bitboards.h"

struct bitboards_t bitboards = {
    0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0,
    0, 0, 0,
    false, false,
    false, false,
    0, 0,
    true,
    {0},
    0,
    0
};

u64 ZOBRIST_TABLE[64][13]; 
u64 whiteToMove;
u64 castlingRights[4];

void initZobrist() {
    // initialize the zobrist table with random 64 bit numbers
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 13; j++) {
            ZOBRIST_TABLE[i][j] = rand64();
        }
    }
    whiteToMove = rand64();
    castlingRights[0] = rand64();
    castlingRights[1] = rand64();
    castlingRights[2] = rand64();
    castlingRights[3] = rand64();
}

u64 initBoardHash(struct bitboards_t BITBOARDS, bool isWhiteToMove) {
    // initialize the board hash for the starting position
    u64 hash = 0;
    if (isWhiteToMove) {hash ^= whiteToMove;}
    if (BITBOARDS.blackCastleKingSide) {hash ^= castlingRights[0];}
    if (BITBOARDS.blackCastleQueenSide) {hash ^= castlingRights[1];}
    if (BITBOARDS.whiteCastleKingSide) {hash ^= castlingRights[2];}
    if (BITBOARDS.whiteCastleQueenSide) {hash ^= castlingRights[3];}
    for (int i = 0; i < 64; i++) {
        if (checkBit(BITBOARDS.enPassantSquare, i)) {
            hash ^= ZOBRIST_TABLE[i][12];
        }
        if (checkBit(BITBOARDS.allPieces, i)) {
            if (checkBit(BITBOARDS.whitePawns, i)) {hash ^= ZOBRIST_TABLE[i][0];}
            else if (checkBit(BITBOARDS.whiteRooks, i)) {hash ^= ZOBRIST_TABLE[i][1];}
            else if (checkBit(BITBOARDS.whiteKnights, i)) {hash ^= ZOBRIST_TABLE[i][2];}
            else if (checkBit(BITBOARDS.whiteBishops, i)) {hash ^= ZOBRIST_TABLE[i][3];}
            else if (checkBit(BITBOARDS.whiteQueens, i)) {hash ^= ZOBRIST_TABLE[i][4];}
            else if (checkBit(BITBOARDS.whiteKing, i)) {hash ^= ZOBRIST_TABLE[i][5];}
            else if (checkBit(BITBOARDS.blackPawns, i)) {hash ^= ZOBRIST_TABLE[i][6];}
            else if (checkBit(BITBOARDS.blackRooks, i)) {hash ^= ZOBRIST_TABLE[i][7];}
            else if (checkBit(BITBOARDS.blackKnights, i)) {hash ^= ZOBRIST_TABLE[i][8];}
            else if (checkBit(BITBOARDS.blackBishops, i)) {hash ^= ZOBRIST_TABLE[i][9];}
            else if (checkBit(BITBOARDS.blackQueens, i)) {hash ^= ZOBRIST_TABLE[i][10];}
            else if (checkBit(BITBOARDS.blackKing, i)) {hash ^= ZOBRIST_TABLE[i][11];}
        }
    }
    return hash;
}

bool stringContainsChar(char* string, char c) {
    return strchr(string, c) != NULL;
}

void initBoards(int startPosition[64], bool isWhiteToMove, char* castle, char* enPas, int fiftyMove, int moveNum) {
    // initialize the bitboards and the move history from a FEN string
    halfMoveCount = fiftyMove;
    moveCount = isWhiteToMove ? moveNum * 2 - 1 : moveNum * 2;

    int piece;
    int b = 63;

    for (int i = 0; i < 64; i++) {
        piece = startPosition[i];
        bitboards.pieceList[b] = piece;
        if (piece != 0) {
                setBit(bitboards.allPieces, b);
            if (piece < 0) {
                setBit(bitboards.blackPieces, b);
            } else {
                setBit(bitboards.whitePieces, b);
            }
            if (piece == -1) {
                setBit(bitboards.blackPawns, b);
                bitboards.blackEval += pawnEvalBlack[b];
                }
            else if (piece == -2) {
                setBit(bitboards.blackKnights, b);
                bitboards.blackEval += knightEvalBlack[b];
                }
            else if (piece == -3) {
                setBit(bitboards.blackBishops, b);
                bitboards.blackEval += bishopEvalBlack[b];
                }
            else if (piece == -4) {
                setBit(bitboards.blackRooks, b);
                bitboards.blackEval += rookEvalBlack[b];
                }
            else if (piece == -5) {
                setBit(bitboards.blackQueens, b);
                bitboards.blackEval += queenEvalBlack[b];
                }
            else if (piece == -6) {
                setBit(bitboards.blackKing, b);
                }
            else if (piece == 1) {
                setBit(bitboards.whitePawns, b);
                bitboards.whiteEval += pawnEvalWhite[b];
                }
            else if (piece == 2) {
                setBit(bitboards.whiteKnights, b);
                bitboards.whiteEval += knightEvalWhite[b];
            }
            else if (piece == 3) {
                setBit(bitboards.whiteBishops, b);
                bitboards.whiteEval += bishopEvalWhite[b];
                }
            else if (piece == 4) {
                setBit(bitboards.whiteRooks, b);
                bitboards.whiteEval += rookEvalWhite[b];
                }
            else if (piece == 5) {
                setBit(bitboards.whiteQueens, b);
                bitboards.whiteEval += queenEvalWhite[b];
                }
            else if (piece == 6) {
                setBit(bitboards.whiteKing, b);
                }
        }
        b--;
    }

    if (stringContainsChar(castle, 'K')) {
        bitboards.whiteCastleKingSide = true;
    }
    if (stringContainsChar(castle, 'Q')) {
        bitboards.whiteCastleQueenSide = true;
    }
    if (stringContainsChar(castle, 'k')) {
        bitboards.blackCastleKingSide = true;
    }
    if (stringContainsChar(castle, 'q')) {
        bitboards.blackCastleQueenSide = true;
    }

    if (enPas[0] != '-') {
        int epSquare = 7 - (enPas[0] - 'a') + (enPas[1] - '1') * 8;
        setBit(bitboards.enPassantSquare, epSquare);
    }
    
    bitboards.hash = initBoardHash(bitboards, isWhiteToMove);
    bitboards.color = isWhiteToMove;
}

void resetBoards() {
    bitboards.whitePawns = 0;
    bitboards.whiteKnights = 0;
    bitboards.whiteBishops = 0;
    bitboards.whiteRooks = 0;
    bitboards.whiteQueens = 0;
    bitboards.whiteKing = 0;
    bitboards.whiteCastleQueenSide = 0;
    bitboards.whiteCastleKingSide = 0;
    bitboards.blackPawns = 0;
    bitboards.blackKnights = 0;
    bitboards.blackBishops = 0;
    bitboards.blackRooks = 0;
    bitboards.blackQueens = 0;
    bitboards.blackKing = 0;
    bitboards.blackCastleQueenSide = 0;
    bitboards.blackCastleKingSide = 0;
    bitboards.whitePieces = 0;
    bitboards.blackPieces = 0;
    bitboards.allPieces = 0;
    bitboards.enPassantSquare = 0;
    bitboards.hash = 0;
    bitboards.color = false;
    memset(bitboards.pieceList, 0, sizeof(bitboards.pieceList));
    bitboards.whiteEval = 0;
    bitboards.blackEval = 0;
}