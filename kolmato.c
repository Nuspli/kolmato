#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <time.h>
#include "inttypes.h"
#include "math.h"

// DEFINITIONS AND MACROS

#define KINGSIDE 1
#define QUEENSIDE 2

#define EXACT 0
#define LOWERBOUND 1
#define UPPERBOUND 2

#define INF 100000

// #define TABLE_SIZE 2097152 // 2^21 = 2 MB
#define TABLE_SIZE 4194304 // 2^22 = 4 MB
// #define TABLE_SIZE 8388608 // 2^23 = 8 MB
#define BOOK_SIZE 2000000
#define MAX_NUM_MOVES 256

enum {
    h1, g1, f1, e1, d1, c1, b1, a1,
    h2, g2, f2, e2, d2, c2, b2, a2,
    h3, g3, f3, e3, d3, c3, b3, a3,
    h4, g4, f4, e4, d4, c4, b4, a4,
    h5, g5, f5, e5, d5, c5, b5, a5,
    h6, g6, f6, e6, d6, c6, b6, a6,
    h7, g7, f7, e7, d7, c7, b7, a7,
    h8, g8, f8, e8, d8, c8, b8, a8
};

#define u64 uint64_t
#define u8 uint8_t

#define bitCount(num) __builtin_popcountll(num)
// Count the number of bits set to 1

#define lsb(num) __builtin_ctzll(num)
// Get the index of the least significant bit set to 1

#define setBit(num, index) num |= (1ULL << index)
// Set the bit at the given index to 1

#define flipBit(num, index) num ^= (1ULL << index)
// Set the bit at the given index to 0

#define checkBit(num, index) num & (1ULL << index)
// Check if the bit at the given index is set to 1

#define bit(index) 1ULL << index
// Get a bitboard with only the bit at the given index set to 1

#define tableEntry(table, hash) ((table)->entries + ((hash) & (table)->mask))

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

void printBinary(u64 x) {
    // print the binary representation of a 64 bit integer
    for (int i = 63; i >= 0; i--) {
        u64 mask = 1ULL << i;  // Create a mask with the i-th bit set to 1
        int bit = (x & mask) ? 1 : 0;  // Extract the i-th bit from n

        printf("%d", bit);
    }
    printf("\n");
}

// BASIC FUNCTIONS

bool stringContainsChar(char* string, char c) {
    return strchr(string, c) != NULL;
}

int min(int x, int y) {
    return x < y ? x : y;
}

int max(int x, int y) {
    return x > y ? x : y;
}

unsigned int randomNum = 1415926535;

unsigned int randInt() {
    // pseudo random integer
	unsigned int x = randomNum;
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
	randomNum = x;
	return x;
}

u64 rand64() {
    u64 u1, u2, u3, u4;
    
    u1 = (u64)(randInt()) & 0xFFFF;
    u2 = (u64)(randInt()) & 0xFFFF;
    u3 = (u64)(randInt()) & 0xFFFF;
    u4 = (u64)(randInt()) & 0xFFFF;
    
    return u1 | (u2 << 16) | (u3 << 32) | (u4 << 48);
}

u64 Rand64FewBits() {
    return rand64() & rand64() & rand64();
}

// STATIC DATA

static u64 rightmostFileMask = 0x0101010101010101;
static u64 leftmostFileMask = 0x8080808080808080;
static u64 whitePawnStartRank = 0x0000000000FF0000; // (1 above actually)
static u64 blackPawnStartRank = 0x0000FF0000000000;
static u64 pawnPromotionMask = 0xFF000000000000FF;

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
    300, 300, 300, 300, 300, 300, 300, 300,
    300, 250, 250, 250, 250, 250, 250, 300,
    300, 250, 150, 150, 150, 150, 250, 300,
    300, 250, 150,  50,  50, 150, 250, 300,
    300, 250, 150,  50,  50, 150, 250, 300, 
    300, 250, 150, 150, 150, 150, 250, 300,
    300, 250, 250, 250, 250, 250, 250, 300,
    300, 300, 300, 300, 300, 300, 300, 300
    };

static int rookRelevantOccupancyAmount[64] = {
    12,11,11,11,11,11,11,12,
    11,10,10,10,10,10,10,11,
    11,10,10,10,10,10,10,11,
    11,10,10,10,10,10,10,11,
    11,10,10,10,10,10,10,11,
    11,10,10,10,10,10,10,11,
    11,10,10,10,10,10,10,11,
    12,11,11,11,11,11,11,12
};

static int bishopRelevantOccupancyAmount[64] = {
     6, 5, 5, 5, 5, 5, 5, 6,
     5, 5, 5, 5, 5, 5, 5, 5,
     5, 5, 7, 7, 7, 7, 5, 5,
     5, 5, 7, 9, 9, 7, 5, 5,
     5, 5, 7, 9, 9, 7, 5, 5,
     5, 5, 7, 7, 7, 7, 5, 5,
     5, 5, 5, 5, 5, 5, 5, 5,
     6, 5, 5, 5, 5, 5, 5, 6
};

static u64 bishopAttackMasks[64] = {
    0x0040201008040200ULL, 0x0000402010080400ULL, 0x0000004020100a00ULL, 0x0000000040221400ULL, 0x0000000002442800ULL, 0x0000000204085000ULL, 0x0000020408102000ULL, 0x0002040810204000ULL,
    0x0020100804020000ULL, 0x0040201008040000ULL, 0x00004020100a0000ULL, 0x0000004022140000ULL, 0x0000000244280000ULL, 0x0000020408500000ULL, 0x0002040810200000ULL, 0x0004081020400000ULL, 
    0x0010080402000200ULL, 0x0020100804000400ULL, 0x004020100a000a00ULL, 0x0000402214001400ULL, 0x0000024428002800ULL, 0x0002040850005000ULL, 0x0004081020002000ULL, 0x0008102040004000ULL,
    0x0008040200020400ULL, 0x0010080400040800ULL, 0x0020100a000a1000ULL, 0x0040221400142200ULL, 0x0002442800284400ULL, 0x0004085000500800ULL, 0x0008102000201000ULL, 0x0010204000402000ULL, 
    0x0004020002040800ULL, 0x0008040004081000ULL, 0x00100a000a102000ULL, 0x0022140014224000ULL, 0x0044280028440200ULL, 0x0008500050080400ULL, 0x0010200020100800ULL, 0x0020400040201000ULL, 
    0x0002000204081000ULL, 0x0004000408102000ULL, 0x000a000a10204000ULL, 0x0014001422400000ULL, 0x0028002844020000ULL, 0x0050005008040200ULL, 0x0020002010080400ULL, 0x0040004020100800ULL, 
    0x0000020408102000ULL, 0x0000040810204000ULL, 0x00000a1020400000ULL, 0x0000142240000000ULL, 0x0000284402000000ULL, 0x0000500804020000ULL, 0x0000201008040200ULL, 0x0000402010080400ULL, 
    0x0002040810204000ULL, 0x0004081020400000ULL, 0x000a102040000000ULL, 0x0014224000000000ULL, 0x0028440200000000ULL, 0x0050080402000000ULL, 0x0020100804020000ULL, 0x0040201008040200ULL
};

static u64 rookAttackMasks[64] = {
    0x000101010101017eULL, 0x000202020202027cULL, 0x000404040404047aULL, 0x0008080808080876ULL, 0x001010101010106eULL, 0x002020202020205eULL, 0x004040404040403eULL, 0x008080808080807eULL, 
    0x0001010101017e00ULL, 0x0002020202027c00ULL, 0x0004040404047a00ULL, 0x0008080808087600ULL, 0x0010101010106e00ULL, 0x0020202020205e00ULL, 0x0040404040403e00ULL, 0x0080808080807e00ULL, 
    0x00010101017e0100ULL, 0x00020202027c0200ULL, 0x00040404047a0400ULL, 0x0008080808760800ULL, 0x00101010106e1000ULL, 0x00202020205e2000ULL, 0x00404040403e4000ULL, 0x00808080807e8000ULL, 
    0x000101017e010100ULL, 0x000202027c020200ULL, 0x000404047a040400ULL, 0x0008080876080800ULL, 0x001010106e101000ULL, 0x002020205e202000ULL, 0x004040403e404000ULL, 0x008080807e808000ULL, 
    0x0001017e01010100ULL, 0x0002027c02020200ULL, 0x0004047a04040400ULL, 0x0008087608080800ULL, 0x0010106e10101000ULL, 0x0020205e20202000ULL, 0x0040403e40404000ULL, 0x0080807e80808000ULL, 
    0x00017e0101010100ULL, 0x00027c0202020200ULL, 0x00047a0404040400ULL, 0x0008760808080800ULL, 0x00106e1010101000ULL, 0x00205e2020202000ULL, 0x00403e4040404000ULL, 0x00807e8080808000ULL, 
    0x007e010101010100ULL, 0x007c020202020200ULL, 0x007a040404040400ULL, 0x0076080808080800ULL, 0x006e101010101000ULL, 0x005e202020202000ULL, 0x003e404040404000ULL, 0x007e808080808000ULL, 
    0x7e01010101010100ULL, 0x7c02020202020200ULL, 0x7a04040404040400ULL, 0x7608080808080800ULL, 0x6e10101010101000ULL, 0x5e20202020202000ULL, 0x3e40404040404000ULL, 0x7e80808080808000ULL
};

static u64 rookMagics[64] = {
    0x0a8002c000108020ULL, 0x06c00049b0002001ULL, 0x0100200010090040ULL, 0x2480041000800801ULL, 
    0x0280028004000800ULL, 0x0900410008040022ULL, 0x0280020001001080ULL, 0x2880002041000080ULL, 
    0xa000800080400034ULL, 0x0004808020004000ULL, 0x2290802004801000ULL, 0x0411000d00100020ULL, 
    0x0402800800040080ULL, 0x000b000401004208ULL, 0x2409000100040200ULL, 0x0001002100004082ULL, 
    0x0022878001e24000ULL, 0x1090810021004010ULL, 0x0801030040200012ULL, 0x0500808008001000ULL, 
    0x0a08018014000880ULL, 0x8000808004000200ULL, 0x0201008080010200ULL, 0x0801020000441091ULL, 
    0x0000800080204005ULL, 0x1040200040100048ULL, 0x0000120200402082ULL, 0x0d14880480100080ULL, 
    0x0012040280080080ULL, 0x0100040080020080ULL, 0x9020010080800200ULL, 0x0813241200148449ULL, 
    0x0491604001800080ULL, 0x0100401000402001ULL, 0x4820010021001040ULL, 0x0400402202000812ULL, 
    0x0209009005000802ULL, 0x0810800601800400ULL, 0x4301083214000150ULL, 0x204026458e001401ULL, 
    0x0040204000808000ULL, 0x8001008040010020ULL, 0x8410820820420010ULL, 0x1003001000090020ULL, 
    0x0804040008008080ULL, 0x0012000810020004ULL, 0x1000100200040208ULL, 0x430000a044020001ULL, 
    0x0280009023410300ULL, 0x00e0100040002240ULL, 0x0000200100401700ULL, 0x2244100408008080ULL, 
    0x0008000400801980ULL, 0x0002000810040200ULL, 0x8010100228810400ULL, 0x2000009044210200ULL, 
    0x4080008040102101ULL, 0x0040002080411d01ULL, 0x2005524060000901ULL, 0x0502001008400422ULL, 
    0x489a000810200402ULL, 0x0001004400080a13ULL, 0x4000011008020084ULL, 0x0026002114058042ULL
};

static u64 bishopMagics[64] = {
    0x89a1121896040240ULL, 0x2004844802002010ULL, 0x2068080051921000ULL, 0x62880a0220200808ULL, 
    0x0004042004000000ULL, 0x0100822020200011ULL, 0xc00444222012000aULL, 0x0028808801216001ULL, 
    0x0400492088408100ULL, 0x0201c401040c0084ULL, 0x00840800910a0010ULL, 0x0000082080240060ULL, 
    0x2000840504006000ULL, 0x30010c4108405004ULL, 0x1008005410080802ULL, 0x8144042209100900ULL, 
    0x0208081020014400ULL, 0x004800201208ca00ULL, 0x0f18140408012008ULL, 0x1004002802102001ULL, 
    0x0841000820080811ULL, 0x0040200200a42008ULL, 0x0000800054042000ULL, 0x88010400410c9000ULL, 
    0x0520040470104290ULL, 0x1004040051500081ULL, 0x2002081833080021ULL, 0x000400c00c010142ULL, 
    0x941408200c002000ULL, 0x0658810000806011ULL, 0x0188071040440a00ULL, 0x4800404002011c00ULL, 
    0x0104442040404200ULL, 0x0511080202091021ULL, 0x0004022401120400ULL, 0x80c0040400080120ULL, 
    0x8040010040820802ULL, 0x0480810700020090ULL, 0x0102008e00040242ULL, 0x0809005202050100ULL, 
    0x8002024220104080ULL, 0x0431008804142000ULL, 0x0019001802081400ULL, 0x0200014208040080ULL, 
    0x3308082008200100ULL, 0x041010500040c020ULL, 0x4012020c04210308ULL, 0x208220a202004080ULL, 
    0x0111040120082000ULL, 0x6803040141280a00ULL, 0x2101004202410000ULL, 0x8200000041108022ULL, 
    0x0000021082088000ULL, 0x0002410204010040ULL, 0x0040100400809000ULL, 0x0822088220820214ULL, 
    0x0040808090012004ULL, 0x00910224040218c9ULL, 0x0402814422015008ULL, 0x0090014004842410ULL, 
    0x0001000042304105ULL, 0x0010008830412a00ULL, 0x2520081090008908ULL, 0x40102000a0a60140ULL
};

