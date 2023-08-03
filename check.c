#include "check.h"

bool canCaptureOpponentsKing(struct bitboards_t *BITBOARDS) {
    int colorOffsetSideToMove = ((BITBOARDS->color == BLACK) * 6);
    int colorOffsetOpponent = ((BITBOARDS->color == WHITE) * 6);

    // as this is only used after a move was played to check its validity, if white is to move
    // white is the opponent and this checks if white can capture the black king

    int opponentsKingIndex = lsb(BITBOARDS->bits[whiteKing + colorOffsetOpponent]);

    // pretend the king was every other piece
    // if that piece which is on the kings position can capture a piece of its kind from there,
    // that enemy piece can also capture the king

    return (

        (kingAttacks[opponentsKingIndex] & BITBOARDS->bits[whiteKing + colorOffsetSideToMove]) | 
        (knightAttacks[opponentsKingIndex] & BITBOARDS->bits[whiteKnights + colorOffsetSideToMove]) |
        (generateBishopMoves(opponentsKingIndex, BITBOARDS->bits[allPieces]) & (BITBOARDS->bits[whiteBishops + colorOffsetSideToMove] | BITBOARDS->bits[whiteQueens + colorOffsetSideToMove])) |
        (generateRookMoves(opponentsKingIndex, BITBOARDS->bits[allPieces]) & (BITBOARDS->bits[whiteRooks + colorOffsetSideToMove] | BITBOARDS->bits[whiteQueens + colorOffsetSideToMove])) |
        (
            (BITBOARDS->color == WHITE) *
            (((BITBOARDS->bits[whiteKing + colorOffsetOpponent] >> 7) & notRightmostFileMask) | ((BITBOARDS->bits[whiteKing + colorOffsetOpponent] >> 9) & notLeftmostFileMask)) & BITBOARDS->bits[whitePawns + colorOffsetSideToMove]
        ) | 
        (
            (BITBOARDS->color == BLACK) *
            (((BITBOARDS->bits[whiteKing + colorOffsetOpponent] << 7) & notLeftmostFileMask) | ((BITBOARDS->bits[whiteKing + colorOffsetOpponent] << 9) & notRightmostFileMask)) & BITBOARDS->bits[whitePawns + colorOffsetSideToMove]
        )

    ) != 0;
}

bool isInCheck(struct bitboards_t *BITBOARDS, u64 *checkers) {
    int colorOffsetSideToMove = ((BITBOARDS->color == BLACK) * 6);
    int colorOffsetOpponent = ((BITBOARDS->color == WHITE) * 6);

    int playerKingIndex = lsb(BITBOARDS->bits[whiteKing + colorOffsetSideToMove]);

    // same as before only this time we check if the side to move is in check and used on the board before making a move
    // mainly to see if a player can castle or not

    *checkers = (

        (kingAttacks[playerKingIndex] & BITBOARDS->bits[whiteKing + colorOffsetOpponent]) | 
        (knightAttacks[playerKingIndex] & BITBOARDS->bits[whiteKnights + colorOffsetOpponent]) |
        (generateBishopMoves(playerKingIndex, BITBOARDS->bits[allPieces]) & (BITBOARDS->bits[whiteBishops + colorOffsetOpponent] | BITBOARDS->bits[whiteQueens + colorOffsetOpponent])) |
        (generateRookMoves(playerKingIndex, BITBOARDS->bits[allPieces]) & (BITBOARDS->bits[whiteRooks + colorOffsetOpponent] | BITBOARDS->bits[whiteQueens + colorOffsetOpponent])) |
        (
            (BITBOARDS->color == BLACK) *
            (((BITBOARDS->bits[whiteKing + colorOffsetSideToMove] >> 7) & notRightmostFileMask) | ((BITBOARDS->bits[whiteKing + colorOffsetSideToMove] >> 9) & notLeftmostFileMask)) & BITBOARDS->bits[whitePawns + colorOffsetOpponent]
        ) | 
        (
            (BITBOARDS->color == WHITE) *
            (((BITBOARDS->bits[whiteKing + colorOffsetSideToMove] << 7) & notLeftmostFileMask) | ((BITBOARDS->bits[whiteKing + colorOffsetSideToMove] << 9) & notRightmostFileMask)) & BITBOARDS->bits[whitePawns + colorOffsetOpponent]
        )

    );

    return *checkers != 0;
}

bool isInCheckLight(struct bitboards_t *BITBOARDS) {
    // in case we dont want to pass a checkers pointer
    int colorOffsetSideToMove = ((BITBOARDS->color == BLACK) * 6);
    int colorOffsetOpponent = ((BITBOARDS->color == WHITE) * 6);

    int playerKingIndex = lsb(BITBOARDS->bits[whiteKing + colorOffsetSideToMove]);

    return (

        (kingAttacks[playerKingIndex] & BITBOARDS->bits[whiteKing + colorOffsetOpponent]) | 
        (knightAttacks[playerKingIndex] & BITBOARDS->bits[whiteKnights + colorOffsetOpponent]) |
        (generateBishopMoves(playerKingIndex, BITBOARDS->bits[allPieces]) & (BITBOARDS->bits[whiteBishops + colorOffsetOpponent] | BITBOARDS->bits[whiteQueens + colorOffsetOpponent])) |
        (generateRookMoves(playerKingIndex, BITBOARDS->bits[allPieces]) & (BITBOARDS->bits[whiteRooks + colorOffsetOpponent] | BITBOARDS->bits[whiteQueens + colorOffsetOpponent])) |
        (
            (BITBOARDS->color == BLACK) *
            (((BITBOARDS->bits[whiteKing + colorOffsetSideToMove] >> 7) & notRightmostFileMask) | ((BITBOARDS->bits[whiteKing + colorOffsetSideToMove] >> 9) & notLeftmostFileMask)) & BITBOARDS->bits[whitePawns + colorOffsetOpponent]
        ) | 
        (
            (BITBOARDS->color == WHITE) *
            (((BITBOARDS->bits[whiteKing + colorOffsetSideToMove] << 7) & notLeftmostFileMask) | ((BITBOARDS->bits[whiteKing + colorOffsetSideToMove] << 9) & notRightmostFileMask)) & BITBOARDS->bits[whitePawns + colorOffsetOpponent]
        )

    ) != 0;
}