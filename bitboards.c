#include "bitboards.h"

// global bitboards
struct bitboards_t *bitboards;

// tables for hash updating
u64 ZOBRIST_TABLE[64][13]; 
u64 whiteToMove;
u64 castlingRights[4];

void allocateBoards() {
    // allocate memory for the bitboards
    bitboards = malloc(sizeof(struct bitboards_t));
    resetBoards(bitboards);
}

void freeBoards() {
    // free the memory allocated for the bitboards
    free(bitboards);
}

void initZobrist() {
    // initialize the zobrist table with random 64 bit numbers
    // for each square and type of piece
    // an en passant square is also treated as a piece type
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

u64 initBoardHash(struct bitboards_t *BITBOARDS, bool isWhiteToMove) {
    // initialize the board hash for the starting position
    // using xor makes it easy to update the hash when making a move and
    // if a move was to be undone, the hash would return back to the original
    // collisions can happen, but they are very rare
    u64 hash = 0;
    if (isWhiteToMove) {hash ^= whiteToMove;}
    if (BITBOARDS->blackCastleKingSide) {hash ^= castlingRights[0];}
    if (BITBOARDS->blackCastleQueenSide) {hash ^= castlingRights[1];}
    if (BITBOARDS->whiteCastleKingSide) {hash ^= castlingRights[2];}
    if (BITBOARDS->whiteCastleQueenSide) {hash ^= castlingRights[3];}
    for (int i = 0; i < 64; i++) {
        if (BITBOARDS->enPassantSquare == i) {
            hash ^= ZOBRIST_TABLE[i][12];
        }
        if (checkBit(BITBOARDS->allPieces, i)) {
            if (checkBit(BITBOARDS->whitePawns, i)) {hash ^= ZOBRIST_TABLE[i][0];}
            else if (checkBit(BITBOARDS->whiteKnights, i)) {hash ^= ZOBRIST_TABLE[i][1];}
            else if (checkBit(BITBOARDS->whiteBishops, i)) {hash ^= ZOBRIST_TABLE[i][2];}
            else if (checkBit(BITBOARDS->whiteRooks, i)) {hash ^= ZOBRIST_TABLE[i][3];}
            else if (checkBit(BITBOARDS->whiteQueens, i)) {hash ^= ZOBRIST_TABLE[i][4];}
            else if (checkBit(BITBOARDS->whiteKing, i)) {hash ^= ZOBRIST_TABLE[i][5];}
            else if (checkBit(BITBOARDS->blackPawns, i)) {hash ^= ZOBRIST_TABLE[i][6];}
            else if (checkBit(BITBOARDS->blackKnights, i)) {hash ^= ZOBRIST_TABLE[i][7];}
            else if (checkBit(BITBOARDS->blackBishops, i)) {hash ^= ZOBRIST_TABLE[i][8];}
            else if (checkBit(BITBOARDS->blackRooks, i)) {hash ^= ZOBRIST_TABLE[i][9];}
            else if (checkBit(BITBOARDS->blackQueens, i)) {hash ^= ZOBRIST_TABLE[i][10];}
            else if (checkBit(BITBOARDS->blackKing, i)) {hash ^= ZOBRIST_TABLE[i][11];}
        }
    }
    return hash;
}

bool stringContainsChar(char* string, char c) {
    return strchr(string, c) != NULL;
}

void initBoards(struct bitboards_t *BITBOARDS, bool isWhiteToMove, char* castle, char* enPas, int fiftyMove, int moveNum) {
    // set up the internal board and game counters
    // the BITBOARDS are used for a very fast move generation
    // the piece list is useful for determening which piece is on a given square since that
    // would require to check that bit in every bitboard resulting in long if chains or a switch
    // the list is mainly to speed up the move ordering
    // the evaluations are also kept within the board to avoid having to call an extra function to evaluate a position
    // they are updated when making a move
    halfMoveCount = fiftyMove;
    fullMoveCount = isWhiteToMove ? moveNum * 2 - 1 : moveNum * 2;

    int piece;

    for (int i = 0; i < 64; i++) {
        piece = BITBOARDS->pieceList[i];
        if (piece != 0) {
                setBit(BITBOARDS->allPieces, i);
            if (piece < 0) {
                setBit(BITBOARDS->blackPieces, i);
            } else {
                setBit(BITBOARDS->whitePieces, i);
            }
            if (piece == -1) {
                setBit(BITBOARDS->blackPawns, i);
                BITBOARDS->blackEvalOpening += blackEvalTables[OPENING][0][i];
                BITBOARDS->blackEvalEndgame += blackEvalTables[ENDGAME][0][i];
                }
            else if (piece == -2) {
                setBit(BITBOARDS->blackKnights, i);
                BITBOARDS->blackEvalOpening += blackEvalTables[OPENING][1][i];
                BITBOARDS->blackEvalEndgame += blackEvalTables[ENDGAME][1][i];
                }
            else if (piece == -3) {
                setBit(BITBOARDS->blackBishops, i);
                BITBOARDS->blackEvalOpening += blackEvalTables[OPENING][2][i];
                BITBOARDS->blackEvalEndgame += blackEvalTables[ENDGAME][2][i];
                }
            else if (piece == -4) {
                setBit(BITBOARDS->blackRooks, i);
                BITBOARDS->blackEvalOpening += blackEvalTables[OPENING][3][i];
                BITBOARDS->blackEvalEndgame += blackEvalTables[ENDGAME][3][i];
                }
            else if (piece == -5) {
                setBit(BITBOARDS->blackQueens, i);
                BITBOARDS->blackEvalOpening += blackEvalTables[OPENING][4][i];
                BITBOARDS->blackEvalEndgame += blackEvalTables[ENDGAME][4][i];
                }
            else if (piece == -6) {
                setBit(BITBOARDS->blackKing, i);
                BITBOARDS->blackEvalOpening += blackEvalTables[OPENING][5][i];
                BITBOARDS->blackEvalEndgame += blackEvalTables[ENDGAME][5][i];
                }
            else if (piece == 1) {
                setBit(BITBOARDS->whitePawns, i);
                BITBOARDS->whiteEvalOpening += whiteEvalTables[OPENING][0][i];
                BITBOARDS->whiteEvalEndgame += whiteEvalTables[ENDGAME][0][i];
                }
            else if (piece == 2) {
                setBit(BITBOARDS->whiteKnights, i);
                BITBOARDS->whiteEvalOpening += whiteEvalTables[OPENING][1][i];
                BITBOARDS->whiteEvalEndgame += whiteEvalTables[ENDGAME][1][i];
            }
            else if (piece == 3) {
                setBit(BITBOARDS->whiteBishops, i);
                BITBOARDS->whiteEvalOpening += whiteEvalTables[OPENING][2][i];
                BITBOARDS->whiteEvalEndgame += whiteEvalTables[ENDGAME][2][i];
                }
            else if (piece == 4) {
                setBit(BITBOARDS->whiteRooks, i);
                BITBOARDS->whiteEvalOpening += whiteEvalTables[OPENING][3][i];
                BITBOARDS->whiteEvalEndgame += whiteEvalTables[ENDGAME][3][i];
                }
            else if (piece == 5) {
                setBit(BITBOARDS->whiteQueens, i);
                BITBOARDS->whiteEvalOpening += whiteEvalTables[OPENING][4][i];
                BITBOARDS->whiteEvalEndgame += whiteEvalTables[ENDGAME][4][i];
                }
            else if (piece == 6) {
                setBit(BITBOARDS->whiteKing, i);
                BITBOARDS->whiteEvalOpening += whiteEvalTables[OPENING][5][i];
                BITBOARDS->whiteEvalEndgame += whiteEvalTables[ENDGAME][5][i];
                }
        }
    }

    if (stringContainsChar(castle, 'K')) {
        BITBOARDS->whiteCastleKingSide = true;
    }
    if (stringContainsChar(castle, 'Q')) {
        BITBOARDS->whiteCastleQueenSide = true;
    }
    if (stringContainsChar(castle, 'k')) {
        BITBOARDS->blackCastleKingSide = true;
    }
    if (stringContainsChar(castle, 'q')) {
        BITBOARDS->blackCastleQueenSide = true;
    }

    if (enPas[0] != '-') {
        BITBOARDS->enPassantSquare = 7 - (enPas[0] - 'a') + (enPas[1] - '1') * 8;
    }
    
    BITBOARDS->hash = initBoardHash(BITBOARDS, isWhiteToMove);
    BITBOARDS->color = isWhiteToMove;
}

void initBoardsLight (struct bitboards_t *BITBOARDS, bool isWhiteToMove, char* castle, char* enPas) {

    int piece;

    for (int i = 0; i < 64; i++) {
        piece = BITBOARDS->pieceList[i];
        
        if (piece != 0) {
            BITBOARDS->allPieces |= (1ULL << i);

            if (piece < 0) {
                BITBOARDS->blackPieces |= (1ULL << i);
            } else {
                BITBOARDS->whitePieces |= (1ULL << i);
            }

            switch (piece) {
                case -1:
                    BITBOARDS->blackPawns |= (1ULL << i);
                    break;
                case -2:
                    BITBOARDS->blackKnights |= (1ULL << i);
                    break;
                case -3:
                    BITBOARDS->blackBishops |= (1ULL << i);
                    break;
                case -4:
                    BITBOARDS->blackRooks |= (1ULL << i);
                    break;
                case -5:
                    BITBOARDS->blackQueens |= (1ULL << i);
                    break;
                case -6:
                    BITBOARDS->blackKing |= (1ULL << i);
                    break;
                case 1:
                    BITBOARDS->whitePawns |= (1ULL << i);
                    break;
                case 2:
                    BITBOARDS->whiteKnights |= (1ULL << i);
                    break;
                case 3:
                    BITBOARDS->whiteBishops |= (1ULL << i);
                    break;
                case 4:
                    BITBOARDS->whiteRooks |= (1ULL << i);
                    break;
                case 5:
                    BITBOARDS->whiteQueens |= (1ULL << i);
                    break;
                case 6:
                    BITBOARDS->whiteKing |= (1ULL << i);
                    break;
            }
        }
    }

    if (stringContainsChar(castle, 'K')) {
        BITBOARDS->whiteCastleKingSide = true;
    }
    if (stringContainsChar(castle, 'Q')) {
        BITBOARDS->whiteCastleQueenSide = true;
    }
    if (stringContainsChar(castle, 'k')) {
        BITBOARDS->blackCastleKingSide = true;
    }
    if (stringContainsChar(castle, 'q')) {
        BITBOARDS->blackCastleQueenSide = true;
    }

    if (enPas[0] != '-') {
        BITBOARDS->enPassantSquare |= (1ULL << (7 - (enPas[0] - 'a') + (enPas[1] - '1') * 8));
    } else {
        BITBOARDS->enPassantSquare = -1;
    }
    
    BITBOARDS->hash = initBoardHash(BITBOARDS, isWhiteToMove);
    BITBOARDS->color = isWhiteToMove;
}

void resetBoards(struct bitboards_t *BITBOARDS) {
    // used for parsing the opening book
    BITBOARDS->whitePawns = 0;
    BITBOARDS->whiteKnights = 0;
    BITBOARDS->whiteBishops = 0;
    BITBOARDS->whiteRooks = 0;
    BITBOARDS->whiteQueens = 0;
    BITBOARDS->whiteKing = 0;
    BITBOARDS->whiteCastleQueenSide = false;
    BITBOARDS->whiteCastleKingSide = false;
    BITBOARDS->blackPawns = 0;
    BITBOARDS->blackKnights = 0;
    BITBOARDS->blackBishops = 0;
    BITBOARDS->blackRooks = 0;
    BITBOARDS->blackQueens = 0;
    BITBOARDS->blackKing = 0;
    BITBOARDS->blackCastleQueenSide = false;
    BITBOARDS->blackCastleKingSide = false;
    BITBOARDS->whitePieces = 0;
    BITBOARDS->blackPieces = 0;
    BITBOARDS->allPieces = 0;
    BITBOARDS->enPassantSquare = -1;
    BITBOARDS->hash = 0;
    BITBOARDS->color = false;
    memset(BITBOARDS->pieceList, 0, sizeof(BITBOARDS->pieceList));
    BITBOARDS->whiteEvalOpening = 0;
    BITBOARDS->blackEvalOpening = 0;
    BITBOARDS->whiteEvalEndgame = 0;
    BITBOARDS->blackEvalEndgame = 0;
}

void resetBoardsLight(struct bitboards_t *BITBOARDS) {
    BITBOARDS->whitePawns = 0;
    BITBOARDS->whiteKnights = 0;
    BITBOARDS->whiteBishops = 0;
    BITBOARDS->whiteRooks = 0;
    BITBOARDS->whiteQueens = 0;
    BITBOARDS->whiteKing = 0;
    BITBOARDS->whiteCastleQueenSide = false;
    BITBOARDS->whiteCastleKingSide = false;
    BITBOARDS->blackPawns = 0;
    BITBOARDS->blackKnights = 0;
    BITBOARDS->blackBishops = 0;
    BITBOARDS->blackRooks = 0;
    BITBOARDS->blackQueens = 0;
    BITBOARDS->blackKing = 0;
    BITBOARDS->blackCastleQueenSide = false;
    BITBOARDS->blackCastleKingSide = false;
    BITBOARDS->whitePieces = 0;
    BITBOARDS->blackPieces = 0;
    BITBOARDS->allPieces = 0;
    BITBOARDS->enPassantSquare = -1;
    BITBOARDS->hash = 0;
    BITBOARDS->color = false;
    memset(BITBOARDS->pieceList, 0, sizeof(BITBOARDS->pieceList));
}