// STRUCTS

typedef struct Bitboards {
    // each bitboard represents a different piece type
    // each bit represents a square on the board
    // 1 means the piece is on that square, 0 means it is not
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
    
    // castling flags and en passant square
    bool whiteCastleKingSide;
    bool whiteCastleQueenSide;
    
    bool blackCastleKingSide;
    bool blackCastleQueenSide;
    
    u64 enPassantSquare;
    // hash of the current position to use for transposition tables
    u64 hash;
    bool color;
    int pieceList[64];

} Bitboards;

typedef struct Move {
    // a move is represented by the starting square, ending square,
    // piece type and any other information needed to modify the bitboards accordingly
    u8 from;
    u8 to;
    u8 pieceType;
    u8 castle;
    u8 isEnPassantCapture;
    u8 createsEnPassant;
    u8 promotesTo;
} Move;

typedef struct BookEntry {
    u64 hash;
    Move* moves;
    int* occourences;
    u8 numMoves;
} BookEntry;

typedef struct Book {
    BookEntry* entries;
    int numEntries;
} Book;

typedef struct HashEntry {
    // a hash entry is a position that has been evaluated before
    // it is used to avoid evaluating the same position multiple times
    u64 hash;
    int8_t depth;
    int value;
    u8 flag;
    Move bestMove;
} HashEntry;

typedef struct Table {
    HashEntry *entries;
    int mask;
    u8 capacity;
} Table;

// GLOBAL VARIABLES

struct Bitboards bitboards = {
    0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0,
    0, 0, 0,
    false, false,
    false, false,
    0, 0,
    true,
    {0}
};

// lookup tables for piece attack patterns
u64 knightAttacks[64];
u64 kingAttacks[64];
u64 bishopAttacks[64][512];
u64 rookAttacks[64][4096];

// tables used for hashing
u64 ZOBRIST_TABLE[64][13]; // 64 squares, 12 pieces and 1 en passant
u64 whiteToMove;
u64 castlingRights[4];

// one transposition table for the main search, one for the quiescence search
struct Table *transTable;
// test fen for transposition table: 8/k7/3p4/p2P1p2/P2P1P2/8/8/K7 w - - 0 1
struct Table *quietTable;

void tableSetEntry(struct Table *table, u64 hash, int8_t depth, int value, u8 flag) {
    HashEntry *entry = tableEntry(table, hash);
    if (entry->depth <= depth) {
        entry->hash = hash;
        entry->depth = depth;
        entry->value = value;
        entry->flag = flag;
    }
}

void tableSetMove(struct Table *table, u64 hash, int8_t depth, Move *move) {
    HashEntry *entry = tableEntry(table, hash);
    if (entry->depth <= depth) {
        entry->hash = hash;
        entry->depth = depth;
        memcpy(&entry->bestMove, move, sizeof(Move));
    }
}

int tableGetEntry(struct Table *table, u64 hash, int8_t depth, int *value, int alpha, int beta) {
    HashEntry *entry = tableEntry(table, hash);
    if (entry->hash == hash && entry->depth >= depth) {
        if ((entry->flag == EXACT) || 
            (entry->flag == UPPERBOUND && entry->value <= alpha) ||
            (entry->flag == LOWERBOUND && entry->value >= beta)) {
            *value = entry->value;
            return 1;
        }
    }
    return 0;
}

Move *tableGetMove(struct Table *table, u64 hash) {
    HashEntry *entry = tableEntry(table, hash);
    if (entry->hash == hash) {
        return &entry->bestMove;
    }
    return NULL;
}

// struct Table *quietTable;

// function call counters
int possibleCalls = 0;
int moveCalls = 0;
int evalCalls = 0;
int quiescenceCalls = 0;

int nodes = 0;
int quietNodes = 0;
int visits = 0; // number of nodes visited after a specific move
int quietVisits = 0;
int transpositions = 0;
int quietTranspositions = 0;

int moveCount = 0;
int halfMoveCount = 0;

int lastfrom = -1; // maintained during the game to avoid repetition
int lastto = -1; // todo use a game state table with the hashes instead

int maxTime;
int maxDepth;

bool useBook = false;

// IN AND OUTPUT TRANSLATION

