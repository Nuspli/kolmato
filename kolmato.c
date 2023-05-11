#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <time.h>
#include "inttypes.h"
#include "math.h"

// DEFINITIONS

# define p 1
# define n 2
# define b 3
# define r 4
# define q 5
# define k 6

# define P -1
# define N -2
# define B -3
# define R -4
# define Q -5
# define K -6

// #define KINGSIDE 1
// #define QUEENSIDE 2

# define EXACT 0
# define LOWERBOUND 1
# define UPPERBOUND 2

# define INF 100000 // infinity

# define TABLE_SIZE 10000000
# define BOOK_SIZE 2000000  // can't be changed unless the book is recompiled
                            // todo: make that possible

// PRINT

void printLogo() {                                                                          
    printf(",dPYb,                 ,dPYb,                                 I8               \n");
    printf("IP'`Yb                 IP'`Yb                                 I8               \n");
    printf("I8  8I                 I8  8I                              88888888            \n");
    printf("I8  8bgg,              I8  8'                                 I8               \n");
    printf("I8 dP\" \"8    ,ggggg,   I8 dP   ,ggg,,ggg,,ggg,     ,gggg,gg   I8     ,ggggg,   \n");
    printf("I8d8bggP\"   dP\"  \"Y8gggI8dP   ,8\" \"8P\" \"8P\" \"8,   dP\"  \"Y8I   I8    dP\"  \"Y8ggg\n");
    printf("I8P' \"Yb,  i8'    ,8I  I8P    I8   8I   8I   8I  i8'    ,8I  ,I8,  i8'    ,8I  \n");
    printf(",d8    `Yb,,d8,   ,d8' ,d8b,_ ,dP   8I   8I   Yb,,d8,   ,d8b,,d88b,,d8,   ,d8' \n");
    printf("88P      Y8P\"Y8888P\"   8P'\"Y888P'   8I   8I   `Y8P\"Y8888P\"`Y88P\"\"Y8P\"Y8888P\"   \n\n\n");
}

void printHelp() {
    printf("help menu\n\n");
    printf("This is a chess engine which was originally written in javascript.\n");
    printf("You can find a hosted version of the old JS one on https://nuspli.github.io/chess/chess.html\n");
    printf("For the source code, please visit [LINK HERE]. If you are looking for more sources to develop your own chess engine,\n");
    printf("I recommend the following:\n");
    printf(">>> https://www.chessprogramming.org\n");
    printf(">>> https://www.cs.cmu.edu/afs/cs/academic/class/15418-s12/www/competition/www.contrib.andrew.cmu.edu/~jvirdo/rasmussen-2004.pdf\n");
    printf(">>> Video by Sebastian Lague - https://www.youtube.com/watch?v=U4ogK0MIzqk\n\n");
    printf("Commands: \n");
    printf(">>> -[h]elp                                - print this help message\n");
    printf(">>> -[e]valuate [FEN_STRING]               - get a quick evaluation of a given FEN\n");
    printf(">>> -[b]est     [FEN_STRING]               - calculate the best move for a given FEN\n");
    printf("                -[t]ime                    - set the time limit in seconds for the engine to think\n");
    printf("                -[d]epth                   - set the depth limit for the engine to think\n");
    printf("                -[b]                       - use the opening book\n");
    printf(">>> -[g]ame     [FEN_STRING]               - play against the engine\n");
    printf("                -[t]ime                    - set the time limit in seconds for the engine to think\n");
    printf("                -[d]epth                   - set the depth limit for the engine to think\n");
    printf("                -[b]ook                    - use the opening book\n\n");
    printf(">>> -[p]erft    [FEN_STRING]               - bulk count of leaf nodes for a given FEN\n\n");
    // passed all of the test positions at: https://www.chessprogramming.org/Perft_Results
    printf("FEN_STRING                                 enter in this format:\n");
    printf("                                           rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1\n");
    printf("DEFAULTS                                   max time: 10 seconds\n");
    printf("                                           max depth: 20 ply\n\n");
}

char notation[64][3] = {
    // to translate a move from number to notation
    "h1", "g1", "f1", "e1", "d1", "c1", "b1", "a1",
    "h2", "g2", "f2", "e2", "d2", "c2", "b2", "a2",
    "h3", "g3", "f3", "e3", "d3", "c3", "b3", "a3",
    "h4", "g4", "f4", "e4", "d4", "c4", "b4", "a4",
    "h5", "g5", "f5", "e5", "d5", "c5", "b5", "a5",
    "h6", "g6", "f6", "e6", "d6", "c6", "b6", "a6",
    "h7", "g7", "f7", "e7", "d7", "c7", "b7", "a7",
    "h8", "g8", "f8", "e8", "d8", "c8", "b8", "a8"
};

void printBinary(uint64_t x) {
    // print the binary representation of a 64 bit integer
    for (int i = 63; i >= 0; i--) {
        uint64_t mask = 1ULL << i;  // Create a mask with the i-th bit set to 1
        int bit = (x & mask) ? 1 : 0;  // Extract the i-th bit from n

        printf("%d", bit);  // Print the bit (0 or 1)
    }
    printf("\n");
}

// BASIC FUNCTIONS AND BIT OPERATIONS

bool stringContainsChar(char* string, char c) {
    for (int i = 0; i < strlen(string); i++) {
        if (string[i] == c) {
            return true;
        }
    }
    return false;
}

int min(int x, int y) {
    return x < y ? x : y;
}

int max(int x, int y) {
    return x > y ? x : y;
}

uint64_t rand64() {
    // "fake" random 64 bit number
    return ((uint64_t)rand() << 32) | rand();
}

uint8_t bitCount(uint64_t num) {
    // count the number of bits set to 1 in number
    return __builtin_popcountll(num);
}

// Set the bit at the given index to 1
uint64_t setBit(uint64_t num, int index) {
    return num | (1ULL << index);
}

// Unset the bit at the given index to 0
uint64_t unsetBit(uint64_t num, int index) {
    return num & ~(1ULL << index);
}

// Check if the bit at the given index is set to 1
bool checkBit(uint64_t bitboard, int index) {
    return bitboard & (1ULL << index);
}

// Get the index of the least significant bit set to 1
uint8_t lsb(uint64_t num) {
    return __builtin_ctzll(num);
}

uint8_t popLsb(uint64_t *num) {
    // remove the least significant bit set to 1 and return its index
    uint8_t index = lsb(*num);
    *num &= ~(1ULL << index);
    return index;
}

// STATIC DATA

static uint64_t rightmostFileMask = 0x0101010101010101;
static uint64_t leftmostFileMask = 0x8080808080808080;
static uint64_t whitePawnStartRank = 0x000000000000FF00;
static uint64_t blackPawnStartRank = 0x00FF000000000000;

/*
normal board:

63 62 61 60 59 58 57 56
55 54 53 52 51 50 49 48
47 46 45 44 43 42 41 40
39 38 37 36 35 34 33 32
31 30 29 28 27 26 25 24
23 22 21 20 19 18 17 16
15 14 13 12 11 10  9  8
 7  6  5  4  3  2  1  0
*/

static int rotated90[64] = {
    // each entry represents where the piece at that index would be after a 90 degree clockwise rotation
     7, 15, 23, 31, 39, 47, 55, 63,
     6, 14, 22, 30, 38, 46, 54, 62,
     5, 13, 21, 29, 37, 45, 53, 61,
     4, 12, 20, 28, 36, 44, 52, 60,
     3, 11, 19, 27, 35, 43, 51, 59,
     2, 10, 18, 26, 34, 42, 50, 58,
     1, 9,  17, 25, 33, 41, 49, 57,
     0, 8,  16, 24, 32, 40, 48, 56
};

static int rotated45R[64] = {
    // each entry represents where the piece at that index would be after a 45 degree clockwise rotation
     0,  2,  5,  9, 14, 20, 27, 35, 
     1,  4,  8, 13, 19, 26, 34, 42, 
     3,  7, 12, 18, 25, 33, 41, 48, 
     6, 11, 17, 24, 32, 40, 47, 53, 
    10, 16, 23, 31, 39, 46, 52, 57, 
    15, 22, 30, 38, 45, 51, 56, 60, 
    21, 29, 37, 44, 50, 55, 59, 62, 
    28, 36, 43, 49, 54, 58, 61, 63
};

static int rotated45L[64] = {
    28, 21, 15, 10,  6,  3,  1,  0, 
    36, 29, 22, 16, 11,  7,  4,  2, 
    43, 37, 30, 23, 17, 12,  8,  5, 
    49, 44, 38, 31, 24, 18, 13,  9, 
    54, 50, 45, 39, 32, 25, 19, 14, 
    58, 55, 51, 46, 40, 33, 26, 20, 
    61, 59, 56, 52, 47, 41, 34, 27, 
    63, 62, 60, 57, 53, 48, 42, 35
};

// evaluation tables for the pieces on their possible positions
// for example, a pawn close to promoting is worth more than a pawn on the starting position
// black and white are the same, but mirrored and negated (black is the minimizer, white is the maximizer)
static int pawnEvalBlack[64] = {
    - 90,- 90,- 90,- 90,- 90,- 90,- 90,- 90,
    -140,-140,-140,-150,-150,-140,-140,-140,
    -100,-100,-110,-130,-130,-110,-100,-100,
    - 95,- 95,-100,-125,-120,-100,- 95,- 95,
    - 90,- 90,- 90,-120,-115,- 90,- 90,- 90,
    - 95,- 85,- 80,- 90,- 90,- 80,- 85,- 95,
    - 95,-100,-100,- 65,- 75,-100,-100,- 95,
    - 90,- 90,- 90,- 90,- 90,- 90,- 90,- 90
    };

static int rookEvalBlack[64] = {
    -500,-500,-500,-500,-500,-500,-500,-500,
    -505,-510,-510,-510,-510,-510,-510,-505,
    -495,-500,-500,-500,-500,-500,-500,-495,
    -495,-500,-500,-500,-500,-500,-500,-495,
    -495,-500,-500,-500,-500,-500,-500,-495,
    -495,-500,-500,-500,-500,-500,-500,-495,
    -495,-500,-500,-500,-500,-500,-500,-495,
    -500,-500,-500,-505,-505,-500,-500,-500
    };

static int knightEvalBlack[64] = {
    -280,-290,-300,-300,-300,-300,-290,-280,
    -290,-310,-330,-330,-330,-330,-310,-290,
    -300,-330,-340,-345,-345,-340,-330,-300,
    -300,-335,-345,-350,-350,-345,-335,-300,
    -300,-330,-345,-350,-350,-345,-330,-300,
    -300,-335,-340,-345,-345,-340,-335,-300,
    -290,-310,-330,-335,-335,-330,-310,-290,
    -280,-290,-300,-300,-300,-300,-290,-280
    };

static int bishopEvalBlack[64] = {
    -310,-320,-320,-320,-320,-320,-320,-310,
    -320,-330,-330,-330,-330,-330,-330,-320,
    -320,-330,-335,-340,-340,-335,-330,-320,
    -320,-335,-335,-340,-340,-335,-335,-320,
    -320,-330,-340,-340,-340,-340,-330,-320,
    -320,-340,-340,-340,-340,-340,-340,-320,
    -320,-335,-330,-330,-330,-330,-335,-320,
    -310,-320,-320,-320,-320,-320,-320,-310
    };
  
  static int queenEvalBlack[64] = {
    -880,-890,-890,-895,-895,-890,-890,-880,
    -890,-900,-900,-900,-900,-900,-900,-890,
    -890,-900,-905,-905,-905,-905,-900,-890,
    -895,-900,-905,-905,-905,-905,-900,-895,
    -895,-900,-905,-905,-905,-905,-900,-900,
    -890,-900,-905,-905,-905,-905,-905,-890,
    -890,-900,-900,-900,-900,-905,-900,-890,
    -880,-890,-890,-895,-895,-890,-890,-880
    };

  static int kingEvalBlack[64] = {
     30,  40,  40,  50,  50,  40,  40,  30,
     30,  40,  40,  50,  50,  40,  40,  30,
     30,  40,  40,  50,  50,  40,  40,  30,
     30,  40,  40,  50,  50,  40,  40,  30,
     20,  30,  30,  40,  40,  30,  30,  20,
     10,  20,  20,  20,  20,  20,  20,  10,
    -20, -20,   0,   0,   0,   0, -20, -20,
    -20, -30, -10,   0,   0, -10, -30, -20
    };

static int pawnEvalWhite[64] = {
      90,  90,  90,  90,  90,  90,  90,  90,
      95, 100, 100,  65,  75, 100, 100,  95,
      95,  85,  80,  90,  90,  80,  85,  95,
      90,  90,  90, 120, 115,  90,  90,  90,
      95,  95, 100, 125, 120, 100,  95,  95,
     100, 100, 110, 130, 130, 110, 100, 100,
     140, 140, 140, 150, 150, 140, 140, 140,
      90,  90,  90,  90,  90,  90,  90,  90
    };

static int rookEvalWhite[64] = {
     500, 500, 500, 505, 505, 500, 500, 500,
     495, 500, 500, 500, 500, 500, 500, 495,
     495, 500, 500, 500, 500, 500, 500, 495,
     495, 500, 500, 500, 500, 500, 500, 495,
     495, 500, 500, 500, 500, 500, 500, 495,
     495, 500, 500, 500, 500, 500, 500, 495,
     505, 510, 510, 510, 510, 510, 510, 505,
     500, 500, 500, 500, 500, 500, 500, 500
    };

static int knightEvalWhite[64] = {
     280, 290, 300, 300, 300, 300, 290, 280,
     290, 310, 330, 335, 335, 330, 310, 290,
     300, 335, 340, 345, 345, 340, 335, 300,
     300, 330, 345, 350, 350, 345, 330, 300,
     300, 335, 345, 350, 350, 345, 335, 300,
     300, 330, 340, 345, 345, 340, 330, 300,
     290, 310, 330, 330, 330, 330, 310, 290,
     280, 290, 300, 300, 300, 300, 290, 280
    };

static int bishopEvalWhite[64] = {
     310, 320, 320, 320, 320, 320, 320, 310,
     320, 335, 330, 330, 330, 330, 335, 320,
     320, 340, 340, 340, 340, 340, 340, 320,
     320, 330, 340, 340, 340, 340, 330, 320,
     320, 335, 335, 340, 340, 335, 335, 320,
     320, 330, 335, 340, 340, 335, 330, 320,
     320, 330, 330, 330, 330, 330, 330, 320,
     310, 320, 320, 320, 320, 320, 320, 310
    };

static int queenEvalWhite[64] = {
     880, 890, 890, 895, 895, 890, 890, 880,
     890, 900, 900, 900, 900, 905, 900, 890,
     890, 900, 905, 905, 905, 905, 905, 890,
     895, 900, 905, 905, 905, 905, 900, 900,
     895, 900, 905, 905, 905, 905, 900, 895,
     890, 900, 905, 905, 905, 905, 900, 890,
     890, 900, 900, 900, 900, 900, 900, 890,
     880, 890, 890, 895, 895, 890, 890, 880
    };

static int kingEvalWhite[64] = {
     20,  30,  10,   0,   0,  10,  30,  20,
     20,  20,   0,   0,   0,   0,  20,  20,
    -10, -20, -20, -20, -20, -20, -20, -10,
    -20, -30, -30, -40, -40, -30, -30, -20,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30
    };

static int kingEvalEnd[64] = {
    300,  300,  300,  300,  300,  300,  300, 300,
    300,  250,  250,  250,  250,  250,  250, 300,
    300,  250,  150,  150,  150,  150,  250, 300,
    300,  250,  150,   50,   50,  150,  250, 300,
    300,  250,  150,   50,   50,  150,  250, 300, 
    300,  250,  150,  150,  150,  150,  250, 300,
    300,  250,  250,  250,  250,  250,  250, 300,
    300,  300,  300,  300,  300,  300,  300, 300
    };

static int diagonalShiftAmount[64] = {
    // needed for move generation
    // each diagonal has a different length, so the shift amount to get it into the last 8 bits depens on the square
     0,  1,  1,  3,  3,  3,  6,  6,
     6,  6, 10, 10, 10, 10, 10, 15,
    15, 15, 15, 15, 15, 21, 21, 21,
    21, 21, 21, 21, 28, 28, 28, 28,
    28, 28, 28, 28, 36, 36, 36, 36,
    36, 36, 36, 43, 43, 43, 43, 43,
    43, 49, 49, 49, 49, 49, 54, 54,
    54, 54, 58, 58, 58, 61, 61, 63,
};

// bottom left to top right diagonals
// the length of the diagonal a square is part of
static int diagonalLenR[64] = {
    0,1,2,3,4,5,6,7,
    1,2,3,4,5,6,7,6,
    2,3,4,5,6,7,6,5,
    3,4,5,6,7,6,5,4,
    4,5,6,7,6,5,4,3,
    5,6,7,6,5,4,3,2,
    6,7,6,5,4,3,2,1,
    7,6,5,4,3,2,1,0
};

// and the position of the square in the diagonal
static int positionInDiagonalR[64] = {
    0,1,2,3,4,5,6,7,
    0,1,2,3,4,5,6,6,
    0,1,2,3,4,5,5,5,
    0,1,2,3,4,4,4,4,
    0,1,2,3,3,3,3,3,
    0,1,2,2,2,2,2,2,
    0,1,1,1,1,1,1,1,
    0,0,0,0,0,0,0,0
};

