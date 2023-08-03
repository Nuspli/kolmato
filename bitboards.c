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
        if (BITBOARDS->pieceList[i] != 0) {
            hash ^= ZOBRIST_TABLE[i][BITBOARDS->pieceList[i] > 0 ? BITBOARDS->pieceList[i] - 1 : -(BITBOARDS->pieceList[i]) + 5];
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
            int index = piece > 0 ? piece - 1 : -(piece) + 5;
            setBit(BITBOARDS->bits[allPieces], i);
            if (piece < 0) {
                setBit(BITBOARDS->bits[blackPieces], i);
                BITBOARDS->blackPositionOpening -= whitePositionTables[OPENING][index - 6][i ^ 56];
                BITBOARDS->blackPositionEndgame -= whitePositionTables[ENDGAME][index - 6][i ^ 56];
                BITBOARDS->blackMaterial -= materialValues[index - 6];

            } else {
                setBit(BITBOARDS->bits[whitePieces], i);
                BITBOARDS->whitePositionOpening += whitePositionTables[OPENING][index][i];
                BITBOARDS->whitePositionEndgame += whitePositionTables[ENDGAME][index][i];
                BITBOARDS->whiteMaterial += materialValues[index];
            }
            setBit(BITBOARDS->bits[index], i);
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
    BITBOARDS->color = isWhiteToMove ? WHITE : BLACK;
}

void initBoardsLight (struct bitboards_t *BITBOARDS, bool isWhiteToMove, char* castle, char* enPas) {

    int piece;

    for (int i = 0; i < 64; i++) {
        piece = BITBOARDS->pieceList[i];
        
        if (piece != 0) {
                setBit(BITBOARDS->bits[allPieces], i);
            if (piece < 0) {
                setBit(BITBOARDS->bits[blackPieces], i);
            } else {
                setBit(BITBOARDS->bits[whitePieces], i);
            }
            setBit(BITBOARDS->bits[piece > 0 ? piece - 1 : -(piece) + 5], i);
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
    BITBOARDS->color = isWhiteToMove ? WHITE : BLACK;
}

void resetBoards(struct bitboards_t *BITBOARDS) {
    // used for parsing the opening book
    memset(BITBOARDS->bits, 0, sizeof(BITBOARDS->bits));
    BITBOARDS->whiteCastleQueenSide = false;
    BITBOARDS->whiteCastleKingSide = false;
    BITBOARDS->blackCastleQueenSide = false;
    BITBOARDS->blackCastleKingSide = false;
    BITBOARDS->enPassantSquare = -1;
    BITBOARDS->hash = 0;
    BITBOARDS->color = 0;
    memset(BITBOARDS->pieceList, 0, sizeof(BITBOARDS->pieceList));
    BITBOARDS->whitePositionOpening = 0;
    BITBOARDS->blackPositionOpening = 0;
    BITBOARDS->whitePositionEndgame = 0;
    BITBOARDS->blackPositionEndgame = 0;
    BITBOARDS->whiteMaterial = 0;
    BITBOARDS->blackMaterial = 0;
    memset(BITBOARDS->pinnerForSquare, 0, sizeof(BITBOARDS->pinnerForSquare));
    BITBOARDS->endgameFlag = 0;
}

void resetBoardsLight(struct bitboards_t *BITBOARDS) {
    memset(BITBOARDS->bits, 0, sizeof(BITBOARDS->bits));
    BITBOARDS->whiteCastleQueenSide = false;
    BITBOARDS->whiteCastleKingSide = false;
    BITBOARDS->blackCastleQueenSide = false;
    BITBOARDS->blackCastleKingSide = false;
    BITBOARDS->enPassantSquare = -1;
    BITBOARDS->hash = 0;
    BITBOARDS->color = 0;
    memset(BITBOARDS->pieceList, 0, sizeof(BITBOARDS->pieceList));
    memset(BITBOARDS->pinnerForSquare, 0, sizeof(BITBOARDS->pinnerForSquare));
}