void fenToPosition(char* fen, int position[64]) {
    
    int rank = 0;  // Start from rank 8 and go down to rank 1
    int file = 0;  // Start from file a and go to file h

    // Loop through the FEN string
    for (int i = 0; fen[i] != '\0'; i++) {
        char c = fen[i];
        int piece;

        // Map FEN characters to piece values
        switch (c) {
            case 'p':
                piece = -1;
                break;
            case 'n':
                piece = -2;
                break;
            case 'b':
                piece = -3;
                break;
            case 'r':
                piece = -4;
                break;
            case 'q':
                piece = -5;
                break;
            case 'k':
                piece = -6;
                break;
            case 'P':
                piece = 1;
                break;
            case 'N':
                piece = 2;
                break;
            case 'B':
                piece = 3;
                break;
            case 'R':
                piece = 4;
                break;
            case 'Q':
                piece = 5;
                break;
            case 'K':
                piece = 6;
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
        position[rank * 8 + file] = piece;
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

    if (((bitboards.whitePawns & bit(from)) != 0) || ((bitboards.blackPawns & bit(from)) != 0)) {
        pieceType = 0;
        if (abs(from - to) == 16) {
            createsEnPassant = 1;
        } else if (bitboards.enPassantSquare & bit(to)) {
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
    } else if (((bitboards.whiteKnights & bit(from)) != 0) || ((bitboards.blackKnights & bit(from)) != 0)) {
        pieceType = 1;
    } else if (((bitboards.whiteBishops & bit(from)) != 0) || ((bitboards.blackBishops & bit(from)) != 0)) {
        pieceType = 2;
    } else if (((bitboards.whiteRooks & bit(from)) != 0) || ((bitboards.blackRooks & bit(from)) != 0)) {
        pieceType = 3;
    } else if (((bitboards.whiteQueens & bit(from)) != 0) || ((bitboards.blackQueens & bit(from)) != 0)) {
        pieceType = 4;
    } else if (((bitboards.whiteKing & bit(from)) != 0) || ((bitboards.blackKing & bit(from)) != 0)) {
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

void printBoard(struct Bitboards boards) {
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
    printf("\n");
    for (int i = 0; i < 8; i++) {
        printf("    +---+---+---+---+---+---+---+---+\n");
        printf("  %d ", 8 - i);
        for (int j = 0; j < 8; j++) {
            printf("| %c ", board[i][j]);
        }
        printf("|\n");
    }
    printf("    +---+---+---+---+---+---+---+---+\n");
    printf("      a   b   c   d   e   f   g   h  \n");
    printf("\n");

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
            while (!(checkBit(boards.allPieces, i))) {
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

    printf(" %c ", boards.color == 1 ? 'w' : 'b');

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

    printf(" %d %d\nhash: ", halfMoveCount, moveCount % 2 == 0 ? (moveCount / 2) + 1 : (moveCount + 1) / 2);
    printBinary(bitboards.hash);

    printf("\n\n");
}

void displayBoard(u64 bitboard) {
    // displays a single bitboard
    int b = 63;
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            if (checkBit(bitboard, b)) {
                printf("X ");
            } else {
                printf(". ");
            }
            b--;
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

void initTables() {
    // allocate memory for the transposition tables and initialize them to 0
    transTable = calloc(1, sizeof(struct Table));
    transTable->mask = TABLE_SIZE - 1;
    transTable->entries = calloc(TABLE_SIZE, sizeof(struct HashEntry));

    if (transTable->entries == NULL || transTable == NULL) {
        printf("Error: failed to allocate memory for transposition table\n");
        exit(1);
    }
    
    quietTable = calloc(1, sizeof(struct Table));
    quietTable->mask = TABLE_SIZE - 1;
    quietTable->entries = calloc(TABLE_SIZE, sizeof(struct HashEntry));

    if (quietTable->entries == NULL || quietTable == NULL) {
        printf("Error: failed to allocate memory for quiet table\n");
        exit(1);
    }
}

void freeTables() {
    // free the memory allocated for the transposition table
    free(transTable->entries);
    free(transTable);
    free(quietTable->entries);
    free(quietTable);
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

u64 initBoardHash(struct Bitboards BITBOARDS, bool isWhiteToMove) {
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
                setBit(bitboards.blackPawns, b);}
            else if (piece == -2) {
                setBit(bitboards.blackKnights, b);}
            else if (piece == -3) {
                setBit(bitboards.blackBishops, b);}
            else if (piece == -4) {
                setBit(bitboards.blackRooks, b);}
            else if (piece == -5) {
                setBit(bitboards.blackQueens, b);}
            else if (piece == -6) {
                setBit(bitboards.blackKing, b);}
            else if (piece == 1) {
                setBit(bitboards.whitePawns, b);}
            else if (piece == 2) {
                setBit(bitboards.whiteKnights, b);}
            else if (piece == 3) {
                setBit(bitboards.whiteBishops, b);}
            else if (piece == 4) {
                setBit(bitboards.whiteRooks, b);}
            else if (piece == 5) {
                setBit(bitboards.whiteQueens, b);}
            else if (piece == 6) {
                setBit(bitboards.whiteKing, b);}
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

// precomputed move lookup tables
void initKnightAttacks() {
    for (int i = 0; i < 64; i++) {
        // knight attacks from each of the 64 squares
        // look at the "normal board" somewhere above for more clarity
        u64 knightboard = 0;
        if (i % 8 < 6 && i / 8 != 7) {setBit(knightboard, i+10);} // consitions are to make sure the knight doesn't go off the board
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

u64 normBishopAttacks(int square, u64 occupied) {
    u64 attacks = 0ULL;
    
    int targetFile, targetRank;
    
    int rank = square / 8;
    int file = square % 8;
    
    for (targetRank = rank + 1, targetFile = file + 1; targetRank <= 7 && targetFile <= 7; targetRank++, targetFile++) { // up left
        attacks |= (1ULL << (targetRank * 8 + targetFile));
        if (occupied & (1ULL << (targetRank * 8 + targetFile))) {break;}
    }
    
    for (targetRank = rank + 1, targetFile = file - 1; targetRank <= 7 && targetFile >= 0; targetRank++, targetFile--) { // up right
        attacks |= (1ULL << (targetRank * 8 + targetFile));
        if (occupied & (1ULL << (targetRank * 8 + targetFile))) {break;}
    }
    
    for (targetRank = rank - 1, targetFile = file + 1; targetRank >= 0 && targetFile <= 7; targetRank--, targetFile++) { // down left
        attacks |= (1ULL << (targetRank * 8 + targetFile));
        if (occupied & (1ULL << (targetRank * 8 + targetFile))) {break;}
    }
    
    for (targetRank = rank - 1, targetFile = file - 1; targetRank >= 0 && targetFile >= 0; targetRank--, targetFile--) { // down right
        attacks |= (1ULL << (targetRank * 8 + targetFile));
        if (occupied & (1ULL << (targetRank * 8 + targetFile))) {break;}
    }
    return attacks;
}

u64 normRookAttacks(int square, u64 occupied) {
    u64 attacks = 0ULL;
    
    int targetFile, targetRank;
    
    int rank = square / 8;
    int file = square % 8;
    
    for (targetRank = rank + 1; targetRank <= 7; targetRank++) {
        attacks |= (1ULL << (targetRank * 8 + file));
        if (occupied & (1ULL << (targetRank * 8 + file))) {break;}
    }
    
    for (targetRank = rank - 1; targetRank >= 0; targetRank--) {
        attacks |= (1ULL << (targetRank * 8 + file));
        if (occupied & (1ULL << (targetRank * 8 + file))) {break;}
    }
    
    for (targetFile = file + 1; targetFile <= 7; targetFile++) {
        attacks |= (1ULL << (rank * 8 + targetFile));
        if (occupied & (1ULL << (rank * 8 + targetFile))) {break;}
    }
    
    for (targetFile = file - 1; targetFile >= 0; targetFile--) {
        attacks |= (1ULL << (rank * 8 + targetFile));
        if (occupied & (1ULL << (rank * 8 + targetFile))) {break;}
    }
    
    return attacks;
}

u64 setOccupancy(int index, int maskBits, u64 attackMask) {
    u64 occupancy = 0ULL;
    for (int count = 0; count < maskBits; count++) {
        int square = lsb(attackMask);
        flipBit(attackMask, square);
        
        if (index & (1 << count))
            occupancy |= (1ULL << square);
    }
    return occupancy;
}

void initSlidingPieceAttacks(bool isRook) {
    for (int square = 0; square < 64; square++) {
        u64 mask = isRook ? rookAttackMasks[square] : bishopAttackMasks[square];
        
        int bitCnt = bitCount(mask);
        
        int occupancy_variations = 1 << bitCnt;
        
        for (int count = 0; count < occupancy_variations; count++) {
            if (isRook) {
                u64 occupancy = setOccupancy(count, bitCnt, mask);
                u64 magicIndex = occupancy * rookMagics[square] >> 64 - rookRelevantOccupancyAmount[square];
                rookAttacks[square][magicIndex] = normRookAttacks(square, occupancy);
            } else {
                u64 occupancy = setOccupancy(count, bitCnt, mask);
                u64 magicIndex = occupancy * bishopMagics[square] >> 64 - bishopRelevantOccupancyAmount[square];
                bishopAttacks[square][magicIndex] = normBishopAttacks(square, occupancy);
            }
        }
    }
}

u64 getMagic(int square, int relevantOccupiedBitAmount, bool isRook) {
    u64 occupancies[4096];
    u64 attacks[4096];
    u64 attacksInUse[4096];
    u64 attackMask = isRook ? rookAttackMasks[square] : bishopAttackMasks[square];

    int occupancyVariations = 1 << relevantOccupiedBitAmount;

    for (int count = 0; count < occupancyVariations; count++) {
        occupancies[count] = setOccupancy(count, relevantOccupiedBitAmount, attackMask);
        attacks[count] = isRook ? normRookAttacks(square, occupancies[count]) : normBishopAttacks(square, occupancies[count]);
    }

    // test magic numbers
    for (int i = 0; i < 100000000; i++) {
        u64 magic = Rand64FewBits();

        // skip testing magic number if inappropriate
        if(bitCount((attackMask * magic) & 0xFF00000000000000ULL) < 6) {continue;}

        // reset used attacks array
        memset(attacksInUse, 0ULL, sizeof(attacksInUse));

        int count, fail;
        
        // test magic index
        for (count = 0, fail = 0; !fail && count < occupancyVariations; count++) {
            int magic_index = (int)((occupancies[count] * magic) >> (64 - relevantOccupiedBitAmount));
          
            if (attacksInUse[magic_index] == 0ULL) {
                attacksInUse[magic_index] = attacks[count];
            } else if (attacksInUse[magic_index] != attacks[count]) {fail = 1;}
        }

        if (!fail) {return magic;}
    }
    
    printf("Error: failed to generate magic numbers\n");
    return 0ULL;
}

void generateNewMagics() {
    printf("static u64 rookMagics[64] = {\n");
    
    for(int square = 0; square < 63; square++) {
        printf("    0x%llxULL,\n", getMagic(square, rookRelevantOccupancyAmount[square], true));
    }
    printf("   0x%llxULL\n", getMagic(63, rookRelevantOccupancyAmount[63], true));
    
    printf("};\n\nstatic u64 bishopMagics[64] = {\n");
    
    for(int square = 0; square < 63; square++) {
        printf("    0x%llxULL,\n", getMagic(square, bishopRelevantOccupancyAmount[square], false));
    }
    printf("   0x%llxULL\n", getMagic(63, bishopRelevantOccupancyAmount[63], false));

    printf("};\n\n");
}

// MOVE GENERATION

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

void pushMove(struct Move **moveArray, u8 from, u8 to, u8 pieceType, u8 castle, u8 isEnPassantCapture, u8 createsEnPassant, u8 promotesTo, int *index) {
    // function to add a move to the move array
    struct Move move;
    move.from = from;
    move.to = to;
    move.pieceType = pieceType;
    move.castle = castle;
    move.isEnPassantCapture = isEnPassantCapture;
    move.createsEnPassant = createsEnPassant;
    move.promotesTo = promotesTo;

    (*moveArray)[*index] = move;
    *index += 1;
}
                        
int possiblemoves(
        bool isWhiteToMove,
        u64 occupied, u64 epSquare, u64 myPieces, u64 enemyPieces, 
        u64 pawns, u64 knights, u64 bishops, u64 rooks, u64 queens, u64 king, 
        bool castleQueen, bool castleKing, Move *MOVES
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
        pushMove(&MOVES, kingIndex, b, 5, 0, 0, 0, 0, &count);
    }
    // castling is only available when the flags are set and the squares are empty, check detection is done in the search tree
    if (castleKing && ((!(checkBit(occupied, kingIndex-1))) && (!(checkBit(occupied, kingIndex-2))) && (checkBit(rooks, kingIndex-3)))) {
        pushMove(&MOVES, kingIndex, kingIndex-2, 5, KINGSIDE, 0, 0, 0, &count);
    }
    if (castleQueen && (!(checkBit(occupied, kingIndex+1)) && (!(checkBit(occupied, kingIndex+2))) && (!(checkBit(occupied, kingIndex+3))) && (checkBit(rooks, kingIndex+4)))) {
        pushMove(&MOVES, kingIndex, kingIndex+2, 5, QUEENSIDE, 0, 0, 0, &count);
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
            pushMove(&MOVES, knightIndex, b, 1, 0, 0, 0, 0, &count);
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
            pushMove(&MOVES, bishopIndex, b, 2, 0, 0, 0, 0, &count);
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
            pushMove(&MOVES, rookIndex, b, 3, 0, 0, 0, 0, &count);
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
            pushMove(&MOVES, queenIndex, b, 4, 0, 0, 0, 0, &count);
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
            pushMove(&MOVES, b + (8 * sign), b, 0, 0, 0, 0, 4, &count);
            pushMove(&MOVES, b + (8 * sign), b, 0, 0, 0, 0, 3, &count);
            pushMove(&MOVES, b + (8 * sign), b, 0, 0, 0, 0, 2, &count);
            pushMove(&MOVES, b + (8 * sign), b, 0, 0, 0, 0, 1, &count);
        } else {
            pushMove(&MOVES, b + (8 * sign), b, 0, 0, 0, 0, 0, &count);
        }
    }

    while (step2) {
        u8 b = lsb(step2);
        flipBit(step2, b);
        pushMove(&MOVES, b + (16 * sign), b, 0, 0, 0, 1, 0, &count);
    }

    while (capture1) {
        u8 b = lsb(capture1);
        flipBit(capture1, b);
        if ((1ULL << b) & epSquare) {
            pushMove(&MOVES, b + (7 * sign), b, 0, 0, 1, 0, 0, &count);
        } else if ((1ULL << b) & pawnPromotionMask) {
            pushMove(&MOVES, b + (7 * sign), b, 0, 0, 0, 0, 4, &count);
            pushMove(&MOVES, b + (7 * sign), b, 0, 0, 0, 0, 3, &count);
            pushMove(&MOVES, b + (7 * sign), b, 0, 0, 0, 0, 2, &count);
            pushMove(&MOVES, b + (7 * sign), b, 0, 0, 0, 0, 1, &count);
        } else {
            pushMove(&MOVES, b + (7 * sign), b, 0, 0, 0, 0, 0, &count);
        }
    }

    while (capture2) {
        u8 b = lsb(capture2);
        flipBit(capture2, b);
        if ((1ULL << b) & epSquare) {
            pushMove(&MOVES, b + (9 * sign), b, 0, 0, 1, 0, 0, &count);
        } else if ((1ULL << b) & pawnPromotionMask) {
            pushMove(&MOVES, b + (9 * sign), b, 0, 0, 0, 0, 4, &count);
            pushMove(&MOVES, b + (9 * sign), b, 0, 0, 0, 0, 3, &count);
            pushMove(&MOVES, b + (9 * sign), b, 0, 0, 0, 0, 2, &count);
            pushMove(&MOVES, b + (9 * sign), b, 0, 0, 0, 0, 1, &count);
        } else {
            pushMove(&MOVES, b + (9 * sign), b, 0, 0, 0, 0, 0, &count);
        }
    }

    return count;
}

int possiblecaptures(
    bool isWhiteToMove,
    u64 occupied, u64 epSquare, u64 myPieces, u64 enemyPieces, 
    u64 pawns, u64 knights, u64 bishops, u64 rooks, u64 queens, u64 king, 
    bool castleQueen, bool castleKing, struct Move *CAPTURES
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
        pushMove(&CAPTURES, kingIndex, b, 5, 0, 0, 0, 0, &count);
    }

    u64 knightCaptures;

    while (knights) {
        u8 knightIndex = lsb(knights);
        flipBit(knights, knightIndex);
        knightCaptures = knightAttacks[knightIndex] & enemyPieces;
        while (knightCaptures) {
            u8 b = lsb(knightCaptures);
            flipBit(knightCaptures, b);
            pushMove(&CAPTURES, knightIndex, b, 1, 0, 0, 0, 0, &count);
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
            pushMove(&CAPTURES, bishopIndex, b, 2, 0, 0, 0, 0, &count);
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
            pushMove(&CAPTURES, rookIndex, b, 3, 0, 0, 0, 0, &count);
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
            pushMove(&CAPTURES, queenIndex, b, 4, 0, 0, 0, 0, &count);
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
            pushMove(&CAPTURES, b + (7 * sign), b, 0, 0, 1, 0, 0, &count);
        } else if ((1ULL << b) & pawnPromotionMask) {
            pushMove(&CAPTURES, b + (7 * sign), b, 0, 0, 0, 0, 4, &count);
            pushMove(&CAPTURES, b + (7 * sign), b, 0, 0, 0, 0, 3, &count);
            pushMove(&CAPTURES, b + (7 * sign), b, 0, 0, 0, 0, 2, &count);
            pushMove(&CAPTURES, b + (7 * sign), b, 0, 0, 0, 0, 1, &count);
        } else {
            pushMove(&CAPTURES, b + (7 * sign), b, 0, 0, 0, 0, 0, &count);
        }
    }

    while (capture2) {
        u8 b = lsb(capture2);
        flipBit(capture2, b);
        if ((1ULL << b) & epSquare) {
            pushMove(&CAPTURES, b + (9 * sign), b, 0, 0, 1, 0, 0, &count);
        } else if ((1ULL << b) & pawnPromotionMask) {
            pushMove(&CAPTURES, b + (9 * sign), b, 0, 0, 0, 0, 4, &count);
            pushMove(&CAPTURES, b + (9 * sign), b, 0, 0, 0, 0, 3, &count);
            pushMove(&CAPTURES, b + (9 * sign), b, 0, 0, 0, 0, 2, &count);
            pushMove(&CAPTURES, b + (9 * sign), b, 0, 0, 0, 0, 1, &count);
        } else {
            pushMove(&CAPTURES, b + (9 * sign), b, 0, 0, 0, 0, 0, &count);
        }
    }

    return count;
}

// MOVE MAKING AND HASH UPDATING

struct Bitboards doMove(struct Move move, struct Bitboards bitboards) {
    // todo: maybe using pointers to the bitboards and undo the move later is faster?

    moveCalls++;
    u64 toBit = 1ULL << move.to;
    u64 fromBit = 1ULL << move.from;

    bitboards.allPieces ^= ((fromBit) | ((toBit) & ~bitboards.allPieces));
    // update the hash
    bitboards.hash ^= whiteToMove;
    bitboards.pieceList[move.from] = 0;
    bitboards.pieceList[move.to] = (int) ((move.pieceType + 1) * (bitboards.color ? 1 : -1));

    // now for the specific pieces
    if (bitboards.color) {
        if (move.isEnPassantCapture) {
            // remove the captured pawn
            bitboards.blackPawns ^= (1ULL << (move.to - 8));
            bitboards.blackPieces ^= (1ULL << (move.to - 8));
            bitboards.allPieces ^= (1ULL << (move.to - 8));
            // update the pawn hash
            bitboards.hash ^= ZOBRIST_TABLE[(move.to - 8)][6];
            bitboards.pieceList[move.to - 8] = 0;

        } else if (bitboards.blackPieces & (toBit)) {
            // if the piece captures a piece
            // here we dont need to remove the bit from the occupied squares, because the capturing piece is already there
            bitboards.blackPieces ^= (toBit);

            if (bitboards.blackPawns & (toBit)) {
                bitboards.blackPawns ^= (toBit);
                bitboards.hash ^= ZOBRIST_TABLE[move.to][6];
    
            } else if (bitboards.blackBishops & (toBit)) {
                bitboards.blackBishops ^= (toBit);
                bitboards.hash ^= ZOBRIST_TABLE[move.to][9];
    
            } else if (bitboards.blackKnights & (toBit)) {
                bitboards.blackKnights ^= (toBit);
                bitboards.hash ^= ZOBRIST_TABLE[move.to][8];
    
            } else if (bitboards.blackRooks & (toBit)) {
                bitboards.blackRooks ^= (toBit);
                bitboards.hash ^= ZOBRIST_TABLE[move.to][7];
    
            } else if (bitboards.blackQueens & (toBit)) {
                bitboards.blackQueens ^= (toBit);
                bitboards.hash ^= ZOBRIST_TABLE[move.to][10];
    
            } else if (bitboards.blackKing & (toBit)) {
                bitboards.blackKing ^= (toBit);
                // in theory this should never happen because the search will stop if one player has no moves
                bitboards.hash ^= ZOBRIST_TABLE[move.to][11];
            }
            // remove the captured bit from all the boards its part of
        }
        
        bitboards.whitePieces ^= ((fromBit) | (toBit));

        if (move.pieceType == 0) { // pawn move
            bitboards.whitePawns ^= (fromBit);
            bitboards.hash ^= ZOBRIST_TABLE[move.from][0];
            if (move.to >= 56) {
                // pawn promotion
                if (move.promotesTo == 4) {
                    bitboards.whiteQueens ^= (toBit);
                    bitboards.hash ^= ZOBRIST_TABLE[move.to][4];
                    bitboards.pieceList[move.to] = 5;
                } else if (move.promotesTo == 3) {
                    bitboards.whiteRooks ^= (toBit);
                    bitboards.hash ^= ZOBRIST_TABLE[move.to][1];
                    bitboards.pieceList[move.to] = 4;
                } else if (move.promotesTo == 2) {
                    bitboards.whiteBishops ^= (toBit);
                    bitboards.hash ^= ZOBRIST_TABLE[move.to][3];
                    bitboards.pieceList[move.to] = 3;
                } else if (move.promotesTo == 1) {
                    bitboards.whiteKnights ^= (toBit);
                    bitboards.hash ^= ZOBRIST_TABLE[move.to][2];
                    bitboards.pieceList[move.to] = 2;
                }
            } else {
                bitboards.whitePawns ^= (toBit);
                bitboards.hash ^= ZOBRIST_TABLE[move.to][0];
            }
        }
        else if (move.pieceType == 1) {// knight move
            bitboards.whiteKnights ^= ((fromBit) | (toBit));
            bitboards.hash ^= ZOBRIST_TABLE[move.from][2] ^ ZOBRIST_TABLE[move.to][2];

        } else if (move.pieceType == 2) { // bishop move
            bitboards.whiteBishops ^= ((fromBit) | (toBit));
            bitboards.hash ^= ZOBRIST_TABLE[move.from][3] ^ ZOBRIST_TABLE[move.to][3];

        } else if (move.pieceType == 3) { // rook move
            bitboards.whiteRooks ^= ((fromBit) | (toBit));
            bitboards.hash ^= ZOBRIST_TABLE[move.from][1] ^ ZOBRIST_TABLE[move.to][1];
            if (bitboards.whiteCastleQueenSide && move.from == 7) {
                // if the rook moves from the a1 square, then the white queen side castle is no longer possible
                bitboards.whiteCastleQueenSide = false;
                bitboards.hash ^= castlingRights[3];
            } else if (bitboards.whiteCastleKingSide && move.from == 0) {
                bitboards.whiteCastleKingSide = false;
                bitboards.hash ^= castlingRights[2];
            }
        } else if (move.pieceType == 4) { // queen move
            bitboards.whiteQueens ^= ((fromBit) | (toBit));
            bitboards.hash ^= ZOBRIST_TABLE[move.from][4] ^ ZOBRIST_TABLE[move.to][4];

        } else if (move.pieceType == 5) { // king move
            bitboards.whiteKing ^= ((fromBit) | (toBit));
            bitboards.hash ^= ZOBRIST_TABLE[move.from][5] ^ ZOBRIST_TABLE[move.to][5];
            if (move.castle) {
                int s;
                int o;
                if (move.castle == KINGSIDE) {s = 0; o = 2;} else {s = 7; o = 4;}
                    // put the rook on its new sqaure
                    bitboards.allPieces ^= (1ULL << s) | (1ULL << o);
                    bitboards.whitePieces ^= (1ULL << s) | (1ULL << o);
                    bitboards.whiteRooks ^= (1ULL << s) | (1ULL << o);

                    bitboards.hash ^= ZOBRIST_TABLE[o][1] ^ ZOBRIST_TABLE[s][1];
                    bitboards.pieceList[s] = 0;
                    bitboards.pieceList[o] = 4;
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
            bitboards.enPassantSquare = 0;
        }

        if (move.createsEnPassant && ((bitboards.blackPawns & (1ULL << (toBit + 1))) || (bitboards.blackPawns & (1ULL << (toBit - 1))))) {
            // if the move creates an en passant square and it it captureable, add it to the hash and set the bit
            bitboards.enPassantSquare ^= 1ULL << (move.to - 8);
            bitboards.hash ^= ZOBRIST_TABLE[move.to - 8][12];
        }

    } else {
        // Black move
        if (move.isEnPassantCapture) {
            bitboards.whitePawns ^= (1ULL << (move.to + 8));
            bitboards.whitePieces ^= (1ULL << (move.to + 8));
            bitboards.allPieces ^= (1ULL << (move.to + 8));
            bitboards.hash ^= ZOBRIST_TABLE[move.to + 8][0];
            bitboards.pieceList[move.to + 8] = 0;

        } else if (bitboards.whitePieces & (toBit)) {
            bitboards.whitePieces ^= (toBit);

            if (bitboards.whitePawns & (toBit)) {
                bitboards.whitePawns ^= (toBit);
                bitboards.hash ^= ZOBRIST_TABLE[move.to][0];
    
            } else if (bitboards.whiteBishops & (toBit)) {
                bitboards.whiteBishops ^= (toBit);
                bitboards.hash ^= ZOBRIST_TABLE[move.to][3];
    
            } else if (bitboards.whiteKnights & (toBit)) {
                bitboards.whiteKnights ^= (toBit);
                bitboards.hash ^= ZOBRIST_TABLE[move.to][2];
    
            } else if (bitboards.whiteRooks & (toBit)) {
                bitboards.whiteRooks ^= (toBit);
                bitboards.hash ^= ZOBRIST_TABLE[move.to][1];
    
            } else if (bitboards.whiteQueens & (toBit)) {
                bitboards.whiteQueens ^= (toBit);
                bitboards.hash ^= ZOBRIST_TABLE[move.to][4];
    
            } else if (bitboards.whiteKing & (toBit)) {
                bitboards.whiteKing ^= (toBit);
                bitboards.hash ^= ZOBRIST_TABLE[move.to][5];
            }
        }
        
        bitboards.blackPieces ^= (fromBit) | (toBit);

        if (move.pieceType == 0) {
            bitboards.blackPawns ^= (fromBit);
            bitboards.hash ^= ZOBRIST_TABLE[move.from][6];
            if (move.to <= 7) {
                if (move.promotesTo == 4) {
                    bitboards.blackQueens ^= (toBit);
                    bitboards.hash ^= ZOBRIST_TABLE[move.to][10];
                    bitboards.pieceList[move.to] = -5;
                } else if (move.promotesTo == 3) {
                    bitboards.blackRooks ^= (toBit);
                    bitboards.hash ^= ZOBRIST_TABLE[move.to][7];
                    bitboards.pieceList[move.to] = -4;
                } else if (move.promotesTo == 2) {
                    bitboards.blackBishops ^= (toBit);
                    bitboards.hash ^= ZOBRIST_TABLE[move.to][9];
                    bitboards.pieceList[move.to] = -3;
                } else if (move.promotesTo == 1) {
                    bitboards.blackKnights ^= (toBit);
                    bitboards.hash ^= ZOBRIST_TABLE[move.to][8];
                    bitboards.pieceList[move.to] = -2;
                }
            } else {
                bitboards.blackPawns ^= (toBit);
                bitboards.hash ^= ZOBRIST_TABLE[move.to][6];
            }
        }
        else if (move.pieceType == 1) {
            bitboards.blackKnights ^= ((toBit) | (fromBit));
            bitboards.hash ^= ZOBRIST_TABLE[move.to][8] ^ ZOBRIST_TABLE[move.from][8];

        } else if (move.pieceType == 2) {
            bitboards.blackBishops ^= ((toBit) | (fromBit));
            bitboards.hash ^= ZOBRIST_TABLE[move.to][9] ^ ZOBRIST_TABLE[move.from][9];

        } else if (move.pieceType == 3) {
            bitboards.blackRooks ^= ((toBit) | (fromBit));
            bitboards.hash ^= ZOBRIST_TABLE[move.to][7] ^ ZOBRIST_TABLE[move.from][7];
            if (bitboards.blackCastleQueenSide && move.from == 31) {
                bitboards.blackCastleQueenSide = false;
                bitboards.hash ^= castlingRights[1];
            } else if (bitboards.blackCastleKingSide && move.from == 24) {
                bitboards.blackCastleKingSide = false;
                bitboards.hash ^= castlingRights[0];
            }
        } else if (move.pieceType == 4) {
            bitboards.blackQueens ^= ((toBit) | (fromBit));
            bitboards.hash ^= ZOBRIST_TABLE[move.to][10] ^ ZOBRIST_TABLE[move.from][10];

        } else if (move.pieceType == 5) {
            bitboards.blackKing ^= ((toBit) | (fromBit));
            bitboards.hash ^= ZOBRIST_TABLE[move.to][11] ^ ZOBRIST_TABLE[move.from][11];
            if (move.castle) {
                int s;
                int o;
                if (move.castle == KINGSIDE) {s = 56; o = 58;} else {s = 63; o = 60;}

                    bitboards.allPieces ^= (1ULL << s) | (1ULL << o);
                    bitboards.blackPieces ^= (1ULL << s) | (1ULL << o);
                    bitboards.blackRooks ^= (1ULL << s) | (1ULL << o);
                    bitboards.hash ^= ZOBRIST_TABLE[o][7] ^ ZOBRIST_TABLE[s][7];
                    bitboards.pieceList[s] = 0;
                    bitboards.pieceList[o] = -4;
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
            bitboards.enPassantSquare = 0;
        }

        if (move.createsEnPassant && ((bitboards.whitePawns & (1ULL << (move.to + 1))) || (bitboards.whitePawns & (1ULL << (move.to - 1))))) {
            bitboards.enPassantSquare ^= (1ULL << (move.to + 8));
            bitboards.hash ^= ZOBRIST_TABLE[move.to + 8][12];
        }
    }
    bitboards.color = !bitboards.color;

    return bitboards;
}

struct Bitboards doNullMove(struct Bitboards bitboards) {
    bitboards.hash ^= whiteToMove;
    bitboards.color = !bitboards.color;
    if (bitboards.enPassantSquare) {
            bitboards.hash ^= ZOBRIST_TABLE[lsb(bitboards.enPassantSquare)][12];
        }
    bitboards.enPassantSquare = 0;
    return bitboards;
}

// CHECK DETECTION

bool canCaptureOpponentsKing(struct Bitboards * bitboards) {
    int kingIndex = 0;
    if (bitboards->color) {
        if (bitboards->blackKing) {
            kingIndex = lsb(bitboards->blackKing);
        } else {
            return true;
        }
        if ((kingAttacks[kingIndex] & bitboards->whiteKing)) {
            return true;
        } else if ((knightAttacks[kingIndex] & bitboards->whiteKnights)) {
            return true;
        } else if ((generateBishopMoves(kingIndex, bitboards->allPieces) & (bitboards->whiteBishops | bitboards->whiteQueens))) {
            return true;
        } else if ((generateRookMoves(kingIndex, bitboards->allPieces) & (bitboards->whiteRooks | bitboards->whiteQueens))) {
            return true;
        } else if ((((bitboards->blackKing >> 7) & ~rightmostFileMask) | ((bitboards->blackKing >> 9) & ~leftmostFileMask)) & bitboards->whitePawns) {
            return true;
        }
    } else {
        if (bitboards->whiteKing) {
            kingIndex = lsb(bitboards->whiteKing);
        } else {
            return true;
        }
        if ((kingAttacks[kingIndex] & bitboards->blackKing)) {
            return true;
        } else if ((knightAttacks[kingIndex] & bitboards->blackKnights)) {
            return true;
        } else if ((generateBishopMoves(kingIndex, bitboards->allPieces) & (bitboards->blackBishops | bitboards->blackQueens))) {
            return true;
        } else if ((generateRookMoves(kingIndex, bitboards->allPieces) & (bitboards->blackRooks | bitboards->blackQueens))) {
            return true;
        } else if ((((bitboards->whiteKing << 7) & ~leftmostFileMask) | ((bitboards->whiteKing << 9) & ~rightmostFileMask)) & bitboards->blackPawns) {
            return true;
        }
    }
    return false;
}

bool isInCheck(struct Bitboards * bitboards) {
    int kingIndex = 0;
    if (!bitboards->color) {
        if (bitboards->blackKing) {
            kingIndex = lsb(bitboards->blackKing);
        } else {
            return true;
        }
        if ((kingAttacks[kingIndex] & bitboards->whiteKing)) {
            return true;
        } else if ((knightAttacks[kingIndex] & bitboards->whiteKnights)) {
            return true;
        } else if ((generateBishopMoves(kingIndex, bitboards->allPieces) & (bitboards->whiteBishops | bitboards->whiteQueens))) {
            return true;
        } else if ((generateRookMoves(kingIndex, bitboards->allPieces) & (bitboards->whiteRooks | bitboards->whiteQueens))) {
            return true;
        } else if ((((bitboards->blackKing >> 7) & ~rightmostFileMask) | ((bitboards->blackKing >> 9) & ~leftmostFileMask)) & bitboards->whitePawns) {
            return true;
        }
    } else {
        if (bitboards->whiteKing) {
            kingIndex = lsb(bitboards->whiteKing);
        } else {
            return true;
        }
        if ((kingAttacks[kingIndex] & bitboards->blackKing)) {
            return true;
        } else if ((knightAttacks[kingIndex] & bitboards->blackKnights)) {
            return true;
        } else if ((generateBishopMoves(kingIndex, bitboards->allPieces) & (bitboards->blackBishops | bitboards->blackQueens))) {
            return true;
        } else if ((generateRookMoves(kingIndex, bitboards->allPieces) & (bitboards->blackRooks | bitboards->blackQueens))) {
            return true;
        } else if ((((bitboards->whiteKing << 7) & ~leftmostFileMask) | ((bitboards->whiteKing << 9) & ~rightmostFileMask)) & bitboards->blackPawns) {
            return true;
        }
    }
    return false;
}

bool isIllegalCastle(struct Move move, struct Bitboards boards) {
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

    struct Bitboards newBoard = doMove(betweenMove, boards); // cant castle if in check while on the between square

    return canCaptureOpponentsKing(&newBoard);
}

bool hasLegalMoves(struct Move *possible, struct Bitboards boards, int numMoves) {
    // checks if there are any legal moves for the current player
    // used to detect checkmate after the search
    int removedAmount = 0;
    for (int i = 0; i < numMoves; i++) {
        bool check = false;
        if (possible[i].castle) {
            check = (isIllegalCastle(possible[i], boards) || isInCheck(&boards));
        } else {
            struct Bitboards newBoard = doMove(possible[i], boards);
            check = canCaptureOpponentsKing(&newBoard);
        }
        if (check) {
            removedAmount++;
        }
    }
    return numMoves - removedAmount > 0;
}

// EVALUATION

int evaluate(u64 whitePawns, u64 whiteKnights, u64 whiteBishops, u64 whiteRooks, u64 whiteQueens, u64 whiteKing, u64 blackPawns, u64 blackKnights, u64 blackBishops, u64 blackRooks, u64 blackQueens, u64 blackKing) {
    // todo: take king safety into account
    // this makes for about 1/6 of the total eval calls
    evalCalls++;
    int evalWhite = 0;
    int evalBlack = 0;
    while (blackKnights) {
        u8 b = lsb(blackKnights);
        flipBit(blackKnights, b);
        evalBlack += knightEvalBlack[b];
        }
    while (blackBishops) {
        u8 b = lsb(blackBishops);
        flipBit(blackBishops, b);
        evalBlack += bishopEvalBlack[b];
        }
    while (blackRooks) {
        u8 b = lsb(blackRooks);
        flipBit(blackRooks, b);
        evalBlack += rookEvalBlack[b];
        }
    while (blackQueens) {
        u8 b = lsb(blackQueens);
        flipBit(blackQueens, b);
        evalBlack += queenEvalBlack[b];
        }
    while (blackPawns) {
        u8 b = lsb(blackPawns);
        flipBit(blackPawns, b);
        evalBlack += pawnEvalBlack[b];
        }

    // white position evaluation
    while (whiteKnights) {
        u8 b = lsb(whiteKnights);
        flipBit(whiteKnights, b);
        evalWhite += knightEvalWhite[b];
        }
    while (whiteBishops) {
        u8 b = lsb(whiteBishops);
        flipBit(whiteBishops, b);
        evalWhite += bishopEvalWhite[b];
        }
    while (whiteRooks) {
        u8 b = lsb(whiteRooks);
        flipBit(whiteRooks, b);
        evalWhite += rookEvalWhite[b];
        }
    while (whiteQueens) {
        u8 b = lsb(whiteQueens);
        flipBit(whiteQueens, b);
        evalWhite += queenEvalWhite[b];
        }
    while (whitePawns) {
        u8 b = lsb(whitePawns);
        flipBit(whitePawns, b);
        evalWhite += pawnEvalWhite[b];
        }
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

void quickSortArrayDec(struct Move structs[], int values[], int left, int right) {
    // sorts the moves decreasingly in orderMoves of their evaluation
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
        quickSortArrayDec(structs, values, left, j);
        quickSortArrayDec(structs, values, i, right);
    }
}

void quickSortArrayInc(struct Move structs[], int values[], int left, int right) {
    // Sorts the moves increasingly in orderMoves of their evaluation
    if (left < right) {
        int pivotValue = values[left + (right - left) / 2];

        int i = left, j = right;
        while (i <= j) {
            while (values[i] < pivotValue) i++;
            while (values[j] > pivotValue) j--;
            if (i <= j) {
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

        quickSortArrayInc(structs, values, left, j);
        quickSortArrayInc(structs, values, i, right);
    }
}

static int pieceValues[6] = {100, 300, 300, 500, 900, 0};
int *whiteEvalTables[6] = {pawnEvalWhite, knightEvalWhite, bishopEvalWhite, rookEvalWhite, queenEvalWhite, kingEvalWhite};
int *blackEvalTables[6] = {pawnEvalBlack, knightEvalBlack, bishopEvalBlack, rookEvalBlack, queenEvalBlack, kingEvalBlack};

void orderMoves(struct Move * moves, Bitboards BITBOARDS, int numMoves) {
    Move *tableMove = tableGetMove(transTable, BITBOARDS.hash);
    int values[numMoves];
    for (int i = 0; i < numMoves; i++) {
        values[i] = 0;
        int pieceMaterial = 0;
        if (tableMove != NULL && moves[i].from == tableMove->from && moves[i].to == tableMove->to) {
            values[i] = BITBOARDS.color ? INF : -INF;
        }
        else
        if (BITBOARDS.color) {
            values[i] += whiteEvalTables[moves[i].pieceType][moves[i].to] - whiteEvalTables[moves[i].pieceType][moves[i].from];
            pieceMaterial = pieceValues[moves[i].pieceType];
            if (BITBOARDS.pieceList[moves[i].to]) {
                values[i] -= blackEvalTables[BITBOARDS.pieceList[moves[i].to] * -1 - 1][moves[i].to];
                values[i] -= pieceMaterial;
            }

        } else {
            values[i] += blackEvalTables[moves[i].pieceType][moves[i].to] - blackEvalTables[moves[i].pieceType][moves[i].from];
            pieceMaterial = pieceValues[moves[i].pieceType];
            if (BITBOARDS.pieceList[moves[i].to]) {
                values[i] -= whiteEvalTables[BITBOARDS.pieceList[moves[i].to] - 1][moves[i].to];
                values[i] += pieceMaterial;
            }
        }
    }
    if (BITBOARDS.color) {
        quickSortArrayDec(moves, values, 0, numMoves-1);
    } else {
        quickSortArrayInc(moves, values, 0, numMoves-1);
    }
}

void orderCaptures(struct Move *moves, Bitboards BITBOARDS, int numMoves) {
    Move *tableMove = tableGetMove(quietTable, BITBOARDS.hash);
    int values[numMoves];
    for (int i = 0; i < numMoves; i++) {
        values[i] = 0;
        if (tableMove != NULL && moves[i].from == tableMove->from && moves[i].to == tableMove->to) {
            values[i] = BITBOARDS.color ? INF : -INF;
        }
        else
        if (BITBOARDS.color) {
            values[i] += whiteEvalTables[moves[i].pieceType][moves[i].to] - whiteEvalTables[moves[i].pieceType][moves[i].from];
            values[i] -= blackEvalTables[(BITBOARDS.pieceList[moves[i].to] * -1) - 1][moves[i].to];
            values[i] -= pieceValues[moves[i].pieceType];

        } else {
            values[i] += blackEvalTables[moves[i].pieceType][moves[i].to] - blackEvalTables[moves[i].pieceType][moves[i].from];
            values[i] -= whiteEvalTables[BITBOARDS.pieceList[moves[i].to] - 1][moves[i].to];
            values[i] += pieceValues[moves[i].pieceType];
        }
    }
    if (BITBOARDS.color) {
        quickSortArrayDec(moves, values, 0, numMoves-1);
    } else {
        quickSortArrayInc(moves, values, 0, numMoves-1);
    }
}

int quiescenceSearch(struct Bitboards BITBOARDS, int alpha, int beta, int depth) {
    // searches the board for captures only using recursion

    if (canCaptureOpponentsKing(&BITBOARDS)) {
        return INF;
    }

    quietVisits++;
    quietNodes++;

    int value;
    if (tableGetEntry(quietTable, BITBOARDS.hash, depth, &value, alpha, beta)) {
        quietTranspositions++;
        return value;
    }

    // start with the normal evaluation since nobody can be forced to capture
    value = evaluate(BITBOARDS.whitePawns, BITBOARDS.whiteKnights, BITBOARDS.whiteBishops, BITBOARDS.whiteRooks, BITBOARDS.whiteQueens, BITBOARDS.whiteKing, BITBOARDS.blackPawns, BITBOARDS.blackKnights, BITBOARDS.blackBishops, BITBOARDS.blackRooks, BITBOARDS.blackQueens, BITBOARDS.blackKing) * (BITBOARDS.color ? 1 : -1);

    if (value >= beta) {
        tableSetEntry(quietTable, BITBOARDS.hash, depth, value, LOWERBOUND);
        return beta;
    }
    if (value > alpha) {
        alpha = value;
    }

    struct Move moves[MAX_NUM_MOVES];
    int moveCount = 0;
    if (BITBOARDS.color) {
        moveCount = possiblecaptures(
            BITBOARDS.color, 
            BITBOARDS.allPieces, BITBOARDS.enPassantSquare, BITBOARDS.whitePieces, BITBOARDS.blackPieces, 
            BITBOARDS.whitePawns, BITBOARDS.whiteKnights, BITBOARDS.whiteBishops, BITBOARDS.whiteRooks, BITBOARDS.whiteQueens, BITBOARDS.whiteKing, 
            BITBOARDS.whiteCastleQueenSide, BITBOARDS.whiteCastleKingSide, &moves[0]
            );
    } else {
        moveCount = possiblecaptures(
            BITBOARDS.color, 
            BITBOARDS.allPieces, BITBOARDS.enPassantSquare, BITBOARDS.blackPieces, BITBOARDS.whitePieces, 
            BITBOARDS.blackPawns, BITBOARDS.blackKnights, BITBOARDS.blackBishops, BITBOARDS.blackRooks, BITBOARDS.blackQueens, BITBOARDS.blackKing, 
            BITBOARDS.blackCastleQueenSide, BITBOARDS.blackCastleKingSide, &moves[0]
            );
    }

    orderCaptures(&moves[0], BITBOARDS, moveCount);
    int flag = UPPERBOUND;
    
    for (int i = 0; i < moveCount; i++) {
        // do each of the moves

        struct Bitboards newBoard = doMove(moves[i], BITBOARDS);

        int value = -quiescenceSearch(newBoard, -beta, -alpha, depth - 1);
        
        // update alpha and try to prune
        if (value >= beta) {
            tableSetEntry(quietTable, BITBOARDS.hash, depth, value, LOWERBOUND);
            tableSetMove(quietTable, BITBOARDS.hash, depth, &moves[i]);
            return beta;
        }
        if (value > alpha) {
            alpha = value;
            flag = EXACT;
            tableSetMove(quietTable, BITBOARDS.hash, depth, &moves[i]);
        }
    }

    tableSetEntry(quietTable, BITBOARDS.hash, depth, alpha, flag);

    return alpha;
}

int tree(struct Bitboards BITBOARDS, int ply, int alpha, int beta, int depth) {

    if (canCaptureOpponentsKing(&BITBOARDS)) {
        return INF;
    }

    nodes++;
    visits++;

    int value;
    if (tableGetEntry(transTable, BITBOARDS.hash, ply, &value, alpha, beta)) {
        transpositions++;
        return value;
    }

    if (ply <= 0) {
        quiescenceCalls++;
        return quiescenceSearch(BITBOARDS, alpha, beta, 0);
        // return evaluate(BITBOARDS.whitePawns, BITBOARDS.whiteKnights, BITBOARDS.whiteBishops, BITBOARDS.whiteRooks, BITBOARDS.whiteQueens, BITBOARDS.whiteKing, BITBOARDS.blackPawns, BITBOARDS.blackKnights, BITBOARDS.blackBishops, BITBOARDS.blackRooks, BITBOARDS.blackQueens, BITBOARDS.blackKing) * (BITBOARDS.color ? 1 : -1);
    }

    struct Move moves[MAX_NUM_MOVES];
    int moveCount = 0;

    if (BITBOARDS.color) { // white
        moveCount = possiblemoves(
            BITBOARDS.color, 
            BITBOARDS.allPieces, BITBOARDS.enPassantSquare, BITBOARDS.whitePieces, BITBOARDS.blackPieces, 
            BITBOARDS.whitePawns, BITBOARDS.whiteKnights, BITBOARDS.whiteBishops, BITBOARDS.whiteRooks, BITBOARDS.whiteQueens, BITBOARDS.whiteKing, 
            BITBOARDS.whiteCastleQueenSide, BITBOARDS.whiteCastleKingSide, &moves[0]
            );
    } else { // black
        moveCount = possiblemoves(
            BITBOARDS.color, 
            BITBOARDS.allPieces, BITBOARDS.enPassantSquare, BITBOARDS.blackPieces, BITBOARDS.whitePieces, 
            BITBOARDS.blackPawns, BITBOARDS.blackKnights, BITBOARDS.blackBishops, BITBOARDS.blackRooks, BITBOARDS.blackQueens, BITBOARDS.blackKing, 
            BITBOARDS.blackCastleQueenSide, BITBOARDS.blackCastleKingSide, &moves[0]
            );
    }

    orderMoves(&moves[0], BITBOARDS, moveCount);

    bool hasLegalMoves = false;
    int flag = UPPERBOUND;
    for (int i = 0; i < moveCount; i++) {

        if (moves[i].castle && (isIllegalCastle(moves[i], BITBOARDS) || isInCheck(&BITBOARDS))) {
            continue;
        }

        struct Bitboards newBoard = doMove(moves[i], BITBOARDS);

        int value = -tree(newBoard, ply - 1, -beta, -alpha, depth);

        if (value > -INF) {
            hasLegalMoves = true;
        }

        if (value >= beta) {
            tableSetEntry(transTable, BITBOARDS.hash, ply, value, LOWERBOUND);
            tableSetMove(transTable, BITBOARDS.hash, ply, &moves[i]);
            return beta;
        }

        if (value > alpha) {
            alpha = value;
            flag = EXACT;
            tableSetMove(transTable, BITBOARDS.hash, ply, &moves[i]);
        }
    }

    if (!hasLegalMoves) {
        if (isInCheck(&BITBOARDS)) {
            return -INF + ply;
        } else {
            return 0;
        }
    }

    tableSetEntry(transTable, BITBOARDS.hash, ply, alpha, flag);

    return alpha;
}

struct Move bestMove(struct Move *possible, struct Bitboards bitboards, int numMoves) {
    // find the best move using iterative deepening
    
    nodes = 0;
    quietNodes = 0;
    transpositions = 0;
    quietTranspositions = 0;

    struct Move best;
    int values[numMoves];

    // ordering moves here is not necessary because at depth 0 the moves will be ordered by the orderMoves function
    clock_t start_time = clock();
    
    bool stopSearch = false;
    int d = 0;
    int s = bitboards.color ? -1 : 1;

    printf("move amount: %d\n", numMoves);

    for (int ply = 0; ply < maxDepth; ply++) {
        printf("Depth: %d - \n", ply+1);
        for (int i = 0; i < numMoves; i++) {
            int moveEval;
            if (stopSearch) {
                moveEval = 0;
            } else {
                struct Bitboards newBoard = doMove(possible[i], bitboards); // engine move
                visits = 0;
                quietVisits = 0;
                printf("nodes after move from: %d to: %d pieceType: %d castle: %d creates EP: %d is EP capture: %d promotion: %d - ", possible[i].from, possible[i].to, possible[i].pieceType, possible[i].castle, possible[i].createsEnPassant, possible[i].isEnPassantCapture, possible[i].promotesTo);
                if (possible[i].castle && (isIllegalCastle(possible[i], bitboards) || isInCheck(&bitboards))) {
                    printf("0\n");
                    continue;
                }
                moveEval = s * tree(newBoard, ply, -INF, INF, maxDepth); // start search
                
                printf("%d (%d quiet) eval: %d\n", visits, quietVisits, moveEval);
            }
            values[i] = moveEval;
            if (((double)(clock() - start_time)) / (CLOCKS_PER_SEC / 1000) > maxTime*1000) {
                // if time is up, stop searching
                break;
                }
            else if ((bitboards.color && (moveEval >= 99000)) || (!bitboards.color && (moveEval <= -99000))) {
                printf("checkmate found\n");
                return possible[i];
                stopSearch = true;
            }
        }
        if (((double)(clock() - start_time)) / (CLOCKS_PER_SEC / 1000) > maxTime*1000) {
            printf("time out\n");
            break;
            }

        if (bitboards.color) {
            quickSortArrayDec(&possible[0], values, 0, numMoves-1);
        } else {
            quickSortArrayInc(&possible[0], values, 0, numMoves-1);
        }

        printf(">>> best move: from: %d  to: %d pieceType: %d value: %d\n", possible[0].from, possible[0].to, possible[0].pieceType, values[0]);
        tableSetMove(transTable, bitboards.hash, ply, &possible[0]);
        d = ply;
    }

    printf("\ntranspositions found: %d (%d quiet)\n", transpositions, quietTranspositions);

    best = possible[0];
    
    if (values[0] == -100001 || values[0] == 100001) {
        // this happens on a position where the engine is already checkmated
        struct Move move = {from: 255, to: 255, pieceType: 255, castle: 255, isEnPassantCapture: 255, createsEnPassant: 255, promotesTo: 255};
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
            fread(&bookPages[i].entries[j].hash, sizeof(u64), 1, origin);
            fread(&bookPages[i].entries[j].numMoves, sizeof(u8), 1, origin);
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
    bookEntries = readBook(bookEntries, "book30.dat");
    printf("book read\n");
}

struct Move getBookMove(u64 hash) {
    if (useBook) {

        struct Book page = bookEntries[hash % BOOK_SIZE];

        for (int i = 0; i < page.numEntries; i++) {
            if (page.entries[i].hash == hash) {
                printf("num moves: %d\n", page.entries[i].numMoves);
                quickSortArrayDec(page.entries[i].moves, page.entries[i].occourences, 0, page.entries[i].numMoves - 1);
                for (int j = 0; j < page.entries[i].numMoves; j++) {
                    printf("from: %d to: %d amount: %d\n", page.entries[i].moves[j].from, page.entries[i].moves[j].to, page.entries[i].occourences[j]);
                }
                // randomize the move selection
                int index;
                if (page.entries[i].numMoves > 3) {
                    // moves are weighted by occourences
                    if (rand() % 100 > 25) {
                        index = 0;
                    } else {
                        if (rand() % 100 > 25) {
                            index = 1;
                        } else {
                            index = 2;
                        }
                    }
                    printf("random index: %d\n", index);
                } else {
                    index = 0;
                }

                struct Move entryMove = {
                    from: page.entries[i].moves[index].from, 
                    to: page.entries[i].moves[index].to, 
                    pieceType: page.entries[i].moves[index].pieceType, 
                    promotesTo: page.entries[i].moves[index].promotesTo, 
                    castle: page.entries[i].moves[index].castle, 
                    createsEnPassant: page.entries[i].moves[index].createsEnPassant, 
                    isEnPassantCapture: page.entries[i].moves[index].isEnPassantCapture
                    };
                return entryMove;
            }
        }

        printf("out of book\n");
    }

    struct Move nullmove;
    nullmove.from = 255;
    nullmove.to = 255;
    nullmove.pieceType = 255;
    nullmove.promotesTo = 255;
    nullmove.castle = 255;
    nullmove.createsEnPassant = 255;
    nullmove.isEnPassantCapture = 255;
    
    return nullmove;
}

void engineMove() {
    // find and make the engine move
    clock_t start_time = clock();
    struct Move best;
    // probe the book
    struct Move bookMove = getBookMove(bitboards.hash);
    int numMoves = 0;
    if (bookMove.from != 255) {
        best = bookMove;
        printf("book move found!\n");
    } else {
        struct Move possible[MAX_NUM_MOVES];
        if (bitboards.color) {
            numMoves = possiblemoves(
                bitboards.color, 
                bitboards.allPieces, bitboards.enPassantSquare, bitboards.whitePieces, bitboards.blackPieces, 
                bitboards.whitePawns, bitboards.whiteKnights, bitboards.whiteBishops, bitboards.whiteRooks, bitboards.whiteQueens, bitboards.whiteKing, 
                bitboards.whiteCastleQueenSide, bitboards.whiteCastleKingSide, &possible[0]
                );
        } else {
            numMoves = possiblemoves(
                bitboards.color, 
                bitboards.allPieces, bitboards.enPassantSquare, bitboards.blackPieces, bitboards.whitePieces, 
                bitboards.blackPawns, bitboards.blackKnights, bitboards.blackBishops, bitboards.blackRooks, bitboards.blackQueens, bitboards.blackKing, 
                bitboards.blackCastleQueenSide, bitboards.blackCastleKingSide, &possible[0]
                );
        }

        best = bestMove(possible, bitboards, numMoves);
    }

    if (best.from != 255) { // if there is a legal move
        updateFenClocks(best);
        bitboards = doMove(best, bitboards);
        printf("final move: %s%s\n", notation[best.from], notation[best.to]);
        int s = bitboards.color ? 1 : -1;
        printf("quick evaluation: %d\n", s * quiescenceSearch(bitboards, -INF, INF, 0));
        printf("board after move\n");
        printBoard(bitboards);

        lastfrom = best.from;
        lastto = best.to;

        printf("nodes searched: %d (%d quiet)\n", nodes, quietNodes);
    } else {
        bitboards.color = !bitboards.color;
        if (canCaptureOpponentsKing(&bitboards)) {
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
    printf("nodes per second: %d (%d quiet)\n", (int)(nodes/elapsed_time*1000), (int)(quietNodes/elapsed_time*1000));
    printf("evaluations per second: %d\n", (int)(evalCalls/elapsed_time*1000));

    // checkmate detection
    struct Move othermoves[MAX_NUM_MOVES];
    int numOtherMoves = 0;
    if (bitboards.color) {
        numOtherMoves = possiblemoves(
            bitboards.color, 
            bitboards.allPieces, bitboards.enPassantSquare, bitboards.whitePieces, bitboards.blackPieces, 
            bitboards.whitePawns, bitboards.whiteKnights, bitboards.whiteBishops, bitboards.whiteRooks, bitboards.whiteQueens, bitboards.whiteKing, 
            bitboards.whiteCastleQueenSide, bitboards.whiteCastleKingSide, &othermoves[0]
            );
    } else {
        numOtherMoves = possiblemoves(
            bitboards.color, 
            bitboards.allPieces, bitboards.enPassantSquare, bitboards.blackPieces, bitboards.whitePieces, 
            bitboards.blackPawns, bitboards.blackKnights, bitboards.blackBishops, bitboards.blackRooks, bitboards.blackQueens, bitboards.blackKing, 
            bitboards.blackCastleQueenSide, bitboards.blackCastleKingSide, &othermoves[0]
            );
    }

    if (!hasLegalMoves(othermoves, bitboards, numOtherMoves)) {
        bitboards.color = !bitboards.color;
        if (canCaptureOpponentsKing(&bitboards)) {
            printf("player is mate\n");
        } else {
            printf("draw\n");
        }
        exit(0);
    }

    // log counters
    printf("eval: %d moves: %d possible: %d quiet: %d\n", evalCalls, moveCalls, possibleCalls, quiescenceCalls);
    evalCalls = 0;
    moveCalls = 0;
    possibleCalls = 0;
    quiescenceCalls = 0;
}

int perft(int depth, Bitboards bitboards, int originalDepth) {
    // perft function for debugging
    if (canCaptureOpponentsKing(&bitboards)) {
        return 0;
    }

    if (depth == 0) {
        return 1;
    }

    struct Move possible[MAX_NUM_MOVES];
    int numMoves = 0;
    if (bitboards.color) {
        numMoves = possiblemoves(
            bitboards.color, 
            bitboards.allPieces, bitboards.enPassantSquare, bitboards.whitePieces, bitboards.blackPieces, 
            bitboards.whitePawns, bitboards.whiteKnights, bitboards.whiteBishops, bitboards.whiteRooks, bitboards.whiteQueens, bitboards.whiteKing, 
            bitboards.whiteCastleQueenSide, bitboards.whiteCastleKingSide, &possible[0]
            );
    } else {
        numMoves = possiblemoves(
            bitboards.color, 
            bitboards.allPieces, bitboards.enPassantSquare, bitboards.blackPieces, bitboards.whitePieces, 
            bitboards.blackPawns, bitboards.blackKnights, bitboards.blackBishops, bitboards.blackRooks, bitboards.blackQueens, bitboards.blackKing, 
            bitboards.blackCastleQueenSide, bitboards.blackCastleKingSide, &possible[0]
            );
    }
    
    int nodes = 0;
    for (int i = 0; i < numMoves; i++) {
        if (possible[i].castle && (isIllegalCastle(possible[i], bitboards) || isInCheck(&bitboards))) {
            continue;
        }
        struct Bitboards newBoard = doMove(possible[i], bitboards);
        int nodesbefore = nodes;
        nodes += perft(depth-1, newBoard, originalDepth);
        if (depth == originalDepth && nodes != nodesbefore) {
            printf("move %d %d", possible[i].from, possible[i].to);
            printf(" %d\n", nodes-nodesbefore);
        }
        
    }

    return nodes;
}

char* fileList[79] = {
    "Lichess Elite Database fens/lichess_elite_2013-09.pgn",
    "Lichess Elite Database fens/lichess_elite_2013-11.pgn",
    "Lichess Elite Database fens/lichess_elite_2014-01.pgn",
    "Lichess Elite Database fens/lichess_elite_2014-02.pgn",
    "Lichess Elite Database fens/lichess_elite_2014-03.pgn",
    "Lichess Elite Database fens/lichess_elite_2014-04.pgn",
    "Lichess Elite Database fens/lichess_elite_2014-05.pgn",
    "Lichess Elite Database fens/lichess_elite_2014-06.pgn",
    "Lichess Elite Database fens/lichess_elite_2014-07.pgn",
    "Lichess Elite Database fens/lichess_elite_2014-08.pgn",
    "Lichess Elite Database fens/lichess_elite_2014-09.pgn",
    "Lichess Elite Database fens/lichess_elite_2014-10.pgn",
    "Lichess Elite Database fens/lichess_elite_2014-11.pgn",
    "Lichess Elite Database fens/lichess_elite_2014-12.pgn",
    "Lichess Elite Database fens/lichess_elite_2015-01.pgn",
    "Lichess Elite Database fens/lichess_elite_2015-02.pgn",
    "Lichess Elite Database fens/lichess_elite_2015-03.pgn",
    "Lichess Elite Database fens/lichess_elite_2015-04.pgn",
    "Lichess Elite Database fens/lichess_elite_2015-05.pgn",
    "Lichess Elite Database fens/lichess_elite_2015-06.pgn",
    "Lichess Elite Database fens/lichess_elite_2015-07.pgn",
    "Lichess Elite Database fens/lichess_elite_2015-08.pgn",
    "Lichess Elite Database fens/lichess_elite_2015-09.pgn",
    "Lichess Elite Database fens/lichess_elite_2015-10.pgn",
    "Lichess Elite Database fens/lichess_elite_2015-11.pgn",
    "Lichess Elite Database fens/lichess_elite_2015-12.pgn",
    "Lichess Elite Database fens/lichess_elite_2016-01.pgn",
    "Lichess Elite Database fens/lichess_elite_2016-02.pgn",
    "Lichess Elite Database fens/lichess_elite_2016-03.pgn",
    "Lichess Elite Database fens/lichess_elite_2016-04.pgn",
    "Lichess Elite Database fens/lichess_elite_2016-05.pgn",
    "Lichess Elite Database fens/lichess_elite_2016-06.pgn",
    "Lichess Elite Database fens/lichess_elite_2016-07.pgn",
    "Lichess Elite Database fens/lichess_elite_2016-08.pgn",
    "Lichess Elite Database fens/lichess_elite_2016-09.pgn",
    "Lichess Elite Database fens/lichess_elite_2016-10.pgn",
    "Lichess Elite Database fens/lichess_elite_2016-11.pgn",
    "Lichess Elite Database fens/lichess_elite_2016-12.pgn",
    "Lichess Elite Database fens/lichess_elite_2017-01.pgn",
    "Lichess Elite Database fens/lichess_elite_2017-02.pgn",
    "Lichess Elite Database fens/lichess_elite_2017-03.pgn",
    "Lichess Elite Database fens/lichess_elite_2017-04.pgn",
    "Lichess Elite Database fens/lichess_elite_2017-05.pgn",
    "Lichess Elite Database fens/lichess_elite_2017-06.pgn",
    "Lichess Elite Database fens/lichess_elite_2017-07.pgn",
    "Lichess Elite Database fens/lichess_elite_2017-08.pgn",
    "Lichess Elite Database fens/lichess_elite_2017-09.pgn",
    "Lichess Elite Database fens/lichess_elite_2017-10.pgn",
    "Lichess Elite Database fens/lichess_elite_2017-11.pgn",
    "Lichess Elite Database fens/lichess_elite_2017-12.pgn",
    "Lichess Elite Database fens/lichess_elite_2018-01.pgn",
    "Lichess Elite Database fens/lichess_elite_2018-02.pgn",
    "Lichess Elite Database fens/lichess_elite_2018-03.pgn",
    "Lichess Elite Database fens/lichess_elite_2018-04.pgn",
    "Lichess Elite Database fens/lichess_elite_2018-05.pgn",
    "Lichess Elite Database fens/lichess_elite_2018-06.pgn",
    "Lichess Elite Database fens/lichess_elite_2018-07.pgn",
    "Lichess Elite Database fens/lichess_elite_2018-08.pgn",
    "Lichess Elite Database fens/lichess_elite_2018-09.pgn",
    "Lichess Elite Database fens/lichess_elite_2018-10.pgn",
    "Lichess Elite Database fens/lichess_elite_2018-11.pgn",
    "Lichess Elite Database fens/lichess_elite_2018-12.pgn",
    "Lichess Elite Database fens/lichess_elite_2019-01.pgn",
    "Lichess Elite Database fens/lichess_elite_2019-02.pgn",
    "Lichess Elite Database fens/lichess_elite_2019-03.pgn",
    "Lichess Elite Database fens/lichess_elite_2019-04.pgn",
    "Lichess Elite Database fens/lichess_elite_2019-05.pgn",
    "Lichess Elite Database fens/lichess_elite_2019-06.pgn",
    "Lichess Elite Database fens/lichess_elite_2019-07.pgn",
    "Lichess Elite Database fens/lichess_elite_2019-08.pgn",
    "Lichess Elite Database fens/lichess_elite_2019-09.pgn",
    "Lichess Elite Database fens/lichess_elite_2019-10.pgn",
    "Lichess Elite Database fens/lichess_elite_2019-11.pgn",
    "Lichess Elite Database fens/lichess_elite_2019-12.pgn",
    "Lichess Elite Database fens/lichess_elite_2020-01.pgn",
    "Lichess Elite Database fens/lichess_elite_2020-02.pgn",
    "Lichess Elite Database fens/lichess_elite_2020-03.pgn",
    "Lichess Elite Database fens/lichess_elite_2020-04.pgn",
    "Lichess Elite Database fens/lichess_elite_2020-05.pgn"
};

void writeBook(Book *bookPages, char* fileName) {
    FILE *out;
    out = fopen(fileName, "wb");
    if (out == NULL) {
        printf("opening book not found\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < BOOK_SIZE; i++) {
        fwrite(&bookPages[i].numEntries, sizeof(int), 1, out);
        for (int j = 0; j < bookPages[i].numEntries; j++) {
            fwrite(&bookPages[i].entries[j].hash, sizeof(u64), 1, out);
            fwrite(&bookPages[i].entries[j].numMoves, sizeof(u8), 1, out);
            for (int l = 0; l < bookPages[i].entries[j].numMoves; l++) {
                fwrite(&bookPages[i].entries[j].moves[l], sizeof(Move), 1, out);
                fwrite(&bookPages[i].entries[j].occourences[l], sizeof(int), 1, out);
            }
        }
    }
    fclose(out);
    printf("finished writing book\n");
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
}

int isMoveInMoves(Move move, Move* moves, int numMoves) {
    for (int i = 0; i < numMoves; i++) {
        if (moves[i].from == move.from && 
            moves[i].to == move.to && 
            moves[i].promotesTo == move.promotesTo && 
            moves[i].isEnPassantCapture == move.isEnPassantCapture && 
            moves[i].castle == move.castle &&
            moves[i].createsEnPassant == move.createsEnPassant &&
            moves[i].pieceType == move.pieceType) {
            return i;
        }
    }
    return -1;
}

void parseBook() {
    // parse the opening book txt file to a binary file
    struct Book* bookPgs;
    FILE *fens;
    FILE *out;
    char *line = NULL;
    size_t len = 0;
    int read;
    int fileCounter = 30;

    bookPgs = calloc(BOOK_SIZE, sizeof(Book));
    readBook(bookPgs, "book30.dat");

    if (bookPgs == NULL) {
        printf("malloc book failed\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 30; i < 40; i++) {
        fens = fopen(fileList[i], "r");
        if (fens == NULL) {
            printf("fens book not found\n");
            exit(EXIT_FAILURE);
        }

        printf("file: %d\n", fileCounter);

        while ((read = getline(&line, &len, fens)) != -1) {
            if (line[0] == '\n') {break;}
            char fen[100];
            int j = 0;
            u64 lastHash = 0;
            Move tmpMoves[MAX_NUM_MOVES];
            u64 *hashList = NULL;
            
            for (int i = 0; i < strlen(line); i++) {
                bool isWhite = true;
                char castlingStr[5] = {'\0', '\0', '\0', '\0', '\0'};
                char enPassantStr[3] = {'\0', '\0', '\0'};
                int halfmoveClock;
                int fullmoveNumber;

                if (line[i] == ' ') {
                    fen[j] = '\0';
                    i++;
                    if (line[i] == 'w') {
                        isWhite = true;
                    } else {
                        isWhite = false;
                    }
                    i += 2;
                    castlingStr[0] = line[i];
                    i++;
                    if (line[i] != ' ') {
                        castlingStr[1] = line[i];
                        i++;
                        if (line[i] != ' ') {
                            castlingStr[2] = line[i];
                            i++;
                            if (line[i] != ' ') {
                                castlingStr[3] = line[i];
                                i++;
                            }
                        }
                    }
                    i++;
                    enPassantStr[0] = line[i];
                    i++;
                    if (line[i] != ' ') {
                        enPassantStr[1] = line[i];
                        i++;
                    }
                    i++;
                    if (line[i+1] == ' ') {
                        halfmoveClock = (line[i] - '0');
                    } else {
                        if (line[i+2] == ' ') {
                            halfmoveClock = (line[i] - '0') * 10 + (line[i+1] - '0');
                            i++;
                        } else {
                            halfmoveClock = (line[i] - '0') * 100 + (line[i+1] - '0') * 10 + (line[i+2] - '0');
                            i += 2;
                        }
                    }
                    i += 2;
                    if (line[i+1] == ',') {
                        fullmoveNumber = (line[i] - '0');
                    } else {
                        if (line[i+2] == ',') {
                            fullmoveNumber = (line[i] - '0') * 10 + (line[i+1] - '0');
                            i++;
                        } else {
                            fullmoveNumber = (line[i] - '0') * 100 + (line[i+1] - '0') * 10 + (line[i+2] - '0');
                            i += 2;
                        }
                    }
                    i += 2;
                    j = 0;

                    int position[64] = {0};
                    fenToPosition(fen, position);
                    initBoards(position, isWhite, castlingStr, enPassantStr, halfmoveClock, fullmoveNumber);

                    if (hashList != NULL) {
                        if (tmpMoves != NULL) {
                            if (lastHash) {
                                int y = 0;
                                while (hashList[y] != 0) {
                                    if (hashList[y] == bitboards.hash) {
                                        Book page = bookPgs[lastHash % BOOK_SIZE];
                                        bool entryFound = false;
                                        for (int t = 0; t < page.numEntries; t++) {
                                            if (lastHash == page.entries[t].hash) {
                                                entryFound = true;
                                                int index = isMoveInMoves(tmpMoves[y], page.entries[t].moves, page.entries[t].numMoves);
                                                if (index != -1) {
                                                    page.entries[t].occourences[index]++;
                                                } else {
                                                    page.entries[t].numMoves++;
                                                    page.entries[t].moves = realloc(page.entries[t].moves, page.entries[t].numMoves * sizeof(Move));
                                                    page.entries[t].moves[page.entries[t].numMoves-1] = tmpMoves[y];
                                                    page.entries[t].occourences = realloc(page.entries[t].occourences, page.entries[t].numMoves * sizeof(int));
                                                    page.entries[t].occourences[page.entries[t].numMoves-1] = 1;
                                                    if (page.entries[t].moves == NULL || page.entries[t].occourences == NULL) {
                                                        printf("realloc failed-\n");
                                                        exit(EXIT_FAILURE);
                                                    }
                                                }
                                                bookPgs[lastHash % BOOK_SIZE] = page;
                                                break;
                                            }
                                        }
                                        if (!entryFound) {
                                            if (page.numEntries == 0) {
                                                page.entries = malloc(sizeof(BookEntry));
                                            } else {
                                                page.entries = realloc(page.entries, (page.numEntries + 1) * sizeof(BookEntry));
                                            }
                                            page.entries[page.numEntries].hash = lastHash;
                                            page.entries[page.numEntries].numMoves = 1;
                                            page.entries[page.numEntries].moves = malloc(sizeof(Move));
                                            page.entries[page.numEntries].moves[0] = tmpMoves[y];
                                            page.entries[page.numEntries].occourences = malloc(sizeof(int));
                                            page.entries[page.numEntries].occourences[0] = 1;
                                            if (page.entries[page.numEntries].moves == NULL || page.entries[page.numEntries].occourences == NULL) {
                                                printf("malloc failed-\n");
                                                exit(EXIT_FAILURE);
                                            }
                                            page.numEntries++;
                                            bookPgs[lastHash % BOOK_SIZE] = page;
                                        }
                                        break;
                                    }
                                    y++;
                                }
                            }
                        }
                        free(hashList);
                    }

                    int numMoves = 0;

                    if (bitboards.color) {
                        numMoves = possiblemoves(
                            bitboards.color, 
                            bitboards.allPieces, bitboards.enPassantSquare, bitboards.whitePieces, bitboards.blackPieces, 
                            bitboards.whitePawns, bitboards.whiteKnights, bitboards.whiteBishops, bitboards.whiteRooks, bitboards.whiteQueens, bitboards.whiteKing, 
                            bitboards.whiteCastleQueenSide, bitboards.whiteCastleKingSide, &tmpMoves[0]
                            );
                    } else {
                        numMoves = possiblemoves(
                            bitboards.color, 
                            bitboards.allPieces, bitboards.enPassantSquare, bitboards.blackPieces, bitboards.whitePieces, 
                            bitboards.blackPawns, bitboards.blackKnights, bitboards.blackBishops, bitboards.blackRooks, bitboards.blackQueens, bitboards.blackKing, 
                            bitboards.blackCastleQueenSide, bitboards.blackCastleKingSide, &tmpMoves[0]
                            );
                    }

                    hashList = malloc(sizeof(u64) * numMoves);

                    if (hashList == NULL) {
                        printf("malloc failed\n");
                        exit(EXIT_FAILURE);
                    }

                    for (int i = 0; i < numMoves; i++) {
                        hashList[i] = doMove(tmpMoves[i], bitboards).hash;
                    }

                    lastHash = bitboards.hash;

                    resetBoards();
                } else {
                    fen[j] = line[i];
                    j++;
                }
            }
        }
        fileCounter++;

        fclose(fens);
    }

    printf("finished reading fens\n");

    // FMA 35 is next

    // test the book using some fen
    int position[64] = {0};

    // rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1
    fenToPosition("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR\0", position);
    initBoards(position, 1, "KQkq", "-", 0, 1);

    printBoard(bitboards);
    u64 hash = bitboards.hash % BOOK_SIZE;
    Book page = bookPgs[hash];
    bool foundHash = false;

    for (int i = 0; i < page.numEntries; i++) {
        if (page.entries[i].hash == bitboards.hash) {
            printf("found hash\n");
            printf("moves: %d\n", page.entries[i].numMoves);
            printf("%llu [ ", page.entries[i].hash);
            for (int j = 0; j < page.entries[i].numMoves; j++) {
                printf("<{%d, %d, %d, %d, %d, %d, %d}, %d> ", 
                        page.entries[i].moves[j].from, page.entries[i].moves[j].to, 
                        page.entries[i].moves[j].pieceType, page.entries[i].moves[j].castle, 
                        page.entries[i].moves[j].isEnPassantCapture, page.entries[i].moves[j].createsEnPassant, 
                        page.entries[i].moves[j].promotesTo, 
                        page.entries[i].occourences[j]);
            }
            printf("]\n");
            foundHash = true;
        }
    }

    if (!foundHash) {
        printf("hash not found\n");
    }

    printf("writing...\n");
    writeBook(bookPgs, "book40.dat");
    free(bookPgs);

    printf("opening book again\n");
    struct Book* loadedBookPgs;
    loadedBookPgs = calloc(BOOK_SIZE, sizeof(Book));
    readBook(loadedBookPgs, "book40.dat");

    int position2[64] = {0};
    resetBoards();

    // rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1
    fenToPosition("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR\0", position2);
    initBoards(position2, 1, "KQkq", "-", 0, 1);

    printBoard(bitboards);
    u64 hash2 = bitboards.hash % BOOK_SIZE;
    Book page2 = loadedBookPgs[hash2];
    bool foundHash2 = false;

    for (int i = 0; i < page2.numEntries; i++) {
        if (page2.entries[i].hash == bitboards.hash) {
            printf("found hash\n");
            printf("moves: %d\n", page2.entries[i].numMoves);
            printf("%llu [ ", page2.entries[i].hash);
            for (int j = 0; j < page2.entries[i].numMoves; j++) {
                printf("<{%d, %d, %d, %d, %d, %d, %d}, %d> ", 
                        page2.entries[i].moves[j].from, page2.entries[i].moves[j].to, 
                        page2.entries[i].moves[j].pieceType, page2.entries[i].moves[j].castle, 
                        page2.entries[i].moves[j].isEnPassantCapture, page2.entries[i].moves[j].createsEnPassant, 
                        page2.entries[i].moves[j].promotesTo, 
                        page2.entries[i].occourences[j]);
            }
            printf("]\n");
            foundHash2 = true;
        }
    }

    if (!foundHash2) {
        printf("hash not found\n");
    }

    free(loadedBookPgs);

    exit(0);
}

int main(int argc, char *argv[]) {
    // main function with command line interface

    initKingAttacks();
    initKnightAttacks();
    initSlidingPieceAttacks(true);
    initSlidingPieceAttacks(false);
    initZobrist();
    initTables();

    // generateNewMagics();
    // parseBook();

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
            int startPosition[64] = {0};
            fenToPosition(argv[2], startPosition);
            int isWhite = strcmp(argv[3], "w") == 0 ? 1 : 0;
            initBoards(startPosition, isWhite, argv[4], argv[5], atoi(argv[6]), atoi(argv[7]));
            int eval = quiescenceSearch(bitboards, -INF, INF, 0);
            printf("quick evaluation: %d\n", eval);
            printf("running deeper evaluation (7ply)...\n");
            int deepEval = tree(bitboards, 7, -INF, INF, 7);
            printf("deeper evaluation: %d\n", deepEval);
            printBoard(bitboards);
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
            int startPosition[64] = {0};
            fenToPosition(argv[2], startPosition);
            initBoards(startPosition, isWhite, argv[4], argv[5], atoi(argv[6]), atoi(argv[7]));
            printBoard(bitboards);

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

            engineMove();

            if (useBook) {
                free(bookEntries);
            }
            freeTables();

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
            int startPosition[64] = {0};
            int isWhiteToMove = strcmp(argv[3], "w") == 0 ? 1 : 0;
            fenToPosition(argv[2], startPosition);
            initBoards(startPosition, isWhiteToMove, argv[4], argv[5], atoi(argv[6]), atoi(argv[7]));
            printBoard(bitboards);
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
            printf("what color do you want to play as? (w/b)\n");
            char color[2];
            scanf("%s", &color);
            int isPlayerWhite = strcmp(color, "w") == 0 ? 1 : 0;
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

            if (isWhiteToMove != isPlayerWhite) {
                engineMove();
            }

            while (true) {
                printf("your move: ");
                char notationMove[4];
                scanf("%s", &notationMove);
                if (strcmp(notationMove, "quit") == 0) {
                    freeTables();
                    if (useBook) {
                        free(bookEntries);
                    }
                    return 0;
                }
                struct Move move = buildMove(notationMove, bitboards);

                struct Move* legalMoves;
                int numMoves;
                if (isPlayerWhite) {
                    numMoves = possiblemoves(
                        isPlayerWhite, 
                        bitboards.allPieces, bitboards.enPassantSquare, bitboards.whitePieces, bitboards.blackPieces, 
                        bitboards.whitePawns, bitboards.whiteKnights, bitboards.whiteBishops, bitboards.whiteRooks, bitboards.whiteQueens, bitboards.whiteKing, 
                        bitboards.whiteCastleQueenSide, bitboards.whiteCastleKingSide, &legalMoves[0]
                        );
                } else {
                    numMoves = possiblemoves(
                        isPlayerWhite, 
                        bitboards.allPieces, bitboards.enPassantSquare, bitboards.blackPieces, bitboards.whitePieces, 
                        bitboards.blackPawns, bitboards.blackKnights, bitboards.blackBishops, bitboards.blackRooks, bitboards.blackQueens, bitboards.blackKing, 
                        bitboards.blackCastleQueenSide, bitboards.blackCastleKingSide, &legalMoves[0]
                        );
                }
                bool isLegal = false;
                for (int i = 0; i < numMoves; i++) {
                    if (
                        legalMoves[i].from == move.from &&
                        legalMoves[i].to == move.to &&
                        legalMoves[i].promotesTo == move.promotesTo &&
                        legalMoves[i].isEnPassantCapture == move.isEnPassantCapture &&
                        legalMoves[i].castle == move.castle &&
                        legalMoves[i].isEnPassantCapture == move.isEnPassantCapture &&
                        legalMoves[i].pieceType == move.pieceType
                        ) {
                            if (legalMoves[i].castle && (isIllegalCastle(legalMoves[i], bitboards) || isInCheck(&bitboards))) {
                                continue;
                            }
                            Bitboards testBoards = doMove(move, bitboards);
                            if (!canCaptureOpponentsKing(&testBoards)) {
                                isLegal = true;
                            }
                            break;
                        }
                }
                free(legalMoves);
                if (isLegal) {
                    printf("updated board:\n");
                    updateFenClocks(move);
                    bitboards = doMove(move, bitboards);
                    printBoard(bitboards);
                    engineMove();
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
            int startPosition[64] = {0};
            fenToPosition(argv[2], startPosition);
            initBoards(startPosition, isWhite, argv[4], argv[5], 0, 0);
            printBoard(bitboards);

            printf("depth: ");
            int depth;
            scanf("%d", &depth);

            clock_t start = clock();
            int bulk = perft(depth, bitboards, depth);
            // for (int i = 0; i < 1633282; i++) {
            //     evaluate(bitboards.whitePawns, bitboards.whiteKnights, bitboards.whiteBishops, bitboards.whiteRooks, bitboards.whiteQueens, bitboards.whiteKing, bitboards.blackPawns, bitboards.blackKnights, bitboards.blackBishops, bitboards.blackRooks, bitboards.blackQueens, bitboards.blackKing); // 549.00 milliseconds
            // }
            // struct Move move = {11, 27, 0, 0, 0, 0, 0};
            // for (int i = 0; i < 13073969; i++) {
            //     canOpponentCaptureKing(false, bitboards.allPieces, bitboards.whiteKing, bitboards.blackKing, bitboards.blackQueens, bitboards.blackRooks, bitboards.blackBishops, bitboards.blackKnights, bitboards.blackPawns); // 682.00 milliseconds
            //     Bitboards newBoard = doMove(move, bitboards, 1); // 899.00 milliseconds
            // }
            // for (int i = 0; i < 2876770; i++) {
            //     struct Move *pos = possiblemoves(1, bitboards.allPieces, bitboards.enPassantSquare, bitboards.whitePieces, bitboards.blackPieces, bitboards.whitePawns, bitboards.whiteKnights, bitboards.whiteBishops, bitboards.whiteRooks, bitboards.whiteQueens, bitboards.whiteKing, bitboards.whiteCastleQueenSide, bitboards.whiteCastleKingSide);
            //     free(pos);  // 3130.00 milliseconds - moves
            //                 // 720.00 milliseconds - captures
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