#include "check.h"

bool canCaptureOpponentsKing(struct bitboards_t * bitboards) {
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

bool isInCheck(struct bitboards_t * bitboards) {
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

bool isIllegalCastle(struct move_t move, struct bitboards_t boards) {
    // checks if a castle move is illegal
    struct move_t betweenMove;
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

    struct bitboards_t newBoard = doMove(betweenMove, boards); // cant castle if in check while on the between square

    return canCaptureOpponentsKing(&newBoard);
}

bool hasLegalMoves(struct move_t *possible, struct bitboards_t boards, int numMoves) {
    // checks if there are any legal moves for the current player
    // used to detect checkmate after the search
    int removedAmount = 0;
    for (int i = 0; i < numMoves; i++) {
        bool check = false;
        if (possible[i].castle) {
            check = (isIllegalCastle(possible[i], boards) || isInCheck(&boards));
        } else {
            struct bitboards_t newBoard = doMove(possible[i], boards);
            check = canCaptureOpponentsKing(&newBoard);
        }
        if (check) {
            removedAmount++;
        }
    }
    return numMoves - removedAmount > 0;
}