// top left to bottom right diagonals
static int diagonalLenL[64] = {
    7,6,5,4,3,2,1,0,
    6,7,6,5,4,3,2,1,
    5,6,7,6,5,4,3,2,
    4,5,6,7,6,5,4,3,
    3,4,5,6,7,6,5,4,
    2,3,4,5,6,7,6,5,
    1,2,3,4,5,6,7,6,
    0,1,2,3,4,5,6,7
};

static int positionInDiagonalL[64] = {
    0,0,0,0,0,0,0,0,
    0,1,1,1,1,1,1,1,
    0,1,2,2,2,2,2,2,
    0,1,2,3,3,3,3,3,
    0,1,2,3,4,4,4,4,
    0,1,2,3,4,5,5,5,
    0,1,2,3,4,5,6,6,
    0,1,2,3,4,5,6,7
};

// STRUCTS

typedef struct Bitboards {
    // each bitboard represents a different piece type
    // each bit represents a square on the board
    // 1 means the piece is on that square, 0 means it is not
    uint64_t whitePawns;
    uint64_t whiteKnights;
    uint64_t whiteBishops;
    uint64_t whiteRooks;
    uint64_t whiteQueens;
    uint64_t whiteKing;

    uint64_t blackPawns;
    uint64_t blackKnights;
    uint64_t blackBishops;
    uint64_t blackRooks;
    uint64_t blackQueens;
    uint64_t blackKing;
    
    uint64_t whitePieces;
    uint64_t blackPieces;
    uint64_t allPieces;
    uint64_t allPieces90;
    uint64_t allPieces45R;
    uint64_t allPieces45L;
    
    uint64_t whiteAttacks;
    uint64_t blackAttacks;
    
    // castling flags and en passant square
    bool whiteCastleKingSide;
    bool whiteCastleQueenSide;
    
    bool blackCastleKingSide;
    bool blackCastleQueenSide;
    
    uint64_t enPassantSquare;
    // hash of the current position to use for transposition tables
    uint64_t hash;

} Bitboards;

typedef struct Move {
    // a move is represented by the starting square, ending square,
    // piece type and any other information needed to modify the bitboards accordingly
    uint8_t from;
    uint8_t to;
    uint8_t pieceType;
    uint8_t castle;
    uint8_t isEnPassantCapture;
    uint8_t createsEnPassant;
    uint8_t promotesTo;
} Move;

typedef struct HashEntry {
    // a hash entry is a position that has been evaluated before
    // it is used to avoid evaluating the same position multiple times
    uint64_t hash;
    int8_t depth;
    int value;
    uint8_t flag;
} HashEntry;

typedef struct BookEntry {
    uint64_t hash;
    Move* moves;
    int* occourences;
    uint8_t numMoves;
} BookEntry;

typedef struct Book {
    BookEntry* entries;
    int numEntries;
} Book;

typedef struct Table {
    HashEntry* entries;
    uint8_t numEntries;
} Table;

// GLOBAL VARIABLES

struct Bitboards bitboards = {
    0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0,
    0, 0,
    false, false,
    false, false,
    0, 0
};

// lookup tables for piece attack patterns
uint64_t knightAttacks[64];
uint64_t kingAttacks[64];
uint64_t rookRowAttacks[64][256];
uint64_t rookRowCaptures[64][256];
uint64_t rookFileAttacks[64][256];
uint64_t rookFileCaptures[64][256];
uint64_t bishopDiagonalAttacksR[64][256];
uint64_t bishopDiagonalCapturesR[64][256];
uint64_t bishopDiagonalAttacksL[64][256];
uint64_t bishopDiagonalCapturesL[64][256];

// tables used for hashing
uint64_t ZOBRIST_TABLE[64][13]; // 64 squares, 12 pieces and 1 en passant
uint64_t whiteToMove;
uint64_t castlingRights[4];

// one transposition table for the main search, one for the quiescence search
struct Table *transTable;
struct Table *quietTable;

// function call counters
int possibleCalls = 0;
int moveCalls = 0;
int evalCalls = 0;
int quiescenceCalls = 0;

int nodes = 0;
int visits = 0; // number of nodes visited after a specific move
int transpositions = 0;

int moveCount = 0;
int halfMoveCount = 0;

int lastfrom = -1; // maintained during the game to avoid repetition
int lastto = -1; // todo use a game state table with the hashes instead

int maxTime;
int maxDepth;

bool useBook = false;

// IN AND OUTPUT TRANSLATION

void fenToPosition(char* fen, int position[8][8]) {
    
    int rank = 0;  // Start from rank 8 and go down to rank 1
    int file = 0;  // Start from file a and go to file h

    // Loop through the FEN string
    for (int i = 0; fen[i] != '\0'; i++) {
        char c = fen[i];
        int piece;

        // Map FEN characters to piece values
        switch (c) {
            case 'p':
                piece = p;
                break;
            case 'n':
                piece = n;
                break;
            case 'b':
                piece = b;
                break;
            case 'r':
                piece = r;
                break;
            case 'q':
                piece = q;
                break;
            case 'k':
                piece = k;
                break;
            case 'P':
                piece = P;
                break;
            case 'N':
                piece = N;
                break;
            case 'B':
                piece = B;
                break;
            case 'R':
                piece = R;
                break;
            case 'Q':
                piece = Q;
                break;
            case 'K':
                piece = K;
                break;
            case '/':
                // End of rank, move to the next rank and reset file
                rank++;
                file = 0;
                continue;
            default:
                // If the character is a number, skip that many files
                file += c - '0';
                continue;
        }

        // Assign the piece value to the corresponding position in the array
        position[rank][file] = piece;
        file++;
    }
}

struct Move buildMove(char *move, struct Bitboards bitboards) {
    // make a useable move struct from a string
    struct Move m;
    int from = 7 - (move[0] - 'a') + (move[1] - '1') * 8;
    int to =  7 - (move[2] - 'a') + (move[3] - '1') * 8;
    // printf("from: %d  to: %d\n", from, to);
    int pieceType = 0;
    int castle = 0;
    int isEnPassantCapture = 0;
    int createsEnPassant = 0;
    char promotingChar;
    int promotesTo = 0;

    if (((bitboards.whitePawns & (1ULL << from)) != 0) || ((bitboards.blackPawns & (1ULL << from)) != 0)) {
        pieceType = 0;
        if (abs(from - to) == 16) {
            createsEnPassant = 1;
        } else if (bitboards.enPassantSquare & (1ULL << to)) {
            isEnPassantCapture = 1;
        } else if (to > 55 || to < 8) {
            printf("promote to: ");
            scanf("%s", &promotingChar);
            if (promotingChar == 'q') {
                promotesTo = 4;
            } else if (promotingChar == 'r') {
                promotesTo = 3;
            } else if (promotingChar == 'b') {
                promotesTo = 2;
            } else if (promotingChar == 'n') {
                promotesTo = 1;
            }
        }
    } else if (((bitboards.whiteKnights & (1ULL << from)) != 0) || ((bitboards.blackKnights & (1ULL << from)) != 0)) {
        pieceType = 1;
    } else if (((bitboards.whiteBishops & (1ULL << from)) != 0) || ((bitboards.blackBishops & (1ULL << from)) != 0)) {
        pieceType = 2;
    } else if (((bitboards.whiteRooks & (1ULL << from)) != 0) || ((bitboards.blackRooks & (1ULL << from)) != 0)) {
        pieceType = 3;
    } else if (((bitboards.whiteQueens & (1ULL << from)) != 0) || ((bitboards.blackQueens & (1ULL << from)) != 0)) {
        pieceType = 4;
    } else if (((bitboards.whiteKing & (1ULL << from)) != 0) || ((bitboards.blackKing & (1ULL << from)) != 0)) {
        pieceType = 5;
        if (abs(from - to) == 2) {
            if (from > to) {
                castle = 1;
            } else {
                castle = 2;
            }
        }
    }

    m.from = from;
    m.to = to;
    m.pieceType = pieceType;
    m.castle = castle;
    m.isEnPassantCapture = isEnPassantCapture;
    m.createsEnPassant = createsEnPassant;
    m.promotesTo = promotesTo;

    return m;
}

void printBoard(struct Bitboards boards, int sideToMove) {
    // print the current board state in a readable format
    char board[8][8] = {0};
    
    for (int i = 63; i >= 0; i--) {
        int row = i / 8;
        int col = i % 8;
        
        if (checkBit(boards.whitePawns, i)) {
            board[7 - row][7 - col] = 'P';
        } else if (checkBit(boards.blackPawns, i)) {
            board[7 - row][7 - col] = 'p';
        } else if (checkBit(boards.whiteKnights, i)) {
            board[7 - row][7 - col] = 'N';
        } else if (checkBit(boards.blackKnights, i)) {
            board[7 - row][7 - col] = 'n';
        } else if (checkBit(boards.whiteBishops, i)) {
            board[7 - row][7 - col] = 'B';
        } else if (checkBit(boards.blackBishops, i)) {
            board[7 - row][7 - col] = 'b';
        } else if (checkBit(boards.whiteRooks, i)) {
            board[7 - row][7 - col] = 'R';
        } else if (checkBit(boards.blackRooks, i)) {
            board[7 - row][7 - col] = 'r';
        } else if (checkBit(boards.whiteQueens, i)) {
            board[7 - row][7 - col] = 'Q';
        } else if (checkBit(boards.blackQueens, i)) {
            board[7 - row][7 - col] = 'q';
        } else if (checkBit(boards.whiteKing, i)) {
            board[7 - row][7 - col] = 'K';
        } else if (checkBit(boards.blackKing, i)) {
            board[7 - row][7 - col] = 'k';
        } else {
            board[7 - row][7 - col] = '.';
        }
    }
    
    // print the board
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            printf("%c ", board[i][j]);
        }
        printf("\n");
    }

    // print the FEN notation
    // todo unify this with the above code

    for (int i = 63; i >= 0; i--) {
        if (checkBit(boards.whitePawns, i)) {
            printf("P");
        } else if (checkBit(boards.blackPawns, i)) {
            printf("p");
        } else if (checkBit(boards.whiteKnights, i)) {
            printf("N");
        } else if (checkBit(boards.blackKnights, i)) {
            printf("n");
        } else if (checkBit(boards.whiteBishops, i)) {
            printf("B");
        } else if (checkBit(boards.blackBishops, i)) {
            printf("b");
        } else if (checkBit(boards.whiteRooks, i)) {
            printf("R");
        } else if (checkBit(boards.blackRooks, i)) {
            printf("r");
        } else if (checkBit(boards.whiteQueens, i)) {
            printf("Q");
        } else if (checkBit(boards.blackQueens, i)) {
            printf("q");
        } else if (checkBit(boards.whiteKing, i)) {
            printf("K");
        } else if (checkBit(boards.blackKing, i)) {
            printf("k");
        } else {
            int count = 0;
            while (!checkBit(boards.allPieces, i)) {
                count++;
                i--;
                if ((i + 1) % 8 == 0) {
                    break;
                }
            }
            printf("%d", count);
            i++;
        }
        if ((i % 8) == 0 && i != 0) {
            printf("/");
        }
    }

    printf(" %c ", sideToMove == 1 ? 'w' : 'b');

    if (boards.whiteCastleKingSide) {
        printf("K");
    }
    if (boards.whiteCastleQueenSide) {
        printf("Q");
    }
    if (boards.blackCastleKingSide) {
        printf("k");
    }
    if (boards.blackCastleQueenSide) {
        printf("q");
    }
    if (!boards.whiteCastleKingSide && !boards.whiteCastleQueenSide && !boards.blackCastleKingSide && !boards.blackCastleQueenSide) {
        printf("-");
    }

    if (boards.enPassantSquare) {
        printf(" %s", notation[lsb(boards.enPassantSquare)]);
    } else {
        printf(" -");
    }

    printf(" %d %d", halfMoveCount, moveCount % 2 == 0 ? (moveCount / 2) + 1 : (moveCount + 1) / 2);

    printf("\n\n");
}

void displayBoard(uint64_t bitboard) {
    // displays a single bitboard
    int bit = 63;
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            if (checkBit(bitboard, bit)) {
                printf("X ");
            } else {
                printf(". ");
            }
            bit--;
        }
        printf("\n");
    }
    printf("\n");
}

void updateFenClocks(struct Move move) {
    // update the clocks in the fen string
    moveCount++;
    if (move.pieceType == 0 || checkBit(bitboards.allPieces, move.to)) {
        halfMoveCount = 0;
    } else {
        halfMoveCount++;
    }

    if (halfMoveCount >= 100) {
        printf("draw by 50 move rule\n");
        exit(0);
    }
}

// INITIALIZATION FUNCTIONS

void initTransTables() {
    // allocate memory for the transposition tables and initialize them to 0
    transTable = malloc(TABLE_SIZE * sizeof(struct Table));
    memset(transTable, 0, TABLE_SIZE * sizeof(struct Table));
    if (transTable == NULL) {
        printf("Error: failed to allocate memory for transposition table\n");
        exit(1);
    }
    
    quietTable = malloc(TABLE_SIZE * sizeof(struct Table));
    memset(quietTable, 0, TABLE_SIZE * sizeof(struct Table));
    if (quietTable == NULL) {
        printf("Error: failed to allocate memory for quiescence transposition table\n");
        exit(1);
    }
}

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

uint64_t initBoardHash(struct Bitboards BITBOARDS, bool isWhiteToMove) {
    // initialize the board hash for the starting position
    uint64_t hash = 0;
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

void initBoards(int startPosition[8][8], bool isWhiteToMove, char* castle, char* enPas, int fiftyMove, int moveNum) {
    // initialize the bitboards and the move history from a FEN string
    halfMoveCount = fiftyMove;
    moveCount = isWhiteToMove ? moveNum * 2 - 1 : moveNum * 2;

    int piece;
    int bit = 63;

    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            piece = startPosition[row][col];
            if (piece != 0) {
                    bitboards.allPieces = setBit(bitboards.allPieces, bit);
                if (piece > 0) {
                    bitboards.blackPieces = setBit(bitboards.blackPieces, bit);
                } else {
                    bitboards.whitePieces = setBit(bitboards.whitePieces, bit);
                }
                if (piece == p) {
                    bitboards.blackPawns = setBit(bitboards.blackPawns, bit);}
                else if (piece == n) {
                    bitboards.blackKnights = setBit(bitboards.blackKnights, bit);}
                else if (piece == b) {
                    bitboards.blackBishops = setBit(bitboards.blackBishops, bit);}
                else if (piece == r) {
                    bitboards.blackRooks = setBit(bitboards.blackRooks, bit);}
                else if (piece == q) {
                    bitboards.blackQueens = setBit(bitboards.blackQueens, bit);}
                else if (piece == k) {
                    bitboards.blackKing = setBit(bitboards.blackKing, bit);}
                else if (piece == P) {
                    bitboards.whitePawns = setBit(bitboards.whitePawns, bit);}
                else if (piece == N) {
                    bitboards.whiteKnights = setBit(bitboards.whiteKnights, bit);}
                else if (piece == B) {
                    bitboards.whiteBishops = setBit(bitboards.whiteBishops, bit);}
                else if (piece == R) {
                    bitboards.whiteRooks = setBit(bitboards.whiteRooks, bit);}
                else if (piece == Q) {
                    bitboards.whiteQueens = setBit(bitboards.whiteQueens, bit);}
                else if (piece == K) {
                    bitboards.whiteKing = setBit(bitboards.whiteKing, bit);}
            }
            bit--;
        }
    }

    int z = 0;

    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            if (checkBit(bitboards.allPieces, z)) { 
                bitboards.allPieces90 = setBit(bitboards.allPieces90, rotated90[z]);
                bitboards.allPieces45R = setBit(bitboards.allPieces45R, rotated45R[z]);
                bitboards.allPieces45L = setBit(bitboards.allPieces45L, rotated45L[z]);
            }
            z++;
        }
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
        bitboards.enPassantSquare = setBit(bitboards.enPassantSquare, epSquare);
    }
    
    bitboards.hash = initBoardHash(bitboards, isWhiteToMove);
}

// precomputed move lookup tables
void initKnightAttacks() {
    for (int i = 0; i < 64; i++) {
        // knight attacks from each of the 64 squares
        // look at the "normal board" somewhere above for more clarity
        uint64_t knightboard = 0;
        if (i % 8 < 6 && i / 8 != 7) { // to prevent wrapping
            knightboard = setBit(knightboard, i+10);}
        if (i % 8 > 1 && i / 8 != 0) {
            knightboard = setBit(knightboard, i-10);}
        if (i % 8 != 7 && i / 8 < 6) {
            knightboard = setBit(knightboard, i+17);}
        if (i % 8 != 0 && i / 8 > 1) {
            knightboard = setBit(knightboard, i-17);}
        if (i % 8 != 0 && i / 8 < 6) {
            knightboard = setBit(knightboard, i+15);}
        if (i % 8 != 7 && i / 8 > 1) {
            knightboard = setBit(knightboard, i-15);}
        if (i % 8 > 1 && i / 8 != 7) {
            knightboard = setBit(knightboard, i+6);}
        if (i % 8 < 6 && i / 8 != 0) {
            knightboard = setBit(knightboard, i-6);}

        knightAttacks[i] = knightboard;
    }
}

void initKingAttacks() {
    for (int i = 0; i < 64; i++) {
        uint64_t kingboard = 0;
        if (i % 8 != 7) {
            kingboard = setBit(kingboard, i+1);}
        if (i % 8 != 0 && i / 8 != 7) {
            kingboard = setBit(kingboard, i+7);}
        if (i / 8 != 7) {
            kingboard = setBit(kingboard, i+8);}
        if (i % 8 != 7 && i / 8 != 7) {
            kingboard = setBit(kingboard, i+9);}
        if (i % 8 != 0) {
            kingboard = setBit(kingboard, i-1);}
        if (i % 8 != 7 && i / 8 != 0) {
            kingboard = setBit(kingboard, i-7);}
        if (i / 8 != 0) {
            kingboard = setBit(kingboard, i-8);}
        if (i % 8 != 0 && i / 8 != 0) {
            kingboard = setBit(kingboard, i-9);}

        kingAttacks[i] = kingboard;
    }
}

