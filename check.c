#include "check.h"

bool canCaptureOpponentsKing(struct bitboards_t *BITBOARDS) {
    int kingIndex = 0;
    if (BITBOARDS->color) {
        // as this is only used after a move was played to check its validity, if white is to move
        // white is the opponent and this checks if white can capture the black king
        if (BITBOARDS->blackKing) {
            kingIndex = lsb(BITBOARDS->blackKing);
        } else {
            return true;
        }
        // pretend the king was every other piece
        // if that piece which is on the kings position can capture a piece of its kind from there,
        // that enemy piece can also capture the king
        if ((kingAttacks[kingIndex] & BITBOARDS->whiteKing)) {
            return true;
        } else if ((knightAttacks[kingIndex] & BITBOARDS->whiteKnights)) {
            return true;
        } else if ((generateBishopMoves(kingIndex, BITBOARDS->allPieces) & (BITBOARDS->whiteBishops | BITBOARDS->whiteQueens))) {
            return true;
        } else if ((generateRookMoves(kingIndex, BITBOARDS->allPieces) & (BITBOARDS->whiteRooks | BITBOARDS->whiteQueens))) {
            return true;
        } else if ((((BITBOARDS->blackKing >> 7) & ~rightmostFileMask) | ((BITBOARDS->blackKing >> 9) & ~leftmostFileMask)) & BITBOARDS->whitePawns) {
            return true;
        }

    } else {

        if (BITBOARDS->whiteKing) {
            kingIndex = lsb(BITBOARDS->whiteKing);
        } else {
            return true;
        }
        if ((kingAttacks[kingIndex] & BITBOARDS->blackKing)) {
            return true;
        } else if ((knightAttacks[kingIndex] & BITBOARDS->blackKnights)) {
            return true;
        } else if ((generateBishopMoves(kingIndex, BITBOARDS->allPieces) & (BITBOARDS->blackBishops | BITBOARDS->blackQueens))) {
            return true;
        } else if ((generateRookMoves(kingIndex, BITBOARDS->allPieces) & (BITBOARDS->blackRooks | BITBOARDS->blackQueens))) {
            return true;
        } else if ((((BITBOARDS->whiteKing << 7) & ~leftmostFileMask) | ((BITBOARDS->whiteKing << 9) & ~rightmostFileMask)) & BITBOARDS->blackPawns) {
            return true;
        }
    }
    return false;
}

bool isInCheck(struct bitboards_t *BITBOARDS) {
    int kingIndex = 0;
    if (!BITBOARDS->color) {
        // same as before only this time we check if the side to move is in check and used on the board before making a move
        // mainly to see if a player can castle or not
        if (BITBOARDS->blackKing) {
            kingIndex = lsb(BITBOARDS->blackKing);
        } else {
            return true;
        }
        if ((kingAttacks[kingIndex] & BITBOARDS->whiteKing)) {
            return true;
        } else if ((knightAttacks[kingIndex] & BITBOARDS->whiteKnights)) {
            return true;
        } else if ((generateBishopMoves(kingIndex, BITBOARDS->allPieces) & (BITBOARDS->whiteBishops | BITBOARDS->whiteQueens))) {
            return true;
        } else if ((generateRookMoves(kingIndex, BITBOARDS->allPieces) & (BITBOARDS->whiteRooks | BITBOARDS->whiteQueens))) {
            return true;
        } else if ((((BITBOARDS->blackKing >> 7) & ~rightmostFileMask) | ((BITBOARDS->blackKing >> 9) & ~leftmostFileMask)) & BITBOARDS->whitePawns) {
            return true;
        }

    } else {

        if (BITBOARDS->whiteKing) {
            kingIndex = lsb(BITBOARDS->whiteKing);
        } else {
            return true;
        }
        if ((kingAttacks[kingIndex] & BITBOARDS->blackKing)) {
            return true;
        } else if ((knightAttacks[kingIndex] & BITBOARDS->blackKnights)) {
            return true;
        } else if ((generateBishopMoves(kingIndex, BITBOARDS->allPieces) & (BITBOARDS->blackBishops | BITBOARDS->blackQueens))) {
            return true;
        } else if ((generateRookMoves(kingIndex, BITBOARDS->allPieces) & (BITBOARDS->blackRooks | BITBOARDS->blackQueens))) {
            return true;
        } else if ((((BITBOARDS->whiteKing << 7) & ~leftmostFileMask) | ((BITBOARDS->whiteKing << 9) & ~rightmostFileMask)) & bitboards->blackPawns) {
            return true;
        }
    }
    return false;
}

bool isIllegalCastle(move_t move, struct bitboards_t *BITBOARDS) {
    // checks if a castle move goes through check by making an inbetween move
    bool illegalCastle = false;
    if (BITBOARDS->color) {
        if (mCastle(move) == QUEENSIDE) {
            BITBOARDS->whiteKing <<= 1;
            if (isInCheck(BITBOARDS)) {
                illegalCastle = true;
            }
            BITBOARDS->whiteKing >>= 1;
        } else {
            BITBOARDS->whiteKing >>= 1;
            if (isInCheck(BITBOARDS)) {
                illegalCastle = true;
            }
            BITBOARDS->whiteKing <<= 1;
        }
    } else {
        if (mCastle(move) == QUEENSIDE) {
            BITBOARDS->blackKing <<= 1;
            if (isInCheck(BITBOARDS)) {
                illegalCastle = true;
            }
            BITBOARDS->blackKing >>= 1;
        } else {
            BITBOARDS->blackKing >>= 1;
            if (isInCheck(BITBOARDS)) {
                illegalCastle = true;
            }
            BITBOARDS->blackKing <<= 1;
        }
    }
    
    return illegalCastle;
}

bool hasLegalMoves(move_t *possible, struct bitboards_t *boards, int numMoves) {
    // checks if there are any legal moves for the current player
    // used to detect checkmate after the search
    int removedAmount = 0;
    for (int i = 0; i < numMoves; i++) {
        bool check = false;
        if (mCastle(possible[i])) {
            check = (isIllegalCastle(possible[i], boards) || isInCheck(boards));
        } else {
            struct undo_t undo;
            doMove(possible[i], boards, &undo);
            check = canCaptureOpponentsKing(boards);
            undoMove(possible[i], boards, &undo);
        }
        if (check) {
            removedAmount++;
        }
    }
    return numMoves - removedAmount > 0;
}