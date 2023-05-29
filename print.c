#include "print.h"

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
    printf("                                           rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1\n");
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

void printBoard(struct bitboards_t boards) {
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