void initRookRowAttacks() {
    for (int i = 0; i < 64; i++) {
        // for each square
        for (int j = 0; j < 256; j++) {
            // there is an 8 bit number (j) representing the occuancy of the rank
            uint64_t rowAttacks = 0;
            uint64_t rowCaptures = 0;
            int dx = -1;
            for (int t = 0; t < 2; t++) { // generate rook moves left and right on that rank
                int target = i % 8 + dx;  // start at the square to the left or right of the original square
                if (!((i%8 == 7 && dx > 0) || (i%8 == 0 && dx < 0))) { 
                    // if we're not already at the edge of the board
                    while (true) {
                        if ((target == 7 && dx > 0) || (target == 0 && dx < 0) || (checkBit(j, target))) {
                            // stop if we're at the edge of the board or hit a piece
                            // it doesn't matter if the piece is friendly or not, we can filter that out later
                            rowAttacks = setBit(rowAttacks, target);
                            if (checkBit(j, target)) {
                                rowCaptures = setBit(rowCaptures, target);
                            }
                            break;
                        } else {
                            // otherwise keep going
                            rowAttacks = setBit(rowAttacks, target);
                            target += dx;
                        }
                    }
                }
                if (t == 0) {dx = 1;} // switch to the other direction
            }
            rowAttacks <<= ((i/8) * 8); // shift the attacked squares back to the rank of the original square
            rowCaptures <<= ((i/8) * 8);
            rookRowAttacks[i][j] = rowAttacks;
            rookRowCaptures[i][j] = rowCaptures;
        }
    }
}

void initRookFileAttacks() {
    // same as above but for files using rotated boards
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 256; j++) {
            uint64_t fileAttacks = 0;
            uint64_t fileCaptures = 0;
            int dx = 1;
            for (int t = 0; t < 2; t++) {
                int target = rotated90[i] % 8 + dx;
                if (!((rotated90[i]%8 == 7 && dx > 0) || (rotated90[i]%8 == 0 && dx < 0))) {
                    while (true) {
                        if ((target == 7 && dx > 0) || (target == 0 && dx < 0) || (checkBit(j, target))) {
                            fileAttacks = setBit(fileAttacks, i%8 + 8 * (7 - target));
                            if (checkBit(j, target)) {
                                fileCaptures = setBit(fileCaptures, i%8 + 8 * (7 - target));
                            }
                            break;
                        } else {
                            fileAttacks = setBit(fileAttacks, i%8 + 8 * (7 - target));
                            target += dx;
                        }
                    }}
                if (t == 0) {dx = -1;}
            }
            rookFileAttacks[i][j] = fileAttacks;
            rookFileCaptures[i][j] = fileCaptures;
        }
    }
}

void initBishopDiagonalAttacksR() {
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 256; j++) {
            uint64_t diagonalAttacksR = 0;
            uint64_t diagonalCapturesR = 0;
            int dx = 1;
            for (int t = 0; t < 2; t++) {
                int target;

                target = positionInDiagonalR[i] + dx;
                
                if (!((positionInDiagonalR[i] == diagonalLenR[i] && dx > 0) || (positionInDiagonalR[i] == 0 && dx < 0))) {
                    while (true) {
                        if ((target >= diagonalLenR[i] && dx > 0) || (target == 0 && dx < 0) || (checkBit(j, target))) {
                            diagonalAttacksR = setBit(diagonalAttacksR, i + (positionInDiagonalR[i] - target)*7);
                            if (checkBit(j, target)) {
                                diagonalCapturesR = setBit(diagonalCapturesR, i + (positionInDiagonalR[i] - target)*7);
                            }
                            break;
                        } else {
                            diagonalAttacksR = setBit(diagonalAttacksR, i + (positionInDiagonalR[i] - target)*7);
                            target += dx;
                        }
                    }}
                if (t == 0) {dx = -1;}
            }
            bishopDiagonalAttacksR[i][j] = diagonalAttacksR;
            bishopDiagonalCapturesR[i][j] = diagonalCapturesR;
        }
    }
}

void initBishopDiagonalAttacksL() {
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 256; j++) {
            uint64_t diagonalAttacksL = 0;
            uint64_t diagonalCapturesL = 0;
            int dx = 1;
            for (int t = 0; t < 2; t++) {
                int target;

                target = positionInDiagonalL[i] + dx;
                
                if (!((positionInDiagonalL[i] == diagonalLenL[i] && dx > 0) || (positionInDiagonalL[i] == 0 && dx < 0))) {
                    while (true) {
                        if ((target == diagonalLenL[i] && dx > 0) || (target == 0 && dx < 0) || (checkBit(j, target))) {
                            diagonalAttacksL = setBit(diagonalAttacksL, i + (target - positionInDiagonalL[i])*9);
                            if (checkBit(j, target)) {
                                diagonalCapturesL = setBit(diagonalCapturesL, i + (target - positionInDiagonalL[i])*9);
                            }
                            break;
                        } else {
                            diagonalAttacksL = setBit(diagonalAttacksL, i + (target - positionInDiagonalL[i])*9);
                            target += dx;
                        }
                    }}
                if (t == 0) {dx = -1;}
            }
            bishopDiagonalAttacksL[i][j] = diagonalAttacksL;
            bishopDiagonalCapturesL[i][j] = diagonalCapturesL;
        }
    }
}

// MOVE GENERATION

uint64_t generateRookMoves(int rookIndex, uint64_t occupied, uint64_t occupied90) {
    // shift both the file and rank into the last 8 bits of the number
    // and use that as an index into the precomputed lookup table
    // this works since a file or rank only has 8 squares (or bits)
    return  rookRowAttacks[rookIndex][(occupied >> (rookIndex / 8) * 8) & 255] | 
            rookFileAttacks[rookIndex][(occupied90 >> (rookIndex % 8) * 8) & 255];
}

uint64_t generateRookAttacks(int rookIndex, uint64_t occupied, uint64_t occupied90, uint64_t enemyPieces) {
    // same as normal moves but only keep the attacks that hit enemy pieces
    return  (rookRowCaptures[rookIndex][(occupied >> (rookIndex / 8) * 8) & 255] | 
            rookFileCaptures[rookIndex][(occupied90 >> (rookIndex % 8) * 8) & 255]) & enemyPieces;
}

uint64_t generateBishopMoves(int bishopIndex, uint64_t occupied45R, uint64_t occupied45L) {
    return  bishopDiagonalAttacksR[bishopIndex][255 & (occupied45R >> diagonalShiftAmount[rotated45R[bishopIndex]])] |
            bishopDiagonalAttacksL[bishopIndex][255 & (occupied45L >> diagonalShiftAmount[rotated45L[bishopIndex]])];
}

uint64_t generateBishopAttacks(int bishopIndex, uint64_t occupied45R, uint64_t occupied45L, uint64_t enemyPieces) {
    return  (bishopDiagonalCapturesR[bishopIndex][255 & (occupied45R >> diagonalShiftAmount[rotated45R[bishopIndex]])] |
            bishopDiagonalCapturesL[bishopIndex][255 & (occupied45L >> diagonalShiftAmount[rotated45L[bishopIndex]])]) & enemyPieces;
}

uint64_t generatePawnMoves(int pawnIndex, uint64_t occupied, uint64_t enemyPieces, bool isWhitePawn, uint64_t enPassantSquare) {
    // todo use precomputed table instead
    uint64_t pawn = 0;
    pawn = setBit(pawn, pawnIndex);

    if (isWhitePawn) {
        return (
                    (((pawn << 7) & ~leftmostFileMask) |    // left capture - leftmost file mask to prevent wraparound
                    ((pawn << 9) & ~rightmostFileMask)) &   // right capture
                    (enemyPieces | enPassantSquare)         // can only capture enemy pieces or en passant square
                ) | (
                    ((pawn << 8) & ~occupied) |             // single push
                    (((pawn & whitePawnStartRank) && ((pawn << 8) & ~occupied)) ? ((pawn << 16) & ~occupied) : 0)
                                                            // double push
                );
    } else {
        return (
                    (((pawn >> 7) & ~rightmostFileMask) | 
                    ((pawn >> 9) & ~leftmostFileMask)) & 
                    (enemyPieces | enPassantSquare)
                ) | (
                    ((pawn >> 8) & ~occupied) |
                    (((pawn & blackPawnStartRank) && ((pawn >> 8) & ~occupied)) ? ((pawn >> 16) & ~occupied) : 0)
                );
    }
}

uint64_t generatePawnAttacks(int pawnIndex, bool isWhitePawn) {
    uint64_t pawn = 0;
    pawn = setBit(pawn, pawnIndex);
    if (isWhitePawn) {
        return ((pawn << 7) & ~leftmostFileMask) | ((pawn << 9) & ~rightmostFileMask);
    } else {
        return ((pawn >> 7) & ~rightmostFileMask) | ((pawn >> 9) & ~leftmostFileMask);
    }
}

void pushMove(struct Move **moveArray, struct Move move, int *index, int *capacity) {
    // function to add a move to the move array
    // todo: run some tests to see which capacity is optimal
    if (*index == *capacity) {
        *capacity *= 2;
        *moveArray = realloc(*moveArray, sizeof(struct Move) * (*capacity));
        if (*moveArray == NULL) {
            printf("\nError: unable to reallocate memory\n");
            exit(1);
        }
    }
    (*moveArray)[*index] = move;
    *index += 1;
}
                        
struct Move* possiblemoves(
        bool isWhiteToMove,
        uint64_t occupied, uint64_t occupied90, uint64_t occupied45R, uint64_t occupied45L, uint64_t epSquare, uint64_t myPieces, uint64_t enemyPieces, 
        uint64_t pawns, uint64_t knights, uint64_t bishops, uint64_t rooks, uint64_t queens, uint64_t king, 
        bool castleQueen, bool castleKing
    ) {

    // function to generate all possible moves for a given player and board state
    possibleCalls++;

    int capacity = 1;
    struct Move *possible = malloc(sizeof(struct Move));

    if (possible == NULL) {
        printf("\nError: failed to allocate memory for possible moves\n");
        exit(1);
    }

    int count = 1;

    uint64_t kingMoves;
    uint8_t kingIndex = 0;

    if (bitCount(king) != 0) {
        kingIndex = lsb(king);
        kingMoves = kingAttacks[kingIndex] & ~myPieces; // exclude own pieces
    } else {
        // without the king some of the other functions will break
        printf("\nError: king not found\n");
        exit(1);
    }

    while (kingMoves) {
        // loop through all possible king move bits and add them to the move list
        struct Move move = {from: kingIndex, to: popLsb(&kingMoves), pieceType: 5, castle: 0, isEnPassantCapture: 0, createsEnPassant: 0, promotesTo: 0};
        pushMove(&possible, move, &count, &capacity);
    }
    // castling is only available when the flags are set and the squares are empty, check detection is done in the search tree
    if (castleKing && ((!checkBit(occupied, kingIndex-1)) && (!checkBit(occupied, kingIndex-2)) && (checkBit(rooks, kingIndex-3)))) {
        struct Move move = {from: kingIndex, to: kingIndex-2, castle: 1, pieceType: 5, isEnPassantCapture: 0, createsEnPassant: 0, promotesTo: 0};
        pushMove(&possible, move, &count, &capacity);
    }
    if (castleQueen && (!checkBit(occupied, kingIndex+1) && (!checkBit(occupied, kingIndex+2)) && (!checkBit(occupied, kingIndex+3)) && (checkBit(rooks, kingIndex+4)))) {
        struct Move move = {from: kingIndex, to: kingIndex+2, castle: 2, pieceType: 5, isEnPassantCapture: 0, createsEnPassant: 0, promotesTo: 0};
        pushMove(&possible, move, &count, &capacity);
    }

    uint64_t knightMoves;

    while (knights) {
        // for each knight on the board, generate all its possible moves
        uint8_t knightIndex = popLsb(&knights);
        knightMoves = knightAttacks[knightIndex] & ~myPieces;
        while (knightMoves) {
            // loop through all possible knight move bits and add them to the move list
            struct Move move = {from: knightIndex, to: popLsb(&knightMoves), pieceType: 1, castle: 0, isEnPassantCapture: 0, createsEnPassant: 0, promotesTo: 0};
            pushMove(&possible, move, &count, &capacity);
        }
    }

    uint64_t bishopMoves;

    while (bishops) {
        uint8_t bishopIndex = popLsb(&bishops);
        bishopMoves = generateBishopMoves(bishopIndex, occupied45R, occupied45L) & ~myPieces;
        while (bishopMoves) {
            struct Move move = {from: bishopIndex, to: popLsb(&bishopMoves), pieceType: 2, castle: 0, isEnPassantCapture: 0, createsEnPassant: 0, promotesTo: 0};
            pushMove(&possible, move, &count, &capacity);
        }
    }

    uint64_t rookMoves;

    while (rooks) {
        uint8_t rookIndex = popLsb(&rooks);
        rookMoves = generateRookMoves(rookIndex, occupied, occupied90) & ~myPieces;
        while (rookMoves) {
            struct Move move = {from: rookIndex, to: popLsb(&rookMoves), pieceType: 3, castle: 0, isEnPassantCapture: 0, createsEnPassant: 0, promotesTo: 0};
            pushMove(&possible, move, &count, &capacity);
        }
    }

    uint64_t queenMoves;
    uint64_t otherQueenMoves;
    // queens are a combination of rooks and bishops
    while (queens) {
        int queenIndex = popLsb(&queens);
        queenMoves = generateBishopMoves(queenIndex, occupied45R, occupied45L) & ~myPieces;
        otherQueenMoves = generateRookMoves(queenIndex, occupied, occupied90) & ~myPieces;
        queenMoves |= otherQueenMoves;
        while (queenMoves) {
            struct Move move = {from: queenIndex, to: popLsb(&queenMoves), pieceType: 4, castle: 0, isEnPassantCapture: 0, createsEnPassant: 0, promotesTo: 0};
            pushMove(&possible, move, &count, &capacity);
        }
    }

    uint64_t pawnMoves;

    while (pawns) {
        uint8_t pawnIndex = popLsb(&pawns);
        pawnMoves = generatePawnMoves(pawnIndex, occupied, enemyPieces, isWhiteToMove, epSquare);
        while (pawnMoves) {
            uint8_t bit = popLsb(&pawnMoves);
            if (bit == lsb(epSquare)) {
                // if the pawn is moving to the en passant square, it is an en passant capture
                struct Move move = {from: pawnIndex, to: bit, isEnPassantCapture: 1, pieceType: 0, castle: 0, createsEnPassant: 0, promotesTo: 0};
                pushMove(&possible, move, &count, &capacity);
            } else if (abs(pawnIndex-bit) == 16) {
                // if the pawn is moving two squares, it is creating an en passant square
                struct Move move = {from: pawnIndex, to: bit, createsEnPassant: 1, pieceType: 0, castle: 0, isEnPassantCapture: 0, promotesTo: 0};
                pushMove(&possible, move, &count, &capacity);
            } else if (bit/8 == 7 || bit/8 == 0) {
                // if the pawn is moving to the last rank, it is a promotion
                struct Move move = {from: pawnIndex, to: bit, promotesTo: 4, pieceType: 0, castle: 0, isEnPassantCapture: 0, createsEnPassant: 0};
                pushMove(&possible, move, &count, &capacity);
                struct Move move1 = {from: pawnIndex, to: bit, promotesTo: 3, pieceType: 0, castle: 0, isEnPassantCapture: 0, createsEnPassant: 0};
                pushMove(&possible, move1, &count, &capacity);
                struct Move move2 = {from: pawnIndex, to: bit, promotesTo: 2, pieceType: 0, castle: 0, isEnPassantCapture: 0, createsEnPassant: 0};
                pushMove(&possible, move2, &count, &capacity);
                struct Move move3 = {from: pawnIndex, to: bit, promotesTo: 1, pieceType: 0, castle: 0, isEnPassantCapture: 0, createsEnPassant: 0};
                pushMove(&possible, move3, &count, &capacity);
            } else {
                // otherwise it is a normal pawn move
                struct Move move = {from: pawnIndex, to: bit, pieceType: 0, castle: 0, isEnPassantCapture: 0, createsEnPassant: 0, promotesTo: 0};
                pushMove(&possible, move, &count, &capacity);
            }
        }
    }

    // add an extra move at the start of the array to indicate the number of moves in the array as its from value
    // this is used to be able to iterate through the array without knowing the size
    struct Move moveAmount = {from: count, to: 0, pieceType: 0, castle: 0, isEnPassantCapture: 0, createsEnPassant: 0, promotesTo: 0};
    possible[0] = moveAmount;

    return possible;
}

