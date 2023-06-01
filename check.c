#include "check.h"

bool canCaptureOpponentsKing(struct bitboards_t * bitboards) {
    int kingIndex = 0;
    if (bitboards->color) {
        // as this is only used after a move way played to check its validity, if white is to move
        // white is the opponent and this checks if white can capture the black king
        if (bitboards->blackKing) {
            kingIndex = lsb(bitboards->blackKing);
        } else {
            return true;
        }
        // pretend the king was every other piece
        // if that piece which is on the kings position can capture a piece of its kind from there,
        // that enemy piece can also capture the king
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
        // same as before only this time we check if the side to move is in check and used on the board before making a move
        // mainly to see if a player can castle or not
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
    // checks if a castle move goes through check by making an inbetween move
    struct move_t betweenMove;
    if (move.castle == QUEENSIDE) {
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

    struct bitboards_t newBoard = doMove(betweenMove, boards);

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