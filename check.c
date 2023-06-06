#include "check.h"

bool canCaptureOpponentsKing(struct bitboards_t *BITBOARDS) {
    int kingIndex = 0;
    if (BITBOARDS->color) {
        // as this is only used after a move way played to check its validity, if white is to move
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

bool isIllegalCastle(struct move_t *move, struct bitboards_t *BITBOARDS) {
    // checks if a castle move goes through check by making an inbetween move
    struct move_t betweenMove;
    if (move->castle == QUEENSIDE) {
        betweenMove.from = move->from;
        betweenMove.to = move->to - 1;
        betweenMove.pieceType = 5;
        betweenMove.isEnPassantCapture = false;
        betweenMove.createsEnPassant = false;
        betweenMove.castle = 0;
    } else {
        betweenMove.from = move->from;
        betweenMove.to = move->to + 1;
        betweenMove.pieceType = 5;
        betweenMove.isEnPassantCapture = false;
        betweenMove.createsEnPassant = false;
        betweenMove.castle = 0;
    }

    struct undo_t undo;
    doMove(&betweenMove, BITBOARDS, &undo);

    bool illegalCastle = false;

    if (canCaptureOpponentsKing(BITBOARDS)) {
        illegalCastle = true;
    }

    undoMove(&betweenMove, BITBOARDS, &undo);

    return illegalCastle;
}

bool hasLegalMoves(struct move_t *possible, struct bitboards_t *boards, int numMoves) {
    // checks if there are any legal moves for the current player
    // used to detect checkmate after the search
    int removedAmount = 0;
    for (int i = 0; i < numMoves; i++) {
        bool check = false;
        if (possible[i].castle) {
            check = (isIllegalCastle(&possible[i], boards) || isInCheck(boards));
        } else {
            struct undo_t undo;
            doMove(&possible[i], boards, &undo);
            check = canCaptureOpponentsKing(boards);
            undoMove(&possible[i], boards, &undo);
        }
        if (check) {
            removedAmount++;
        }
    }
    return numMoves - removedAmount > 0;
}