struct Move* possiblecaptures(
    bool isWhiteToMove,
    uint64_t occupied, uint64_t occupied90, uint64_t occupied45R, uint64_t occupied45L, uint64_t epSquare, uint64_t myPieces, uint64_t enemyPieces, 
    uint64_t pawns, uint64_t knights, uint64_t bishops, uint64_t rooks, uint64_t queens, uint64_t king, 
    bool castleQueen, bool castleKing
    ) {

    // just like possiblemoves, but only returns captures
    // this is used to generate the quiescence search tree
    possibleCalls++;
    int capacity = 1;
    struct Move *possibleCaptures = malloc(sizeof(struct Move));

    if (possibleCaptures == NULL) {
        printf("\nError: failed to allocate memory for possibleCaptures\n");
        exit(1);
    }

    int count = 1;

    uint64_t kingMoves;
    uint8_t kingIndex = 0;

    if (bitCount(king) != 0) {
        kingIndex = lsb(king);
        kingMoves = kingAttacks[kingIndex] & enemyPieces; // this time we only want to look at moves that capture enemy pieces
    } else {
        printf("\nError: king not found\n");
        exit(1);
    }

    while (kingMoves) {
        struct Move move = {from: kingIndex, to: popLsb(&kingMoves), pieceType: 5, castle: 0, isEnPassantCapture: 0, createsEnPassant: 0, promotesTo: 0};
        pushMove(&possibleCaptures, move, &count, &capacity);
    }

    uint64_t knightMoves;

    while (knights) {
        uint8_t knightIndex = popLsb(&knights);
        knightMoves = knightAttacks[knightIndex] & enemyPieces;
        while (knightMoves) {
            struct Move move = {from: knightIndex, to: popLsb(&knightMoves), pieceType: 1, castle: 0, isEnPassantCapture: 0, createsEnPassant: 0, promotesTo: 0};
            pushMove(&possibleCaptures, move, &count, &capacity);
        }
    }
    
    uint64_t bishopMoves;

    while (bishops) {
        uint8_t bishopIndex = popLsb(&bishops);
        bishopMoves = generateBishopAttacks(bishopIndex, occupied45R, occupied45L, enemyPieces);
        while (bishopMoves) {
            struct Move move = {from: bishopIndex, to: popLsb(&bishopMoves), pieceType: 2, castle: 0, isEnPassantCapture: 0, createsEnPassant: 0, promotesTo: 0};
            pushMove(&possibleCaptures, move, &count, &capacity);
        }
    }

    uint64_t rookMoves;

    while (rooks) {
        uint8_t rookIndex = popLsb(&rooks);
        rookMoves = generateRookAttacks(rookIndex, occupied, occupied90, enemyPieces);
        while (rookMoves) {
            struct Move move = {from: rookIndex, to: popLsb(&rookMoves), pieceType: 3, castle: 0, isEnPassantCapture: 0, createsEnPassant: 0, promotesTo: 0};
            pushMove(&possibleCaptures, move, &count, &capacity);
        }
    }

    uint64_t queenMoves;
    uint64_t otherQueenMoves;

    while (queens) {
        uint8_t queenIndex = popLsb(&queens);
        queenMoves = generateBishopAttacks(queenIndex, occupied45R, occupied45L, enemyPieces);
        otherQueenMoves = generateRookAttacks(queenIndex, occupied, occupied90, enemyPieces);
        queenMoves |= otherQueenMoves;
        while (queenMoves) {
            struct Move move = {from: queenIndex, to: popLsb(&queenMoves), pieceType: 4, castle: 0, isEnPassantCapture: 0, createsEnPassant: 0, promotesTo: 0};
            pushMove(&possibleCaptures, move, &count, &capacity);
        }
    }

    uint64_t pawnMoves;

    while (pawns) {
        uint8_t pawnIndex = popLsb(&pawns);
        pawnMoves = generatePawnAttacks(pawnIndex, isWhiteToMove) & (enemyPieces | epSquare);
        while (pawnMoves) {
            uint8_t bit = popLsb(&pawnMoves);
            if (bit == lsb(epSquare)) {
                struct Move move = {from: pawnIndex, to: bit, isEnPassantCapture: 1, pieceType: 0, castle: 0, createsEnPassant: 0, promotesTo: 0};
                pushMove(&possibleCaptures, move, &count, &capacity);                
            } else {
                struct Move move = {from: pawnIndex, to: bit, pieceType: 0, castle: 0, isEnPassantCapture: 0, createsEnPassant: 0, promotesTo: 0};
                pushMove(&possibleCaptures, move, &count, &capacity);                
            }
        }
    }

    struct Move moveAmount = {from: count, to: 0, pieceType: 0, castle: 0, isEnPassantCapture: 0, createsEnPassant: 0, promotesTo: 0};
    possibleCaptures[0] = moveAmount;

    return possibleCaptures;
}

// MOVE MAKING AND HASH UPDATING

struct Bitboards doMove(struct Move move, struct Bitboards bitboards, bool isWhiteMove) {
    // todo: maybe use pointers to the bitboards and undo the move later
    moveCalls++;
    // yes this looks long, but there are many cases to consider
    // set the bit where it's going, the occupancy bitboards are always affected
    bitboards.allPieces = setBit(bitboards.allPieces, move.to);
    bitboards.allPieces90 = setBit(bitboards.allPieces90, rotated90[move.to]);
    bitboards.allPieces45R = setBit(bitboards.allPieces45R, rotated45R[move.to]);
    bitboards.allPieces45L = setBit(bitboards.allPieces45L, rotated45L[move.to]);

    // remove the bit from where it was
    bitboards.allPieces = unsetBit(bitboards.allPieces, move.from);
    bitboards.allPieces90 = unsetBit(bitboards.allPieces90, rotated90[move.from]);
    bitboards.allPieces45R = unsetBit(bitboards.allPieces45R, rotated45R[move.from]);
    bitboards.allPieces45L = unsetBit(bitboards.allPieces45L, rotated45L[move.from]);

    // update the hash
    bitboards.hash ^= whiteToMove;

    // now for the specific pieces
    if (isWhiteMove) {
        if (move.isEnPassantCapture) {
            // remove the captured pawn
            bitboards.blackPawns = unsetBit(bitboards.blackPawns, (move.to - 8));
            bitboards.blackPieces = unsetBit(bitboards.blackPieces, (move.to - 8));
            bitboards.allPieces = unsetBit(bitboards.allPieces, (move.to - 8));
            bitboards.allPieces90 = unsetBit(bitboards.allPieces90, rotated90[move.to - 8]);
            bitboards.allPieces45R = unsetBit(bitboards.allPieces45R, rotated45R[move.to - 8]);
            bitboards.allPieces45L = unsetBit(bitboards.allPieces45L, rotated45L[move.to - 8]);
            // update the pawn hash
            bitboards.hash ^= ZOBRIST_TABLE[(move.to - 8)][6];

        } else if (checkBit(bitboards.blackPieces, move.to)) {
            // if the piece captures a piece
            // here we dont need to remove the bit from the occupied squares, because the capturing piece is already there
            bitboards.blackPieces = unsetBit(bitboards.blackPieces, move.to);

            if (checkBit(bitboards.blackPawns, move.to)) {
                bitboards.blackPawns = unsetBit(bitboards.blackPawns, move.to);
                bitboards.hash ^= ZOBRIST_TABLE[move.to][6];
    
            } else if (checkBit(bitboards.blackBishops, move.to)) {
                bitboards.blackBishops = unsetBit(bitboards.blackBishops, move.to);
                bitboards.hash ^= ZOBRIST_TABLE[move.to][9];
    
            } else if (checkBit(bitboards.blackKnights, move.to)) {
                bitboards.blackKnights = unsetBit(bitboards.blackKnights, move.to);
                bitboards.hash ^= ZOBRIST_TABLE[move.to][8];
    
            } else if (checkBit(bitboards.blackRooks, move.to)) {
                bitboards.blackRooks = unsetBit(bitboards.blackRooks, move.to);
                bitboards.hash ^= ZOBRIST_TABLE[move.to][7];
    
            } else if (checkBit(bitboards.blackQueens, move.to)) {
                bitboards.blackQueens = unsetBit(bitboards.blackQueens, move.to);
                bitboards.hash ^= ZOBRIST_TABLE[move.to][10];
    
            } else if (checkBit(bitboards.blackKing, move.to)) {
                bitboards.blackKing = unsetBit(bitboards.blackKing, move.to);
                // in theory this should never happen because the search will stop if one player has no moves
                bitboards.hash ^= ZOBRIST_TABLE[move.to][11];
            }
            // remove the captured bit from all the boards its part of
        }
        
        bitboards.whitePieces = setBit(bitboards.whitePieces, move.to);
        bitboards.whitePieces = unsetBit(bitboards.whitePieces, move.from);

        if (move.pieceType == 0) { // pawn move
            bitboards.whitePawns = unsetBit(bitboards.whitePawns, move.from);
            bitboards.hash ^= ZOBRIST_TABLE[move.from][0];
            if (move.to >= 56) {
                // pawn promotion
                if (move.promotesTo == 4) {
                    bitboards.whiteQueens = setBit(bitboards.whiteQueens, move.to);
                    bitboards.hash ^= ZOBRIST_TABLE[move.to][4];
                } else if (move.promotesTo == 3) {
                    bitboards.whiteRooks = setBit(bitboards.whiteRooks, move.to);
                    bitboards.hash ^= ZOBRIST_TABLE[move.to][1];
                } else if (move.promotesTo == 2) {
                    bitboards.whiteBishops = setBit(bitboards.whiteBishops, move.to);
                    bitboards.hash ^= ZOBRIST_TABLE[move.to][3];
                } else if (move.promotesTo == 1) {
                    bitboards.whiteKnights = setBit(bitboards.whiteKnights, move.to);
                    bitboards.hash ^= ZOBRIST_TABLE[move.to][2];
                }
            } else {
                bitboards.whitePawns = setBit(bitboards.whitePawns, move.to);
                bitboards.hash ^= ZOBRIST_TABLE[move.to][0];
            }
        }
        else if (move.pieceType == 1) { // knight move
            bitboards.whiteKnights = setBit(bitboards.whiteKnights, move.to);
            bitboards.hash ^= ZOBRIST_TABLE[move.to][2];
            bitboards.whiteKnights = unsetBit(bitboards.whiteKnights, move.from);
            bitboards.hash ^= ZOBRIST_TABLE[move.from][2];

        } else if (move.pieceType == 2) { // bishop move
            bitboards.whiteBishops = setBit(bitboards.whiteBishops, move.to);
            bitboards.hash ^= ZOBRIST_TABLE[move.to][3];
            bitboards.whiteBishops = unsetBit(bitboards.whiteBishops, move.from);
            bitboards.hash ^= ZOBRIST_TABLE[move.from][3];

        } else if (move.pieceType == 3) { // rook move
            bitboards.whiteRooks = setBit(bitboards.whiteRooks, move.to);
            bitboards.hash ^= ZOBRIST_TABLE[move.to][1];
            bitboards.whiteRooks = unsetBit(bitboards.whiteRooks, move.from);
            bitboards.hash ^= ZOBRIST_TABLE[move.from][1];
            if (bitboards.whiteCastleQueenSide && move.from == 7) {
                // if the rook moves from the a1 square, then the white queen side castle is no longer possible
                bitboards.whiteCastleQueenSide = false;
                bitboards.hash ^= castlingRights[3];
            } else if (bitboards.whiteCastleKingSide && move.from == 0) {
                bitboards.whiteCastleKingSide = false;
                bitboards.hash ^= castlingRights[2];
            }
        } else if (move.pieceType == 4) { // queen move
            bitboards.whiteQueens = setBit(bitboards.whiteQueens, move.to);
            bitboards.hash ^= ZOBRIST_TABLE[move.to][4];
            bitboards.whiteQueens = unsetBit(bitboards.whiteQueens, move.from);
            bitboards.hash ^= ZOBRIST_TABLE[move.from][4];

        } else if (move.pieceType == 5) { // king move
            bitboards.whiteKing = setBit(bitboards.whiteKing, move.to);
            bitboards.hash ^= ZOBRIST_TABLE[move.to][5];
            bitboards.whiteKing = unsetBit(bitboards.whiteKing, move.from);
            bitboards.hash ^= ZOBRIST_TABLE[move.from][5];
            if (move.castle) {
                int s;
                int o;
                if (move.castle == 1) {s = 0; o = 2;} else {s = 7; o = 4;}
                    // put the rook on its new sqaure
                    bitboards.allPieces = unsetBit(bitboards.allPieces, s);
                    bitboards.allPieces90 = unsetBit(bitboards.allPieces90, rotated90[s]);
                    bitboards.allPieces45R = unsetBit(bitboards.allPieces45R, rotated45R[s]);
                    bitboards.allPieces45L = unsetBit(bitboards.allPieces45L, rotated45L[s]);
                    bitboards.whitePieces = unsetBit(bitboards.whitePieces, s);
                    bitboards.whiteRooks = unsetBit(bitboards.whiteRooks, s);
                    bitboards.hash ^= ZOBRIST_TABLE[s][1];

                    bitboards.allPieces = setBit(bitboards.allPieces, o);
                    bitboards.allPieces90 = setBit(bitboards.allPieces90, rotated90[o]);
                    bitboards.allPieces45R = setBit(bitboards.allPieces45R, rotated45R[o]);
                    bitboards.allPieces45L = setBit(bitboards.allPieces45L, rotated45L[o]);
                    bitboards.whitePieces = setBit(bitboards.whitePieces, o);
                    bitboards.whiteRooks = setBit(bitboards.whiteRooks, o);
                    bitboards.hash ^= ZOBRIST_TABLE[o][1];
                }
            if (bitboards.whiteCastleQueenSide) {
                bitboards.whiteCastleQueenSide = false;
                bitboards.hash ^= castlingRights[3];
            }
            if (bitboards.whiteCastleKingSide) {
                bitboards.whiteCastleKingSide = false;
                bitboards.hash ^= castlingRights[2];
            }
        }
        
        if (bitboards.enPassantSquare) {
            // if there was an en passant square, remove it from the hash
            bitboards.hash ^= ZOBRIST_TABLE[lsb(bitboards.enPassantSquare)][12];
        }

        bitboards.enPassantSquare = 0;

        if (move.createsEnPassant && (checkBit(bitboards.blackPawns, move.to + 1) || checkBit(bitboards.blackPawns, move.to - 1))) {
            // if the move creates an en passant square and it it captureable, add it to the hash and set the bit
            bitboards.enPassantSquare = setBit(bitboards.enPassantSquare, move.to - 8);
            bitboards.hash ^= ZOBRIST_TABLE[move.to - 8][12];
        }

    } else {
        // Black move
        if (move.isEnPassantCapture) {
            bitboards.whitePawns = unsetBit(bitboards.whitePawns, (move.to + 8));
            bitboards.whitePieces = unsetBit(bitboards.whitePieces, (move.to + 8));
            bitboards.allPieces = unsetBit(bitboards.allPieces, (move.to + 8));
            bitboards.allPieces90 = unsetBit(bitboards.allPieces90, rotated90[move.to + 8]);
            bitboards.allPieces45R = unsetBit(bitboards.allPieces45R, rotated45R[move.to + 8]);
            bitboards.allPieces45L = unsetBit(bitboards.allPieces45L, rotated45L[move.to + 8]);
            bitboards.hash ^= ZOBRIST_TABLE[move.to + 8][0];

        } else if (checkBit(bitboards.whitePieces, move.to)) {
            bitboards.whitePieces = unsetBit(bitboards.whitePieces, move.to);

            if (checkBit(bitboards.whitePawns, move.to)) {
                bitboards.whitePawns = unsetBit(bitboards.whitePawns, move.to);
                bitboards.hash ^= ZOBRIST_TABLE[move.to][0];
    
            } else if (checkBit(bitboards.whiteBishops, move.to)) {
                bitboards.whiteBishops = unsetBit(bitboards.whiteBishops, move.to);
                bitboards.hash ^= ZOBRIST_TABLE[move.to][3];
    
            } else if (checkBit(bitboards.whiteKnights, move.to)) {
                bitboards.whiteKnights = unsetBit(bitboards.whiteKnights, move.to);
                bitboards.hash ^= ZOBRIST_TABLE[move.to][2];
    
            } else if (checkBit(bitboards.whiteRooks, move.to)) {
                bitboards.whiteRooks = unsetBit(bitboards.whiteRooks, move.to);
                bitboards.hash ^= ZOBRIST_TABLE[move.to][1];
    
            } else if (checkBit(bitboards.whiteQueens, move.to)) {
                bitboards.whiteQueens = unsetBit(bitboards.whiteQueens, move.to);
                bitboards.hash ^= ZOBRIST_TABLE[move.to][4];
    
            } else if (checkBit(bitboards.whiteKing, move.to)) {
                bitboards.whiteKing = unsetBit(bitboards.whiteKing, move.to);
                bitboards.hash ^= ZOBRIST_TABLE[move.to][5];
            }
        }
        
        bitboards.blackPieces = setBit(bitboards.blackPieces, move.to);
        bitboards.blackPieces = unsetBit(bitboards.blackPieces, move.from);

        if (move.pieceType == 0) {
            bitboards.blackPawns = unsetBit(bitboards.blackPawns, move.from);
            bitboards.hash ^= ZOBRIST_TABLE[move.from][6];
            if (move.to <= 7) {
                if (move.promotesTo == 4) {
                    bitboards.blackQueens = setBit(bitboards.blackQueens, move.to);
                    bitboards.hash ^= ZOBRIST_TABLE[move.to][10];
                } else if (move.promotesTo == 3) {
                    bitboards.blackRooks = setBit(bitboards.blackRooks, move.to);
                    bitboards.hash ^= ZOBRIST_TABLE[move.to][7];
                } else if (move.promotesTo == 2) {
                    bitboards.blackBishops = setBit(bitboards.blackBishops, move.to);
                    bitboards.hash ^= ZOBRIST_TABLE[move.to][9];
                } else if (move.promotesTo == 1) {
                    bitboards.blackKnights = setBit(bitboards.blackKnights, move.to);
                    bitboards.hash ^= ZOBRIST_TABLE[move.to][8];
                }
            } else {
                bitboards.blackPawns = setBit(bitboards.blackPawns, move.to);
                bitboards.hash ^= ZOBRIST_TABLE[move.to][6];
            }
        }
        else if (move.pieceType == 1) {
            bitboards.blackKnights = setBit(bitboards.blackKnights, move.to);
            bitboards.hash ^= ZOBRIST_TABLE[move.to][8];
            bitboards.blackKnights = unsetBit(bitboards.blackKnights, move.from);
            bitboards.hash ^= ZOBRIST_TABLE[move.from][8];

        } else if (move.pieceType == 2) {
            bitboards.blackBishops = setBit(bitboards.blackBishops, move.to);
            bitboards.hash ^= ZOBRIST_TABLE[move.to][9];
            bitboards.blackBishops = unsetBit(bitboards.blackBishops, move.from);
            bitboards.hash ^= ZOBRIST_TABLE[move.from][9];

        } else if (move.pieceType == 3) {
            bitboards.blackRooks = setBit(bitboards.blackRooks, move.to);
            bitboards.hash ^= ZOBRIST_TABLE[move.to][7];
            bitboards.blackRooks = unsetBit(bitboards.blackRooks, move.from);
            bitboards.hash ^= ZOBRIST_TABLE[move.from][7];
            if (bitboards.blackCastleQueenSide && move.from == 31) {
                bitboards.blackCastleQueenSide = false;
                bitboards.hash ^= castlingRights[1];
            } else if (bitboards.blackCastleKingSide && move.from == 24) {
                bitboards.blackCastleKingSide = false;
                bitboards.hash ^= castlingRights[0];
            }
        } else if (move.pieceType == 4) {
            bitboards.blackQueens = setBit(bitboards.blackQueens, move.to);
            bitboards.hash ^= ZOBRIST_TABLE[move.to][10];
            bitboards.blackQueens = unsetBit(bitboards.blackQueens, move.from);
            bitboards.hash ^= ZOBRIST_TABLE[move.from][10];

        } else if (move.pieceType == 5) {
            bitboards.blackKing = setBit(bitboards.blackKing, move.to);
            bitboards.hash ^= ZOBRIST_TABLE[move.to][11];
            bitboards.blackKing = unsetBit(bitboards.blackKing, move.from);
            bitboards.hash ^= ZOBRIST_TABLE[move.from][11];
            if (move.castle) {
                int s;
                int o;
                if (move.castle == 1) {s = 56; o = 58;} else {s = 63; o = 60;}
                    bitboards.allPieces = unsetBit(bitboards.allPieces, s);
                    bitboards.allPieces90 = unsetBit(bitboards.allPieces90, rotated90[s]);
                    bitboards.allPieces45R = unsetBit(bitboards.allPieces45R, rotated45R[s]);
                    bitboards.allPieces45L = unsetBit(bitboards.allPieces45L, rotated45L[s]);
                    bitboards.blackPieces = unsetBit(bitboards.blackPieces, s);
                    bitboards.blackRooks = unsetBit(bitboards.blackRooks, s);
                    bitboards.hash ^= ZOBRIST_TABLE[s][7];

                    bitboards.allPieces = setBit(bitboards.allPieces, o);
                    bitboards.allPieces90 = setBit(bitboards.allPieces90, rotated90[o]);
                    bitboards.allPieces45R = setBit(bitboards.allPieces45R, rotated45R[o]);
                    bitboards.allPieces45L = setBit(bitboards.allPieces45L, rotated45L[o]);
                    bitboards.blackPieces = setBit(bitboards.blackPieces, o);
                    bitboards.blackRooks = setBit(bitboards.blackRooks, o);
                    bitboards.hash ^= ZOBRIST_TABLE[o][7];
                }
            if (bitboards.blackCastleQueenSide) {
                bitboards.blackCastleQueenSide = false;
                bitboards.hash ^= castlingRights[1];
            }
            if (bitboards.blackCastleKingSide) {
                bitboards.blackCastleKingSide = false;
                bitboards.hash ^= castlingRights[0];
            }
        }

        if (bitboards.enPassantSquare) {
            bitboards.hash ^= ZOBRIST_TABLE[lsb(bitboards.enPassantSquare)][12];
        }

        bitboards.enPassantSquare = 0;

        if (move.createsEnPassant && (checkBit(bitboards.whitePawns, move.to + 1) || checkBit(bitboards.whitePawns, move.to - 1))) {
            // okay this is bullshit. Why did they change the FEN notation to only include the en passant square if it is possible to capture it?
            // I had to search for this bug for hours because my opening book was not working properly.
            bitboards.enPassantSquare = setBit(bitboards.enPassantSquare, move.to + 8);
            bitboards.hash ^= ZOBRIST_TABLE[move.to + 8][12];
        }
    }
    return bitboards;
}

