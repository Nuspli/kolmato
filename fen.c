#include "fen.h"

void fenToPosition(char* fen, bitboards_t *bitboards) {
    
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
        bitboards->pieceList[63 - (rank * 8 + file)] = piece;
        file++;
    }
}

void updateFenClocks(move_t move) {
    // update the clocks in the fen string
    fullMoveCount++;
    if (bitboards->pieceList[mFrom(move)] == 1 || bitboards->pieceList[mFrom(move)] == -1 || checkBit(bitboards->allPieces, mTo(move))) {
        halfMoveCount = 0;
    } else {
        halfMoveCount++;
    }

    if (halfMoveCount >= 100) {
        printf("draw by 50 move rule\n");
        // todo: make the engine see that too using another game state table
        exit(0);
    }
}