// CHECK DETECTION

bool canOpponentCaptureKing(
        bool isKingWhite, uint64_t occupied, uint64_t occupied90, uint64_t occupied45R, uint64_t occupied45L, 
        uint64_t king, 
        uint64_t enemyKing, uint64_t enemyQueens, uint64_t enemyRooks, uint64_t enemyBishops, uint64_t enemyKnights, uint64_t enemyPawns
    ) {
    // function to check wether the king can be captured by the enemy
    int kingIndex = 0;
    // get the kings position
    if (bitCount(king) != 0) {
        kingIndex = lsb(king);
    } else {
        return true;
    }
    if ((kingAttacks[kingIndex] & enemyKing)) {
        // if a king is in range of the king, it is in check even though thats not possible
        return true;
    } else if ((knightAttacks[kingIndex] & enemyKnights)) {
        // if the king were a knight and it could attack an enemy knight, that knight is attacking the king too, so it is in check
        return true;
    } else if ((generateBishopMoves(kingIndex, occupied45R, occupied45L) & (enemyBishops | enemyQueens))) {
        // if the king were a bishop and it could attack an enemy bishop or queen, that bishop or queen is attacking the king too, so it is in check
        return true;
    } else if ((generateRookMoves(kingIndex, occupied, occupied90) & (enemyRooks | enemyQueens))) {
        return true;
    } else if ((generatePawnAttacks(kingIndex, isKingWhite) & enemyPawns)) {
        return true;
    }
    return false;
}

bool isIllegalCastle(struct Move move, struct Bitboards boards, bool isWhiteToMove) {
    // checks if a castle move is illegal
    struct Move betweenMove;
    if (move.castle == 2) {
        betweenMove.from = move.from;
        betweenMove.to = move.to - 1;
        betweenMove.pieceType = 5;
        betweenMove.isEnPassantCapture = false;
        betweenMove.createsEnPassant = false;
        betweenMove.castle = 0;
    } else {
        betweenMove.from = move.from;
        betweenMove.to = move.to + 1;
        betweenMove.pieceType = 5;
        betweenMove.isEnPassantCapture = false;
        betweenMove.createsEnPassant = false;
        betweenMove.castle = 0;
    }
    struct Bitboards newBoard = doMove(betweenMove, boards, isWhiteToMove); // cant castle if in check while on the between square
    if (isWhiteToMove) {
        return canOpponentCaptureKing(isWhiteToMove, newBoard.allPieces, newBoard.allPieces90, newBoard.allPieces45R, newBoard.allPieces45L, newBoard.whiteKing, newBoard.blackKing, newBoard.blackQueens, newBoard.blackRooks, newBoard.blackBishops, newBoard.blackKnights, newBoard.blackPawns);
    }
    return canOpponentCaptureKing(isWhiteToMove, newBoard.allPieces, newBoard.allPieces90, newBoard.allPieces45R, newBoard.allPieces45L, newBoard.blackKing, newBoard.whiteKing, newBoard.whiteQueens, newBoard.whiteRooks, newBoard.whiteBishops, newBoard.whiteKnights, newBoard.whitePawns);
}

bool hasLegalMoves(struct Move *possible, struct Bitboards boards, bool isWhiteToMove) {
    // checks if there are any legal moves for the current player
    // used to detect checkmate after the search
    int removedAmount = 0;
    for (int i = 1; i <= possible[0].from - 1; i++) {
        bool check = false;
        if (possible[i].castle) {
            check = isIllegalCastle(possible[i], boards, isWhiteToMove);
        } else {
            struct Bitboards newBoard = doMove(possible[i], boards, isWhiteToMove);
            if (isWhiteToMove) {
                check = canOpponentCaptureKing(isWhiteToMove, newBoard.allPieces, newBoard.allPieces90, newBoard.allPieces45R, newBoard.allPieces45L, newBoard.whiteKing, newBoard.blackKing, newBoard.blackQueens, newBoard.blackRooks, newBoard.blackBishops, newBoard.blackKnights, newBoard.blackPawns);
            }
            check = canOpponentCaptureKing(isWhiteToMove, newBoard.allPieces, newBoard.allPieces90, newBoard.allPieces45R, newBoard.allPieces45L, newBoard.blackKing, newBoard.whiteKing, newBoard.whiteQueens, newBoard.whiteRooks, newBoard.whiteBishops, newBoard.whiteKnights, newBoard.whitePawns);
        }
        if (check) {
            removedAmount++;
        }
    }
    return (possible[0].from - 1) - removedAmount > 0;
}

// EVALUATION

int evaluateFinal(uint64_t *whitePawns, uint64_t *whiteKnights, uint64_t *whiteBishops, uint64_t *whiteRooks, uint64_t *whiteQueens, uint64_t *whiteKing, uint64_t *blackPawns, uint64_t *blackKnights, uint64_t *blackBishops, uint64_t *blackRooks, uint64_t *blackQueens, uint64_t *blackKing) {
    // todo: take king safety into account
    // we can use pointers here because after popping all the bits, we dont need the original bitboards anymore as it's the final evaluation
    evalCalls++;
    
    int evalWhite = 0;
    int evalBlack = 0;
    // black position evaluation
    // iterate through all the pawns and add their position value to the total
    while (*blackKnights) {evalBlack += knightEvalBlack[popLsb(blackKnights)];}
    while (*blackBishops) {evalBlack += bishopEvalBlack[popLsb(blackBishops)];}
    while (*blackRooks) {evalBlack += rookEvalBlack[popLsb(blackRooks)];}
    while (*blackQueens) {evalBlack += queenEvalBlack[popLsb(blackQueens)];}
    while (*blackPawns) {evalBlack += pawnEvalBlack[popLsb(blackPawns)];}
    // white position evaluation

    while (*whiteKnights) {evalWhite += knightEvalWhite[popLsb(whiteKnights)];}
    while (*whiteBishops) {evalWhite += bishopEvalWhite[popLsb(whiteBishops)];}
    while (*whiteRooks) {evalWhite += rookEvalWhite[popLsb(whiteRooks)];}
    while (*whiteQueens) {evalWhite += queenEvalWhite[popLsb(whiteQueens)];}
    while (*whitePawns) {evalWhite += pawnEvalWhite[popLsb(whitePawns)];}
    // endgame evaluation, takes affect when there is only little material left on the board
    if (evalWhite < 1000 || evalBlack > -1000) {
        // in the endgame an active king is important
        if (evalWhite + evalBlack > 0) { // white is winning
            evalWhite -= abs((lsb((int)whiteKing) % 8) - (lsb((int)blackKing) % 8));
            evalWhite -= abs((lsb((int)whiteKing) / 8) - (lsb((int)blackKing) / 8));
            evalWhite += kingEvalEnd[lsb((int)blackKing)];
        } else {
            evalBlack += abs((lsb((int)whiteKing) % 8) - (lsb((int)blackKing) % 8));
            evalBlack += abs((lsb((int)whiteKing) / 8) - (lsb((int)blackKing) / 8));
            evalBlack -= kingEvalEnd[lsb((int)whiteKing)];
        }
    }
    else {
        evalWhite += kingEvalWhite[lsb((int)whiteKing)];
        evalBlack += kingEvalBlack[lsb((int)blackKing)];
    }
    return (int)(((evalWhite + evalBlack) * 17800) / (evalWhite - evalBlack + 10000));
    //             normal eval        times 17800  /  total of evals     plus 10000
    // this is to encourage the engine to take more exchanges especially when it is ahead
}

int evaluateEarly(uint64_t whitePawns, uint64_t whiteKnights, uint64_t whiteBishops, uint64_t whiteRooks, uint64_t whiteQueens, uint64_t whiteKing, uint64_t blackPawns, uint64_t blackKnights, uint64_t blackBishops, uint64_t blackRooks, uint64_t blackQueens, uint64_t blackKing) {
    // todo: take king safety into account
    evalCalls++;
    int evalWhite = 0;
    int evalBlack = 0;
    while (blackKnights) {evalBlack += knightEvalBlack[popLsb(&blackKnights)];}
    while (blackBishops) {evalBlack += bishopEvalBlack[popLsb(&blackBishops)];}
    while (blackRooks) {evalBlack += rookEvalBlack[popLsb(&blackRooks)];}
    while (blackQueens) {evalBlack += queenEvalBlack[popLsb(&blackQueens)];}
    while (blackPawns) {evalBlack += pawnEvalBlack[popLsb(&blackPawns)];}
    while (whiteKnights) {evalWhite += knightEvalWhite[popLsb(&whiteKnights)];}
    while (whiteBishops) {evalWhite += bishopEvalWhite[popLsb(&whiteBishops)];}
    while (whiteRooks) {evalWhite += rookEvalWhite[popLsb(&whiteRooks)];}
    while (whiteQueens) {evalWhite += queenEvalWhite[popLsb(&whiteQueens)];}
    while (whitePawns) {evalWhite += pawnEvalWhite[popLsb(&whitePawns)];}
    if (evalWhite < 1000 || evalBlack > -1000) {
        if (evalWhite + evalBlack > 0) {
            evalWhite -= abs((lsb(whiteKing) % 8) - (lsb(blackKing) % 8));
            evalWhite -= abs((lsb(whiteKing) / 8) - (lsb(blackKing) / 8));
            evalWhite += kingEvalEnd[lsb(blackKing)];
        } else {
            evalBlack += abs((lsb(whiteKing) % 8) - (lsb(blackKing) % 8));
            evalBlack += abs((lsb(whiteKing) / 8) - (lsb(blackKing) / 8));
            evalBlack -= kingEvalEnd[lsb(whiteKing)];
        }
    }
    else {
        evalWhite += kingEvalWhite[lsb(whiteKing)];
        evalBlack += kingEvalBlack[lsb(blackKing)];
    }
    return (int)(((evalWhite + evalBlack) * 17800) / (evalWhite - evalBlack + 10000));
}

void quickSortArray(struct Move structs[], int values[], int left, int right) {
    // sorts the moves in order of their evaluation
    if (left < right) {
        // Choose pivot value
        int pivotValue = values[left + (right - left) / 2];

        // Partition the arrays
        int i = left, j = right;
        while (i <= j) {
            while (values[i] > pivotValue) i++;
            while (values[j] < pivotValue) j--;
            if (i <= j) {
                // Swap elements
                int tmpValue = values[i];
                values[i] = values[j];
                values[j] = tmpValue;
                struct Move tmpStruct = structs[i];
                structs[i] = structs[j];
                structs[j] = tmpStruct;
                i++;
                j--;
            }
        }

        // Recursively sort the sub-arrays
        quickSortArray(structs, values, left, j);
        quickSortArray(structs, values, i, right);
    }
}

int entryInTable(struct Table page, uint64_t hash) {
    for (int i = 0; i < page.numEntries; i++) {
        if (page.entries[i].hash == hash) {
            return i;
        }
    }
    return -1;
}

struct Move* order(struct Move* moves, Bitboards BITBOARDS, bool isWhiteMove) {
    // orders the moves basen on their immediate evaluation
    // todo: maybe take guesses instead of evaluating every move
    int values[moves[0].from];
    for (int i = 1; i <= moves[0].from - 1; i++) {
        Bitboards newBoard = doMove(moves[i], BITBOARDS, isWhiteMove);
        struct Table page = transTable[newBoard.hash % TABLE_SIZE];
        int entryIndex = entryInTable(page, newBoard.hash);
        if (entryIndex != -1) {
            transpositions++;
            if (page.entries[entryIndex].flag == EXACT) {
                values[i] = page.entries[entryIndex].value;
            } else{
                values[i] = evaluateFinal(&newBoard.whitePawns, &newBoard.whiteKnights, &newBoard.whiteBishops, &newBoard.whiteRooks, &newBoard.whiteQueens, &newBoard.whiteKing, &newBoard.blackPawns, &newBoard.blackKnights, &newBoard.blackBishops, &newBoard.blackRooks, &newBoard.blackQueens, &newBoard.blackKing);
            }
        } else {
            values[i] = evaluateFinal(&newBoard.whitePawns, &newBoard.whiteKnights, &newBoard.whiteBishops, &newBoard.whiteRooks, &newBoard.whiteQueens, &newBoard.whiteKing, &newBoard.blackPawns, &newBoard.blackKnights, &newBoard.blackBishops, &newBoard.blackRooks, &newBoard.blackQueens, &newBoard.blackKing);
        }
    }
    quickSortArray(moves, values, 1, moves[0].from - 1);
    return moves;
}

int quiescenceSearch(struct Bitboards BITBOARDS, int alpha, int beta, bool maximizingPlayer, int depth) {
    // searches the board for captures only using recursion
    // todo: include checks, maybe set a time limit? (not sure if this is necessary because it's usually done in under 1 clock cycle)
    visits++;
    nodes++;

    // probe the table
    struct Table page = transTable[BITBOARDS.hash % TABLE_SIZE]; // todo: what is better, using transTable or a seperate quietTable?
    int entryIndex = entryInTable(page, BITBOARDS.hash);
    if (entryIndex != -1) {
        if (page.entries[entryIndex].depth >= depth) {
            transpositions++;
            if (page.entries[entryIndex].flag == EXACT) {
                return page.entries[entryIndex].value;
            } else if (page.entries[entryIndex].flag == LOWERBOUND) {
                alpha = max(alpha, page.entries[entryIndex].value);
            } else if (page.entries[entryIndex].flag == UPPERBOUND) {
                beta = min(beta, page.entries[entryIndex].value);
            }
            if (alpha >= beta) {
                return page.entries[entryIndex].value;
            }
        }
    }

    // generate capture moves
    struct Move* moves;
    if (maximizingPlayer) {
        moves = possiblecaptures(
            maximizingPlayer, 
            BITBOARDS.allPieces, BITBOARDS.allPieces90, BITBOARDS.allPieces45R, BITBOARDS.allPieces45L, BITBOARDS.enPassantSquare, BITBOARDS.whitePieces, BITBOARDS.blackPieces, 
            BITBOARDS.whitePawns, BITBOARDS.whiteKnights, BITBOARDS.whiteBishops, BITBOARDS.whiteRooks, BITBOARDS.whiteQueens, BITBOARDS.whiteKing, 
            BITBOARDS.whiteCastleQueenSide, BITBOARDS.whiteCastleKingSide
            );
    } else {
        moves = possiblecaptures(
            maximizingPlayer, 
            BITBOARDS.allPieces, BITBOARDS.allPieces90, BITBOARDS.allPieces45R, BITBOARDS.allPieces45L, BITBOARDS.enPassantSquare, BITBOARDS.blackPieces, BITBOARDS.whitePieces, 
            BITBOARDS.blackPawns, BITBOARDS.blackKnights, BITBOARDS.blackBishops, BITBOARDS.blackRooks, BITBOARDS.blackQueens, BITBOARDS.blackKing, 
            BITBOARDS.blackCastleQueenSide, BITBOARDS.blackCastleKingSide
            );
    }

    if (moves[0].from == 1) {
        // no more capture moves possible (quiet position)
        return evaluateFinal(&BITBOARDS.whitePawns, &BITBOARDS.whiteKnights, &BITBOARDS.whiteBishops, &BITBOARDS.whiteRooks, &BITBOARDS.whiteQueens, &BITBOARDS.whiteKing, &BITBOARDS.blackPawns, &BITBOARDS.blackKnights, &BITBOARDS.blackBishops, &BITBOARDS.blackRooks, &BITBOARDS.blackQueens, &BITBOARDS.blackKing);
    }

    // order the moves to get the best ones first and increase alpha beta pruning
    order(moves, BITBOARDS, maximizingPlayer);

    if (maximizingPlayer) { // white player
        // start with the normal evaluation since nobody can be forced to capture
        int value = evaluateEarly(BITBOARDS.whitePawns, BITBOARDS.whiteKnights, BITBOARDS.whiteBishops, BITBOARDS.whiteRooks, BITBOARDS.whiteQueens, BITBOARDS.whiteKing, BITBOARDS.blackPawns, BITBOARDS.blackKnights, BITBOARDS.blackBishops, BITBOARDS.blackRooks, BITBOARDS.blackQueens, BITBOARDS.blackKing);
        uint8_t legalMoves = 0;
        for (int i = 1; i <= moves[0].from - 1; i++) {
            // do each of the moves
            struct Bitboards newBoard = doMove(moves[i], BITBOARDS, maximizingPlayer);
            // if the move is legal, do a quiescence search on the new board
            if (!canOpponentCaptureKing(maximizingPlayer, newBoard.allPieces, newBoard.allPieces90, newBoard.allPieces45R, newBoard.allPieces45L, newBoard.whiteKing, newBoard.blackKing, newBoard.blackQueens, newBoard.blackRooks, newBoard.blackBishops, newBoard.blackKnights, newBoard.blackPawns) && 
                (moves[i].castle ? (!isIllegalCastle(moves[i], BITBOARDS, maximizingPlayer) && !canOpponentCaptureKing(maximizingPlayer, BITBOARDS.allPieces, BITBOARDS.allPieces90, BITBOARDS.allPieces45R, BITBOARDS.allPieces45L, BITBOARDS.whiteKing, BITBOARDS.blackKing, BITBOARDS.blackQueens, BITBOARDS.blackRooks, BITBOARDS.blackBishops, BITBOARDS.blackKnights, BITBOARDS.blackPawns)) : 1)) {
                int newValue = quiescenceSearch(newBoard, alpha, beta, false, depth - 1);
                if (newValue > value) {
                    value = newValue;
                }
                legalMoves++;
            }
            // update alpha and try to prune
            alpha = max(alpha, value);
            if (beta <= alpha) {
                break;
            }
        }
        free(moves);
        if (legalMoves == 0) {
            if (canOpponentCaptureKing(maximizingPlayer, BITBOARDS.allPieces, BITBOARDS.allPieces90, BITBOARDS.allPieces45R, BITBOARDS.allPieces45L, BITBOARDS.whiteKing, BITBOARDS.blackKing, BITBOARDS.blackQueens, BITBOARDS.blackRooks, BITBOARDS.blackBishops, BITBOARDS.blackKnights, BITBOARDS.blackPawns)) {
                // if the opponent can capture the king, we can't continue
              return evaluateFinal(&BITBOARDS.whitePawns, &BITBOARDS.whiteKnights, &BITBOARDS.whiteBishops, &BITBOARDS.whiteRooks, &BITBOARDS.whiteQueens, &BITBOARDS.whiteKing, &BITBOARDS.blackPawns, &BITBOARDS.blackKnights, &BITBOARDS.blackBishops, &BITBOARDS.blackRooks, &BITBOARDS.blackQueens, &BITBOARDS.blackKing);
            }
          }
        // store the result in the table
        struct HashEntry newEntry;
        newEntry.hash = BITBOARDS.hash;
        newEntry.depth = depth;
        newEntry.value = value;
        if (value <= alpha) {
            newEntry.flag = UPPERBOUND;
        } else if (value >= beta) {
            newEntry.flag = LOWERBOUND;
        } else {
            newEntry.flag = EXACT;
        }

        if (transTable[BITBOARDS.hash % TABLE_SIZE].numEntries > 10) { // if the table gets to full, start overwriting old entries
            transTable[BITBOARDS.hash % TABLE_SIZE].numEntries = 1;
        } else {
            transTable[BITBOARDS.hash % TABLE_SIZE].numEntries++;
            transTable[BITBOARDS.hash % TABLE_SIZE].entries = realloc(transTable[BITBOARDS.hash % TABLE_SIZE].entries, transTable[BITBOARDS.hash % TABLE_SIZE].numEntries * sizeof(struct HashEntry));
            if (transTable[BITBOARDS.hash % TABLE_SIZE].entries == NULL) {
                printf("Error reallocating memory for quiet table\n");
                exit(1);
            }
        }

        transTable[BITBOARDS.hash % TABLE_SIZE].entries[transTable[BITBOARDS.hash % TABLE_SIZE].numEntries - 1] = newEntry;
        return value;
    } else { // black (minimizing player)
        int value = evaluateEarly(BITBOARDS.whitePawns, BITBOARDS.whiteKnights, BITBOARDS.whiteBishops, BITBOARDS.whiteRooks, BITBOARDS.whiteQueens, BITBOARDS.whiteKing, BITBOARDS.blackPawns, BITBOARDS.blackKnights, BITBOARDS.blackBishops, BITBOARDS.blackRooks, BITBOARDS.blackQueens, BITBOARDS.blackKing);
        int legalMoves = 0;
        // loop over the moves in reverse order because they are sorted in descending order
        for (int i = moves[0].from - 1; i >= 1; i--) {
            struct Bitboards newBoard = doMove(moves[i], BITBOARDS, maximizingPlayer);
            if (!canOpponentCaptureKing(maximizingPlayer, newBoard.allPieces, newBoard.allPieces90, newBoard.allPieces45R, newBoard.allPieces45L, newBoard.blackKing, newBoard.whiteKing, newBoard.whiteQueens, newBoard.whiteRooks, newBoard.whiteBishops, newBoard.whiteKnights, newBoard.whitePawns) && 
                (moves[i].castle ? (!isIllegalCastle(moves[i], BITBOARDS, maximizingPlayer) && !canOpponentCaptureKing(maximizingPlayer, BITBOARDS.allPieces, BITBOARDS.allPieces90, BITBOARDS.allPieces45R, BITBOARDS.allPieces45L, BITBOARDS.blackKing, BITBOARDS.whiteKing, BITBOARDS.whiteQueens, BITBOARDS.whiteRooks, BITBOARDS.whiteBishops, BITBOARDS.whiteKnights, BITBOARDS.whitePawns)) : 1)) {
                int newValue = quiescenceSearch(newBoard, alpha, beta, true, depth - 1);
                if (newValue < value) {
                    value = newValue;
                }
                legalMoves++;
            }
            beta = min(beta, value);
            if (beta <= alpha) {
                break;
            }
        }
        free(moves);
        
        if (legalMoves == 0) {
                if (canOpponentCaptureKing(maximizingPlayer, BITBOARDS.allPieces, BITBOARDS.allPieces90, BITBOARDS.allPieces45R, BITBOARDS.allPieces45L, BITBOARDS.blackKing, BITBOARDS.whiteKing, BITBOARDS.whiteQueens, BITBOARDS.whiteRooks, BITBOARDS.whiteBishops, BITBOARDS.whiteKnights, BITBOARDS.whitePawns)) {
                    return evaluateFinal(&BITBOARDS.whitePawns, &BITBOARDS.whiteKnights, &BITBOARDS.whiteBishops, &BITBOARDS.whiteRooks, &BITBOARDS.whiteQueens, &BITBOARDS.whiteKing, &BITBOARDS.blackPawns, &BITBOARDS.blackKnights, &BITBOARDS.blackBishops, &BITBOARDS.blackRooks, &BITBOARDS.blackQueens, &BITBOARDS.blackKing);
                }
            }
        struct HashEntry newEntry;
        newEntry.hash = BITBOARDS.hash;
        newEntry.depth = depth;
        newEntry.value = value;
        if (value <= alpha) {
            newEntry.flag = UPPERBOUND;
        } else if (value >= beta) {
            newEntry.flag = LOWERBOUND;
        } else {
            newEntry.flag = EXACT;
        }

        if (transTable[BITBOARDS.hash % TABLE_SIZE].numEntries > 10) {
            transTable[BITBOARDS.hash % TABLE_SIZE].numEntries = 1;
        } else {
            transTable[BITBOARDS.hash % TABLE_SIZE].numEntries++;
            transTable[BITBOARDS.hash % TABLE_SIZE].entries = realloc(transTable[BITBOARDS.hash % TABLE_SIZE].entries, transTable[BITBOARDS.hash % TABLE_SIZE].numEntries * sizeof(struct HashEntry));
            if (transTable[BITBOARDS.hash % TABLE_SIZE].entries == NULL) {
                printf("Error reallocating memory for quiet table\n");
                exit(1);
            }
        }

        transTable[BITBOARDS.hash % TABLE_SIZE].entries[transTable[BITBOARDS.hash % TABLE_SIZE].numEntries - 1] = newEntry;
        return value;
    }
}

int tree(struct Bitboards BITBOARDS, int ply, int alpha, int beta, bool maximizingPlayer, int depth) {
    nodes++;
    visits++;

    struct Table page = transTable[BITBOARDS.hash % TABLE_SIZE];
    int entryIndex = entryInTable(page, BITBOARDS.hash);
    if (entryIndex != -1) {
        if (page.entries[entryIndex].depth >= ply) {
            transpositions++;
            if (page.entries[entryIndex].flag == EXACT) {
                return page.entries[entryIndex].value;
            } else if (page.entries[entryIndex].flag == LOWERBOUND) {
                alpha = max(alpha, page.entries[entryIndex].value);
            } else if (page.entries[entryIndex].flag == UPPERBOUND) {
                beta = min(beta, page.entries[entryIndex].value);
            }
            if (alpha >= beta) {
                return page.entries[entryIndex].value;
            }
        }
    }

    if (ply == 0) {
        quiescenceCalls++;
        return quiescenceSearch(BITBOARDS, alpha, beta, maximizingPlayer, 0);
        // return evaluate(BITBOARDS);
        // why not just return evaluate(BITBOARDS) here?
        // because the eval could change on the very next move and the engine would not see it
        // this way, we only evaluate positions where no captures are possible and thus it's very unlikely that the eval changes
    }
    struct Move* moves;
    if (maximizingPlayer) { // white
        moves = possiblemoves(
            maximizingPlayer, 
            BITBOARDS.allPieces, BITBOARDS.allPieces90, BITBOARDS.allPieces45R, BITBOARDS.allPieces45L, BITBOARDS.enPassantSquare, BITBOARDS.whitePieces, BITBOARDS.blackPieces, 
            BITBOARDS.whitePawns, BITBOARDS.whiteKnights, BITBOARDS.whiteBishops, BITBOARDS.whiteRooks, BITBOARDS.whiteQueens, BITBOARDS.whiteKing, 
            BITBOARDS.whiteCastleQueenSide, BITBOARDS.whiteCastleKingSide
            );
    } else { // black
        moves = possiblemoves(
            maximizingPlayer, 
            BITBOARDS.allPieces, BITBOARDS.allPieces90, BITBOARDS.allPieces45R, BITBOARDS.allPieces45L, BITBOARDS.enPassantSquare, BITBOARDS.blackPieces, BITBOARDS.whitePieces, 
            BITBOARDS.blackPawns, BITBOARDS.blackKnights, BITBOARDS.blackBishops, BITBOARDS.blackRooks, BITBOARDS.blackQueens, BITBOARDS.blackKing, 
            BITBOARDS.blackCastleQueenSide, BITBOARDS.blackCastleKingSide
            );
    }
    order(moves, BITBOARDS, maximizingPlayer);

    if (maximizingPlayer) { // white's turn
        int value = -INF;
        int legalMoves = 0;
        for (int i = 1; i <= moves[0].from - 1; i++) {
            struct Bitboards newBoard = doMove(moves[i], BITBOARDS, maximizingPlayer);
            if (!canOpponentCaptureKing(maximizingPlayer, newBoard.allPieces, newBoard.allPieces90, newBoard.allPieces45R, newBoard.allPieces45L, newBoard.whiteKing, newBoard.blackKing, newBoard.blackQueens, newBoard.blackRooks, newBoard.blackBishops, newBoard.blackKnights, newBoard.blackPawns) && 
                (moves[i].castle ? (!isIllegalCastle(moves[i], BITBOARDS, maximizingPlayer) && !canOpponentCaptureKing(maximizingPlayer, BITBOARDS.allPieces, BITBOARDS.allPieces90, BITBOARDS.allPieces45R, BITBOARDS.allPieces45L, BITBOARDS.whiteKing, BITBOARDS.blackKing, BITBOARDS.blackQueens, BITBOARDS.blackRooks, BITBOARDS.blackBishops, BITBOARDS.blackKnights, BITBOARDS.blackPawns)) : 1)) {
                int newValue = tree(newBoard, ply - 1, alpha, beta, false, depth);
                if (newValue > value) {
                    value = newValue;
                }
                legalMoves++;
            }
            alpha = max(alpha, value);
            if (beta <= alpha) {
                break;
            }
        }
        free(moves);
        if (legalMoves == 0) {
            if (canOpponentCaptureKing(maximizingPlayer, BITBOARDS.allPieces, BITBOARDS.allPieces90, BITBOARDS.allPieces45R, BITBOARDS.allPieces45L, BITBOARDS.whiteKing, BITBOARDS.blackKing, BITBOARDS.blackQueens, BITBOARDS.blackRooks, BITBOARDS.blackBishops, BITBOARDS.blackKnights, BITBOARDS.blackPawns)) {
              return -INF + ply;
            } else {
              return 0;
            }
          }
        struct HashEntry newEntry;
        newEntry.hash = BITBOARDS.hash;
        newEntry.depth = ply;
        newEntry.value = value;
        if (value <= alpha) {
            newEntry.flag = UPPERBOUND;
        } else if (value >= beta) {
            newEntry.flag = LOWERBOUND;
        } else {
            newEntry.flag = EXACT;
        }

        if (transTable[BITBOARDS.hash % TABLE_SIZE].numEntries > 10) {
            transTable[BITBOARDS.hash % TABLE_SIZE].numEntries = 1;
        } else {
            transTable[BITBOARDS.hash % TABLE_SIZE].numEntries++;
            transTable[BITBOARDS.hash % TABLE_SIZE].entries = realloc(transTable[BITBOARDS.hash % TABLE_SIZE].entries, transTable[BITBOARDS.hash % TABLE_SIZE].numEntries * sizeof(struct HashEntry));
            if (transTable[BITBOARDS.hash % TABLE_SIZE].entries == NULL) {
                printf("Error reallocating memory for quiet table\n");
                exit(1);
            }
        }
        
        transTable[BITBOARDS.hash % TABLE_SIZE].entries[transTable[BITBOARDS.hash % TABLE_SIZE].numEntries - 1] = newEntry;

        return value;
    } else { // black's turn
        int value = INF;
        int legalMoves = 0;
        for (int i = moves[0].from - 1; i >= 1; i--) { // loop over all moves in reverse order because they are sorted in descending order
            struct Bitboards newBoard = doMove(moves[i], BITBOARDS, maximizingPlayer); // on first call do black moves
            if (!canOpponentCaptureKing(maximizingPlayer, newBoard.allPieces, newBoard.allPieces90, newBoard.allPieces45R, newBoard.allPieces45L, newBoard.blackKing, newBoard.whiteKing, newBoard.whiteQueens, newBoard.whiteRooks, newBoard.whiteBishops, newBoard.whiteKnights, newBoard.whitePawns) && 
                (moves[i].castle ? (!isIllegalCastle(moves[i], BITBOARDS, maximizingPlayer) && !canOpponentCaptureKing(maximizingPlayer, BITBOARDS.allPieces, BITBOARDS.allPieces90, BITBOARDS.allPieces45R, BITBOARDS.allPieces45L, BITBOARDS.blackKing, BITBOARDS.whiteKing, BITBOARDS.whiteQueens, BITBOARDS.whiteRooks, BITBOARDS.whiteBishops, BITBOARDS.whiteKnights, BITBOARDS.whitePawns)) : 1)) {
                int newValue = tree(newBoard, ply - 1, alpha, beta, true, depth);
                if (newValue < value) {
                    value = newValue;
                }
                legalMoves++;
            }
            beta = min(beta, value);
            if (beta <= alpha) {
                break;
            }
        }
        free(moves);
        if (legalMoves == 0) {
                if (canOpponentCaptureKing(maximizingPlayer, BITBOARDS.allPieces, BITBOARDS.allPieces90, BITBOARDS.allPieces45R, BITBOARDS.allPieces45L, BITBOARDS.blackKing, BITBOARDS.whiteKing, BITBOARDS.whiteQueens, BITBOARDS.whiteRooks, BITBOARDS.whiteBishops, BITBOARDS.whiteKnights, BITBOARDS.whitePawns)) {
                    return INF - ply;
                } else {
                    return 0;
                }
            }
        struct HashEntry newEntry;
        newEntry.hash = BITBOARDS.hash;
        newEntry.depth = ply;
        newEntry.value = value;
        if (value <= alpha) {
            newEntry.flag = UPPERBOUND;
        } else if (value >= beta) {
            newEntry.flag = LOWERBOUND;
        } else {
            newEntry.flag = EXACT;
        }
        if (transTable[BITBOARDS.hash % TABLE_SIZE].numEntries > 10) {
            transTable[BITBOARDS.hash % TABLE_SIZE].numEntries = 1;
        } else {
            transTable[BITBOARDS.hash % TABLE_SIZE].numEntries++;
            transTable[BITBOARDS.hash % TABLE_SIZE].entries = realloc(transTable[BITBOARDS.hash % TABLE_SIZE].entries, transTable[BITBOARDS.hash % TABLE_SIZE].numEntries * sizeof(struct HashEntry));
            if (transTable[BITBOARDS.hash % TABLE_SIZE].entries == NULL) {
                printf("Error reallocating memory for quiet table\n");
                exit(1);
            }
        }
        
        transTable[BITBOARDS.hash % TABLE_SIZE].entries[transTable[BITBOARDS.hash % TABLE_SIZE].numEntries - 1] = newEntry;
        return value;
    }
}

struct Move bestMove(struct Move *possible, struct Bitboards bitboards, bool isWhiteMove) {
    // find the best move using iterative deepening
    
    nodes = 0;
    transpositions = 0;

    struct Move best;
    int values[possible[0].from];

    // ordering moves here is not necessary because at depth 0 the moves will be ordered by the order function
    clock_t start_time = clock();
    
    bool stopSearch = false;
    int d = 0;

    printf("move amount: %d\n", possible[0].from - 1);

    for (int ply = 0; ply < maxDepth; ply++) {
        printf("Depth: %d - \n", ply+1);
        for (int i = 1; i <= possible[0].from - 1; i++) {
            int moveEval;
            if (stopSearch) {
                moveEval = 0;
            } else {
                struct Bitboards newBoard = doMove(possible[i], bitboards, isWhiteMove); // engine move
                visits = 0;
                printf("nodes after move from: %d to: %d pieceType: %d castle: %d creates EP: %d is EP capture: %d promotion: %d - ", possible[i].from, possible[i].to, possible[i].pieceType, possible[i].castle, possible[i].createsEnPassant, possible[i].isEnPassantCapture, possible[i].promotesTo);
                if (isWhiteMove ? (!canOpponentCaptureKing(isWhiteMove, newBoard.allPieces, newBoard.allPieces90, newBoard.allPieces45R, newBoard.allPieces45L, newBoard.whiteKing, newBoard.blackKing, newBoard.blackQueens, newBoard.blackRooks, newBoard.blackBishops, newBoard.blackKnights, newBoard.blackPawns) && (possible[i].castle ? (!isIllegalCastle(possible[i], bitboards, isWhiteMove) && !canOpponentCaptureKing(isWhiteMove, bitboards.allPieces, bitboards.allPieces90, bitboards.allPieces45R, bitboards.allPieces45L, bitboards.whiteKing, bitboards.blackKing, bitboards.blackQueens, bitboards.blackRooks, bitboards.blackBishops, bitboards.blackKnights, bitboards.blackPawns)) : 1)) : 
                                  (!canOpponentCaptureKing(isWhiteMove, newBoard.allPieces, newBoard.allPieces90, newBoard.allPieces45R, newBoard.allPieces45L, newBoard.blackKing, newBoard.whiteKing, newBoard.whiteQueens, newBoard.whiteRooks, newBoard.whiteBishops, newBoard.whiteKnights, newBoard.whitePawns) && (possible[i].castle ? (!isIllegalCastle(possible[i], bitboards, isWhiteMove) && !canOpponentCaptureKing(isWhiteMove, bitboards.allPieces, bitboards.allPieces90, bitboards.allPieces45R, bitboards.allPieces45L, bitboards.blackKing, bitboards.whiteKing, bitboards.whiteQueens, bitboards.whiteRooks, bitboards.whiteBishops, bitboards.whiteKnights, bitboards.whitePawns)) : 1))
                    ) {
                    moveEval = tree(newBoard, ply, -INF, INF, !isWhiteMove, maxDepth); // start search
                } else {
                    if (isWhiteMove) {
                        // illegal move
                        moveEval = -100001;
                    } else {
                        moveEval = 100001;
                    }
                }
                printf("%d\n", visits);
            }
            values[i] = moveEval;
            if (((double)(clock() - start_time)) / (CLOCKS_PER_SEC / 1000) > maxTime*1000) {
                // if time is up, stop searching
                break;
                }
            else if ((isWhiteMove && (moveEval >= 90000)) || (!isWhiteMove && (moveEval <= -90000))) {
                printf("checkmate found\n");
                return possible[i];
                stopSearch = true;
            }
        }
        if (((double)(clock() - start_time)) / (CLOCKS_PER_SEC / 1000) > maxTime*1000) {
            printf("time out\n");
            break;
            }

        quickSortArray(possible, values, 1, possible[0].from - 1);

        if (!isWhiteMove) {
            printf(">>> best move: from: %d  to: %d pieceType: %d value: %d\n", possible[possible[0].from-1].from, possible[possible[0].from-1].to, possible[possible[0].from-1].pieceType, values[possible[0].from-1]);
        } else {
            printf(">>> best move: from: %d  to: %d pieceType: %d value: %d\n", possible[1].from, possible[1].to, possible[1].pieceType, values[1]);
        }
        d = ply;
    }

    printf("\ntranspositions found: %d\n", transpositions);

    if (isWhiteMove) {
        best = possible[1];
    } else {
        best = possible[possible[0].from - 1];
    }
    

    if (values[1] == -100001 || values[possible[0].from-1] == 100001) {
        // this happens on a position where the engine is already checkmated
        struct Move move = {from: -1, to: -1, pieceType: -1, castle: -1, isEnPassantCapture: -1, createsEnPassant: -1, promotesTo: -1};
        return move;
    } else {
        return best;
    }
}

struct Book* readBook(Book *bookPages, char* fileName) {
    FILE *origin;
    origin = fopen(fileName, "rb");
    if (origin == NULL) {
        printf("opening book not found\n");
        exit(EXIT_FAILURE);
    }

    printf("reading book\n");

    for (int i = 0; i < BOOK_SIZE; i++) {
        // for each page in the book (the number of indices useable to access the book)
        fread(&bookPages[i].numEntries, sizeof(int), 1, origin);
        bookPages[i].entries = malloc(sizeof(BookEntry) * bookPages[i].numEntries);
        if (bookPages[i].entries == NULL) {
            printf("malloc failed\n");
            exit(EXIT_FAILURE);
        }
        for (int j = 0; j < bookPages[i].numEntries; j++) {
            // for each of the entries read its hash and the number of moves
            fread(&bookPages[i].entries[j].hash, sizeof(uint64_t), 1, origin);
            fread(&bookPages[i].entries[j].numMoves, sizeof(uint8_t), 1, origin);
            bookPages[i].entries[j].moves = malloc(sizeof(Move) * bookPages[i].entries[j].numMoves);
            bookPages[i].entries[j].occourences = malloc(sizeof(int) * bookPages[i].entries[j].numMoves);
            if (bookPages[i].entries[j].moves == NULL || bookPages[i].entries[j].occourences == NULL) {
                printf("malloc failed\n");
                exit(EXIT_FAILURE);
            }
            // read all of the moves and move occourences of that entry
            for (int l = 0; l < bookPages[i].entries[j].numMoves; l++) {
                fread(&bookPages[i].entries[j].moves[l], sizeof(Move), 1, origin);
                fread(&bookPages[i].entries[j].occourences[l], sizeof(int), 1, origin);
            }
        }
    }

    fclose(origin);
    return bookPages;
}

struct Book *bookEntries;

void initBook() {
    bookEntries = malloc(sizeof(Book) * BOOK_SIZE);
    bookEntries = memset(bookEntries, 0, sizeof(Book) * BOOK_SIZE);
    if (bookEntries == NULL) {
        printf("malloc failed\n");
        exit(EXIT_FAILURE);
    }
    bookEntries = readBook(bookEntries, "testBook4.dat");
    printf("book read\n");
}

struct Move getBookMove(uint64_t hash) {
    if (useBook) {

        struct Book page = bookEntries[hash % BOOK_SIZE];

        for (int i = 0; i < page.numEntries; i++) {
            if (page.entries[i].hash == hash) {
                printf("num moves: %d\n", page.entries[i].numMoves);
                // for (int j = 0; j < page.entries[i].numMoves; j++) {
                //     printf("from: %d to: %d amount: %d\n", page.entries[i].moves[j].from, page.entries[i].moves[j].to, page.entries[i].occourences[j]);
                // }
                quickSortArray(page.entries[i].moves, page.entries[i].occourences, 0, page.entries[i].numMoves - 1);
                for (int j = 0; j < page.entries[i].numMoves; j++) {
                    printf("from: %d to: %d amount: %d\n", page.entries[i].moves[j].from, page.entries[i].moves[j].to, page.entries[i].occourences[j]);
                }
                struct Move entryMove = {
                    from: page.entries[i].moves[0].from, 
                    to: page.entries[i].moves[0].to, 
                    pieceType: page.entries[i].moves[0].pieceType, 
                    promotesTo: page.entries[i].moves[0].promotesTo, 
                    castle: page.entries[i].moves[0].castle, 
                    createsEnPassant: page.entries[i].moves[0].createsEnPassant, 
                    isEnPassantCapture: page.entries[i].moves[0].isEnPassantCapture
                    };
                return entryMove;
            }
        }

        printf("nothing there\n");
    }

    struct Move nullmove;
    nullmove.from = -1;
    nullmove.to = -1;
    nullmove.pieceType = -1;
    nullmove.promotesTo = -1;
    nullmove.castle = -1;
    nullmove.createsEnPassant = -1;
    nullmove.isEnPassantCapture = -1;
    
    return nullmove;
}

void engineMove(bool isWhite) {
    // find and make the engine move
    clock_t start_time = clock();
    struct Move best;
    // probe the book
    struct Move bookMove = getBookMove(bitboards.hash);
    if (bookMove.from != 255) {
        best = bookMove;
        printf("book move found!\n");
    } else {
        struct Move* possible;
        if (isWhite) {
            possible = possiblemoves(
                isWhite, 
                bitboards.allPieces, bitboards.allPieces90, bitboards.allPieces45R, bitboards.allPieces45L, bitboards.enPassantSquare, bitboards.whitePieces, bitboards.blackPieces, 
                bitboards.whitePawns, bitboards.whiteKnights, bitboards.whiteBishops, bitboards.whiteRooks, bitboards.whiteQueens, bitboards.whiteKing, 
                bitboards.whiteCastleQueenSide, bitboards.whiteCastleKingSide
                );
        } else {
            possible = possiblemoves(
                isWhite, 
                bitboards.allPieces, bitboards.allPieces90, bitboards.allPieces45R, bitboards.allPieces45L, bitboards.enPassantSquare, bitboards.blackPieces, bitboards.whitePieces, 
                bitboards.blackPawns, bitboards.blackKnights, bitboards.blackBishops, bitboards.blackRooks, bitboards.blackQueens, bitboards.blackKing, 
                bitboards.blackCastleQueenSide, bitboards.blackCastleKingSide
                );
        }

        best = bestMove(possible, bitboards, isWhite);
        free(possible);
    }

    if (best.from != 255) { // if there is a legal move
        updateFenClocks(best);
        bitboards = doMove(best, bitboards, isWhite);
        printf("final move: %s%s\n", notation[best.from], notation[best.to]);
        printf("evaluation: %d\n", quiescenceSearch(bitboards, -INF, INF, !isWhite, 0));
        printf("board after move\n");
        printBoard(bitboards, !isWhite);
        printBinary(bitboards.hash);

        lastfrom = best.from;
        lastto = best.to;

        printf("nodes searched: %d\n", nodes);
    } else {
        if (isWhite ? (!canOpponentCaptureKing(isWhite, bitboards.allPieces, bitboards.allPieces90, bitboards.allPieces45R, bitboards.allPieces45L, bitboards.whiteKing, bitboards.blackKing, bitboards.blackQueens, bitboards.blackRooks, bitboards.blackBishops, bitboards.blackKnights, bitboards.blackPawns)) : 
                      (!canOpponentCaptureKing(isWhite, bitboards.allPieces, bitboards.allPieces90, bitboards.allPieces45R, bitboards.allPieces45L, bitboards.blackKing, bitboards.whiteKing, bitboards.whiteQueens, bitboards.whiteRooks, bitboards.whiteBishops, bitboards.whiteKnights, bitboards.whitePawns))
        ) {
            printf("engine is mate\n");
        } else {
            printf("draw\n");
        }
        exit(0);
    }

    // log some stats
    clock_t end_time = clock();
    double elapsed_time = ((double)(end_time - start_time)) / (CLOCKS_PER_SEC / 1000); // Convert to milliseconds
    printf("Elapsed time: %.2f milliseconds\n", elapsed_time);
    printf("nodes per second: %d\n", (int)(nodes/elapsed_time*1000));
    printf("evaluations per second: %d\n", (int)(evalCalls/elapsed_time*1000));

    // checkmate detection
    struct Move* othermoves;
    if (isWhite) {
        othermoves = possiblemoves(
            !isWhite, 
            bitboards.allPieces, bitboards.allPieces90, bitboards.allPieces45R, bitboards.allPieces45L, bitboards.enPassantSquare, bitboards.blackPieces, bitboards.whitePieces, 
            bitboards.blackPawns, bitboards.blackKnights, bitboards.blackBishops, bitboards.blackRooks, bitboards.blackQueens, bitboards.blackKing, 
            bitboards.blackCastleQueenSide, bitboards.blackCastleKingSide
            );
    } else {
        othermoves = possiblemoves(
            !isWhite, 
            bitboards.allPieces, bitboards.allPieces90, bitboards.allPieces45R, bitboards.allPieces45L, bitboards.enPassantSquare, bitboards.whitePieces, bitboards.blackPieces, 
            bitboards.whitePawns, bitboards.whiteKnights, bitboards.whiteBishops, bitboards.whiteRooks, bitboards.whiteQueens, bitboards.whiteKing, 
            bitboards.whiteCastleQueenSide, bitboards.whiteCastleKingSide
            );
    }

    if (!hasLegalMoves(othermoves, bitboards, !isWhite)) {
        if (!isWhite ? (!canOpponentCaptureKing(!isWhite, bitboards.allPieces, bitboards.allPieces90, bitboards.allPieces45R, bitboards.allPieces45L, bitboards.whiteKing, bitboards.blackKing, bitboards.blackQueens, bitboards.blackRooks, bitboards.blackBishops, bitboards.blackKnights, bitboards.blackPawns)) : 
                      (!canOpponentCaptureKing(!isWhite, bitboards.allPieces, bitboards.allPieces90, bitboards.allPieces45R, bitboards.allPieces45L, bitboards.blackKing, bitboards.whiteKing, bitboards.whiteQueens, bitboards.whiteRooks, bitboards.whiteBishops, bitboards.whiteKnights, bitboards.whitePawns))
        ) {
            printf("player is mate\n");
        } else {
            printf("draw\n");
        }
        exit(0);
    }
    free(othermoves);

    // log counters
    printf("eval: %d moves: %d possible: %d quiet: %d\n", evalCalls, moveCalls, possibleCalls, quiescenceCalls);
    evalCalls = 0;
    moveCalls = 0;
    possibleCalls = 0;
    quiescenceCalls = 0;
}

int perft(bool isWhiteMove, int depth, Bitboards bitboards) {
    // perft function for debugging
    if (depth == 0) {
        return 1;
    }

    struct Move* possible;
    if (isWhiteMove) {
        possible = possiblemoves(
            isWhiteMove, 
            bitboards.allPieces, bitboards.allPieces90, bitboards.allPieces45R, bitboards.allPieces45L, bitboards.enPassantSquare, bitboards.whitePieces, bitboards.blackPieces, 
            bitboards.whitePawns, bitboards.whiteKnights, bitboards.whiteBishops, bitboards.whiteRooks, bitboards.whiteQueens, bitboards.whiteKing, 
            bitboards.whiteCastleQueenSide, bitboards.whiteCastleKingSide
            );
    } else {
        possible = possiblemoves(
            isWhiteMove, 
            bitboards.allPieces, bitboards.allPieces90, bitboards.allPieces45R, bitboards.allPieces45L, bitboards.enPassantSquare, bitboards.blackPieces, bitboards.whitePieces, 
            bitboards.blackPawns, bitboards.blackKnights, bitboards.blackBishops, bitboards.blackRooks, bitboards.blackQueens, bitboards.blackKing, 
            bitboards.blackCastleQueenSide, bitboards.blackCastleKingSide
            );
    }
    
    int nodes = 0;
    for (int i = 1; i <= possible[0].from - 1; i++) {
        struct Bitboards newBoard = doMove(possible[i], bitboards, isWhiteMove);
        if (isWhiteMove ? (!canOpponentCaptureKing(isWhiteMove, newBoard.allPieces, newBoard.allPieces90, newBoard.allPieces45R, newBoard.allPieces45L, newBoard.whiteKing, newBoard.blackKing, newBoard.blackQueens, newBoard.blackRooks, newBoard.blackBishops, newBoard.blackKnights, newBoard.blackPawns) && (possible[i].castle ? (!isIllegalCastle(possible[i], bitboards, isWhiteMove) && !canOpponentCaptureKing(isWhiteMove, bitboards.allPieces, bitboards.allPieces90, bitboards.allPieces45R, bitboards.allPieces45L, bitboards.whiteKing, bitboards.blackKing, bitboards.blackQueens, bitboards.blackRooks, bitboards.blackBishops, bitboards.blackKnights, bitboards.blackPawns)) : 1)) : 
                          (!canOpponentCaptureKing(isWhiteMove, newBoard.allPieces, newBoard.allPieces90, newBoard.allPieces45R, newBoard.allPieces45L, newBoard.blackKing, newBoard.whiteKing, newBoard.whiteQueens, newBoard.whiteRooks, newBoard.whiteBishops, newBoard.whiteKnights, newBoard.whitePawns) && (possible[i].castle ? (!isIllegalCastle(possible[i], bitboards, isWhiteMove) && !canOpponentCaptureKing(isWhiteMove, bitboards.allPieces, bitboards.allPieces90, bitboards.allPieces45R, bitboards.allPieces45L, bitboards.blackKing, bitboards.whiteKing, bitboards.whiteQueens, bitboards.whiteRooks, bitboards.whiteBishops, bitboards.whiteKnights, bitboards.whitePawns)) : 1))
            ) {
            nodes += perft(!isWhiteMove, depth-1, newBoard);
        }
    }
    free(possible);
    return nodes;
}

int main(int argc, char *argv[]) {
    // main function with command line interface

    srand(209749); // constant seed for testing and maybe an opening book later

    initKingAttacks();
    initKnightAttacks();
    initBishopDiagonalAttacksR();
    initBishopDiagonalAttacksL();
    initRookRowAttacks();
    initRookFileAttacks();
    initZobrist();
    initTransTables();

    if (argc > 1)
    {
        if (
            strcmp(argv[1], "-h") == 0 || 
            strcmp(argv[1], "--help") == 0 ||
            strcmp(argv[1], "-help") == 0
            ) {
            printLogo();
            printHelp();
            return 0;
        }
        else 
        if (
            strcmp(argv[1], "-e") == 0 ||
            strcmp(argv[1], "--evaluate") == 0 ||
            strcmp(argv[1], "-evaluate") == 0
            ) {

            if (argc < 3) {
                printf("please provide a fen string\n");
                return 0;
            }
            int startPosition[8][8] = {0};
            fenToPosition(argv[2], startPosition);
            int isWhite = strcmp(argv[3], "w") == 0 ? 1 : 0;
            initBoards(startPosition, isWhite, argv[4], argv[5], atoi(argv[6]), atoi(argv[7]));
            int eval = quiescenceSearch(bitboards, -INF, INF, isWhite, 0);
            printf("quick evaluation: %d\n", eval);
            printf("running deeper evaluation (7ply)...\n");
            int deepEval = tree(bitboards, 7, -INF, INF, isWhite, 5);
            printf("deeper evaluation: %d\n", deepEval);
            printBoard(bitboards, isWhite);
            return 0;
        }
        else
        if (
            strcmp(argv[1], "-b") == 0 ||
            strcmp(argv[1], "--best") == 0 ||
            strcmp(argv[1], "-best") == 0
            ) {
            if (argc < 3) {
                printf("please provide a fen string\n");
                return 0;
            }
            int isWhite = strcmp(argv[3], "w") == 0 ? 1 : 0;
            int startPosition[8][8] = {0};
            fenToPosition(argv[2], startPosition);
            initBoards(startPosition, isWhite, argv[4], argv[5], atoi(argv[6]), atoi(argv[7]));
            printBoard(bitboards, isWhite);
            printBinary(bitboards.hash);
            maxDepth = 20;
            maxTime = 10;

            if (argc > 8) {
                if (
                    strcmp(argv[8], "-t") == 0 ||
                    strcmp(argv[8], "--time") == 0 ||
                    strcmp(argv[8], "-time") == 0
                    ) {
                        maxTime = atoi(argv[9]);
                } else if (
                    strcmp(argv[8], "-d") == 0 ||
                    strcmp(argv[8], "--depth") == 0 ||
                    strcmp(argv[8], "-depth") == 0
                    ) {
                        maxDepth = atoi(argv[9]);
                } else if (
                    strcmp(argv[8], "-b") == 0 ||
                    strcmp(argv[8], "--book") == 0 ||
                    strcmp(argv[8], "-book") == 0
                    ) {
                        useBook = true;
                }
            }

            if (argc > 10) {
                if (
                    strcmp(argv[10], "-d") == 0 ||
                    strcmp(argv[10], "--depth") == 0 ||
                    strcmp(argv[10], "-depth") == 0
                    ) {
                        maxDepth = atoi(argv[11]);
                } else if (
                    strcmp(argv[10], "-t") == 0 ||
                    strcmp(argv[10], "--time") == 0 ||
                    strcmp(argv[10], "-time") == 0
                    ) {
                        maxTime = atoi(argv[11]);
                } else if (
                    strcmp(argv[10], "-b") == 0 ||
                    strcmp(argv[10], "--book") == 0 ||
                    strcmp(argv[10], "-book") == 0
                    ) {
                        useBook = true;
                }
            }

            if (argc > 12) {
                if (
                    strcmp(argv[12], "-t") == 0 ||
                    strcmp(argv[12], "--time") == 0 ||
                    strcmp(argv[12], "-time") == 0
                    ) {
                        maxTime = atoi(argv[13]);
                } else if (
                    strcmp(argv[12], "-d") == 0 ||
                    strcmp(argv[12], "--depth") == 0 ||
                    strcmp(argv[12], "-depth") == 0
                    ) {
                        maxDepth = atoi(argv[13]);
                } else if (
                    strcmp(argv[12], "-b") == 0 ||
                    strcmp(argv[12], "--book") == 0 ||
                    strcmp(argv[12], "-book") == 0
                    ) {
                        useBook = true;
                }
            }

            printf("max time: %d\n", maxTime);
            printf("max depth: %d\n", maxDepth);
            printf("use book: %d\n", useBook);

            if (useBook) {
                initBook();
            }

            engineMove(isWhite);

            if (useBook) {
                free(bookEntries);
            }
            free(transTable);
            free(quietTable);

            return 0;
        }
        else
        if (
            strcmp(argv[1], "-g") == 0 ||
            strcmp(argv[1], "--game") == 0 ||
            strcmp(argv[1], "-game") == 0
            ) {
            
            if (argc < 3) {
                printf("please provide a fen string\n");
                return 0;
            }
            int startPosition[8][8] = {0};
            int isPlayerWhite = strcmp(argv[3], "w") == 0 ? 1 : 0;
            fenToPosition(argv[2], startPosition);
            initBoards(startPosition, isPlayerWhite, argv[4], argv[5], atoi(argv[6]), atoi(argv[7]));
            printBoard(bitboards, isPlayerWhite);
            maxDepth = 20;
            maxTime = 10;

            if (argc > 8) {
                if (
                    strcmp(argv[8], "-t") == 0 ||
                    strcmp(argv[8], "--time") == 0 ||
                    strcmp(argv[8], "-time") == 0
                    ) {
                        maxTime = atoi(argv[9]);
                } else if (
                    strcmp(argv[8], "-d") == 0 ||
                    strcmp(argv[8], "--depth") == 0 ||
                    strcmp(argv[8], "-depth") == 0
                    ) {
                        maxDepth = atoi(argv[9]);
                } else if (
                    strcmp(argv[8], "-b") == 0 ||
                    strcmp(argv[8], "--book") == 0 ||
                    strcmp(argv[8], "-book") == 0
                    ) {
                        useBook = 1;
                }
            }

            if (argc > 10) {
                if (
                    strcmp(argv[10], "-d") == 0 ||
                    strcmp(argv[10], "--depth") == 0 ||
                    strcmp(argv[10], "-depth") == 0
                    ) {
                        maxDepth = atoi(argv[11]);
                } else if (
                    strcmp(argv[10], "-t") == 0 ||
                    strcmp(argv[10], "--time") == 0 ||
                    strcmp(argv[10], "-time") == 0
                    ) {
                        maxTime = atoi(argv[11]);
                } else if (
                    strcmp(argv[10], "-b") == 0 ||
                    strcmp(argv[10], "--book") == 0 ||
                    strcmp(argv[10], "-book") == 0
                    ) {
                        useBook = 1;
                }
            }

            if (argc > 12) {
                if (
                    strcmp(argv[12], "-t") == 0 ||
                    strcmp(argv[12], "--time") == 0 ||
                    strcmp(argv[12], "-time") == 0
                    ) {
                        maxTime = atoi(argv[13]);
                } else if (
                    strcmp(argv[12], "-d") == 0 ||
                    strcmp(argv[12], "--depth") == 0 ||
                    strcmp(argv[12], "-depth") == 0
                    ) {
                        maxDepth = atoi(argv[13]);
                } else if (
                    strcmp(argv[12], "-b") == 0 ||
                    strcmp(argv[12], "--book") == 0 ||
                    strcmp(argv[12], "-book") == 0
                    ) {
                        useBook = 1;
                }
            }

            printf("max time: %d\n", maxTime);
            printf("max depth: %d\n", maxDepth);
            printf("use book: %d\n", useBook);

            if (useBook) {
                initBook();
            }

            printf("play mode entered\n");
            printf("you play as %s\n", isPlayerWhite ? "white" : "black");
            printf("to make a move type it in the format: e2e4\n");
            printf("to quit type: quit [WARNING: NOT QUITTING CAN CAUSE MEMORY LEAKS]\n");
            printf("on promoting, you will be asked to enter a piece type in this format:\n");
            printf("q for queen, r for rook, b for bishop, n for knight\n");
            printf("continue? (1/0)\n");

            int cont = 0;
            scanf("%d", &cont);
            if (!cont) {
                return 0;
            }

            if (!isPlayerWhite) {
                engineMove(!isPlayerWhite);
            }

            while (true) {
                printf("your move: ");
                char notationMove[4];
                scanf("%s", &notationMove);
                if (strcmp(notationMove, "quit") == 0) {
                    free(transTable);
                    free(quietTable);
                    if (useBook) {
                        free(bookEntries);
                    }
                    return 0;
                }
                struct Move move = buildMove(notationMove, bitboards);

                struct Move* legalMoves;
                if (isPlayerWhite) {
                    legalMoves = possiblemoves(
                        isPlayerWhite, 
                        bitboards.allPieces, bitboards.allPieces90, bitboards.allPieces45R, bitboards.allPieces45L, bitboards.enPassantSquare, bitboards.whitePieces, bitboards.blackPieces, 
                        bitboards.whitePawns, bitboards.whiteKnights, bitboards.whiteBishops, bitboards.whiteRooks, bitboards.whiteQueens, bitboards.whiteKing, 
                        bitboards.whiteCastleQueenSide, bitboards.whiteCastleKingSide
                        );
                } else {
                    legalMoves = possiblemoves(
                        isPlayerWhite, 
                        bitboards.allPieces, bitboards.allPieces90, bitboards.allPieces45R, bitboards.allPieces45L, bitboards.enPassantSquare, bitboards.blackPieces, bitboards.whitePieces, 
                        bitboards.blackPawns, bitboards.blackKnights, bitboards.blackBishops, bitboards.blackRooks, bitboards.blackQueens, bitboards.blackKing, 
                        bitboards.blackCastleQueenSide, bitboards.blackCastleKingSide
                        );
                }
                bool isLegal = false;
                for (int i = 1; i <= legalMoves[0].from - 1; i++) {
                    if (
                        legalMoves[i].from == move.from &&
                        legalMoves[i].to == move.to &&
                        legalMoves[i].promotesTo == move.promotesTo &&
                        legalMoves[i].isEnPassantCapture == move.isEnPassantCapture &&
                        legalMoves[i].castle == move.castle &&
                        legalMoves[i].isEnPassantCapture == move.isEnPassantCapture &&
                        legalMoves[i].pieceType == move.pieceType
                        ) {
                            Bitboards testBoards = doMove(move, bitboards, isPlayerWhite);
                            if (isPlayerWhite ? (!canOpponentCaptureKing(isPlayerWhite, bitboards.allPieces, bitboards.allPieces90, bitboards.allPieces45R, bitboards.allPieces45L, bitboards.whiteKing, bitboards.blackKing, bitboards.blackQueens, bitboards.blackRooks, bitboards.blackBishops, bitboards.blackKnights, bitboards.blackPawns)) : 
                                                (!canOpponentCaptureKing(isPlayerWhite, bitboards.allPieces, bitboards.allPieces90, bitboards.allPieces45R, bitboards.allPieces45L, bitboards.blackKing, bitboards.whiteKing, bitboards.whiteQueens, bitboards.whiteRooks, bitboards.whiteBishops, bitboards.whiteKnights, bitboards.whitePawns))
                                ) {
                                isLegal = true;
                            }
                            break;
                        }
                }
                free(legalMoves);
                if (isLegal) {
                    printf("updated board:\n");
                    updateFenClocks(move);
                    bitboards = doMove(move, bitboards, isPlayerWhite);
                    printBoard(bitboards, !isPlayerWhite);
                    printBinary(bitboards.hash);
                    engineMove(!isPlayerWhite);
                } else {
                    printf("illegal move\n");
                }
            }

            return 0;
        } 
        else if (
            strcmp(argv[1], "-p") == 0 ||
            strcmp(argv[1], "--perft") == 0 ||
            strcmp(argv[1], "-perft") == 0
        ) {
            if (argc < 3) {
                    printf("please provide a fen string\n");
                    return 0;
                }
            int isWhite = strcmp(argv[3], "w") == 0 ? 1 : 0;
            int startPosition[8][8] = {0};
            fenToPosition(argv[2], startPosition);
            initBoards(startPosition, isWhite, argv[4], argv[5], 0, 0);
            printBoard(bitboards, isWhite);

            printf("depth: ");
            int depth;
            scanf("%d", &depth);

            clock_t start = clock();
            int bulk = perft(isWhite, depth, bitboards);
            // for (int i = 0; i < 12524004; i++) {
            //     // evaluate(bitboards); // 4147.00 milliseconds
            //     evaluateEarly(bitboards.whitePawns, bitboards.whiteKnights, bitboards.whiteBishops, bitboards.whiteRooks, bitboards.whiteQueens, bitboards.whiteKing, bitboards.blackPawns, bitboards.blackKnights, bitboards.blackBishops, bitboards.blackRooks, bitboards.blackQueens, bitboards.blackKing);
            // }
            // struct Move move = {11, 27, 0, 0, 0, 0, 0};
            // for (int i = 0; i < 13073969; i++) {
            //     Bitboards newBoard = doMove(move, bitboards, 1); // 1129.00 milliseconds
            // }
            // for (int i = 0; i < 2876770; i++) {
            //     struct Move *pos = possiblemoves(1, bitboards.allPieces, bitboards.allPieces90, bitboards.allPieces45R, bitboards.allPieces45L, bitboards.enPassantSquare, bitboards.whitePieces, bitboards.blackPieces, bitboards.whitePawns, bitboards.whiteKnights, bitboards.whiteBishops, bitboards.whiteRooks, bitboards.whiteQueens, bitboards.whiteKing, bitboards.whiteCastleQueenSide, bitboards.whiteCastleKingSide);
            //     free(pos);  // 5152.00 milliseconds - moves
            //                 // 1931.00 milliseconds - captures
            // }
            clock_t end = clock();
            printf("bulk: %d\n", bulk);
            printf("Elapsed time: %.2f milliseconds\n", (double)(end - start) / CLOCKS_PER_SEC * 1000);
        } else {
            printLogo();
            printHelp();
            printf("unknown command, press any key to exit...\n");
            getchar();
        }
    } else {
        printLogo();
        printHelp();
        printf("press any key to exit...\n");
        getchar();
    }
    
    return 0;
}

// some benchmark

// nodes searched: 12883075
// Elapsed time: 13311.00 milliseconds
// nodes per second: 967851