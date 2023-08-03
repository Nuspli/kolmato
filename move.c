#include "move.h"

void doMove(move_t move, struct bitboards_t *BB, struct undo_t *undo) {

    moveCalls++;
    ply++;

    u8 f = mFrom(move);
    u8 t = mTo(move);

    undo->capturedPiece = BB->pieceList[t];
    undo->enPassantSquare = BB->enPassantSquare;

    u64 toBit = 1ULL << t;
    u64 fromBit = 1ULL << f;
    // flip the from bit and only flip the to bit if its not a capture
    BB->bits[allPieces] ^= ((fromBit) | ((toBit) & ~BB->bits[allPieces]));
    // update the hash
    BB->hash ^= whiteToMove;
    // update the piece list
    BB->pieceList[t] = BB->pieceList[f];
    BB->pieceList[f] = 0;
    // now for the specific piece bitboards

    undo->whiteCastleKingSide = BB->whiteCastleKingSide; 
    undo->whiteCastleQueenSide = BB->whiteCastleQueenSide;
    undo->blackCastleKingSide = BB->blackCastleKingSide;
    undo->blackCastleQueenSide = BB->blackCastleQueenSide;

    if (BB->color == WHITE) {

        if (t == BB->enPassantSquare && BB->pieceList[t] == 1) {
            // remove the captured pawn
            BB->bits[blackPawns] ^= (1ULL << (t - 8));
            BB->bits[blackPieces] ^= (1ULL << (t - 8));
            BB->bits[allPieces] ^= (1ULL << (t - 8));

            BB->hash ^= ZOBRIST_TABLE[(t - 8)][6];
            BB->pieceList[t - 8] = 0;
            // also update the eval
            BB->blackPositionOpening += pawnEvalWhiteOpeningPosition[(t - 8) ^ 56];
            BB->blackPositionEndgame += pawnEvalWhiteEndgamePosition[(t - 8) ^ 56];
            BB->blackMaterial += PAWNVALUE;

        } else if (undo->capturedPiece < 0) {
            // capture move
            // here we dont need to remove the bit from the occupied squares, because the capturing piece is already there
            BB->bits[blackPieces] ^= (toBit);
            BB->bits[-(undo->capturedPiece) + 5] ^= (toBit);
            BB->hash ^= ZOBRIST_TABLE[t][-(undo->capturedPiece) + 5];

            BB->blackPositionOpening += whitePositionTables[OPENING][-(undo->capturedPiece) - 1][t ^ 56];
            BB->blackPositionEndgame += whitePositionTables[ENDGAME][-(undo->capturedPiece) - 1][t ^ 56];
            BB->blackMaterial += materialValues[-(undo->capturedPiece) - 1];

            if (BB->blackCastleQueenSide && t == 63) {
                // capturing a rook will invalidate the castling move for that side
                BB->blackCastleQueenSide = false;
                BB->hash ^= castlingRights[1];

            } else if (BB->blackCastleKingSide && t == 56) {
                
                BB->blackCastleKingSide = false;
                BB->hash ^= castlingRights[0];
            }
        }
        
        BB->bits[whitePieces] ^= ((fromBit) | (toBit));

        if (BB->pieceList[t] == 1) {
            // white pawn move
            BB->bits[whitePawns] ^= (fromBit);
            BB->hash ^= ZOBRIST_TABLE[f][0];
            BB->whitePositionOpening -= pawnEvalWhiteOpeningPosition[f];
            BB->whitePositionEndgame -= pawnEvalWhiteEndgamePosition[f];

            if (t >= 56) {
                // pawn promotion
                BB->bits[mPromo(move)] ^= (toBit);
                BB->hash ^= ZOBRIST_TABLE[t][mPromo(move)];
                BB->pieceList[t] = mPromo(move) + 1;
                BB->whitePositionOpening += whitePositionTables[OPENING][mPromo(move)][t];
                BB->whitePositionEndgame += whitePositionTables[ENDGAME][mPromo(move)][t];
                BB->whiteMaterial += materialValues[mPromo(move)] - PAWNVALUE;
                
            } else {
                BB->bits[whitePawns] ^= (toBit);
                BB->hash ^= ZOBRIST_TABLE[t][0];
                BB->whitePositionOpening += pawnEvalWhiteOpeningPosition[t];
                BB->whitePositionEndgame += pawnEvalWhiteEndgamePosition[t];
            }

        } else if (BB->pieceList[t] == 4) {
            // white rook move
            BB->bits[whiteRooks] ^= ((fromBit) | (toBit));
            BB->hash ^= ZOBRIST_TABLE[f][3] ^ ZOBRIST_TABLE[t][3];
            BB->whitePositionOpening += rookEvalWhiteOpeningPosition[t] - rookEvalWhiteOpeningPosition[f];
            BB->whitePositionEndgame += rookEvalWhiteEndgamePosition[t] - rookEvalWhiteEndgamePosition[f];

            if (BB->whiteCastleQueenSide && f == 7) {
                // if the rook moves from the a1 square, then the white queen side castle is no longer possible
                BB->whiteCastleQueenSide = false;
                BB->hash ^= castlingRights[3];

            } else if (BB->whiteCastleKingSide && f == 0) {

                BB->whiteCastleKingSide = false;
                BB->hash ^= castlingRights[2];
            }

        } else if (BB->pieceList[t] == 6) {
            // white king move
            BB->bits[whiteKing] ^= ((fromBit) | (toBit));
            BB->hash ^= ZOBRIST_TABLE[f][5] ^ ZOBRIST_TABLE[t][5];
            BB->whitePositionOpening += kingEvalWhiteOpeningPosition[t] - kingEvalWhiteOpeningPosition[f];
            BB->whitePositionEndgame += kingEvalWhiteEndgamePosition[t] - kingEvalWhiteEndgamePosition[f];

            if (mCastle(move)) {
                // castle move
                int s = 7 - ((mCastle(move) == KINGSIDE) * 7);
                int o = 4 - ((mCastle(move) == KINGSIDE) * 2);
                // if (mCastle(move) == KINGSIDE) {
                //     s = 0; o = 2;
                // } else {
                //     s = 7; o = 4;
                // }
                // put the rook on its new sqaure
                BB->bits[allPieces] ^= (1ULL << s) | (1ULL << o);
                BB->bits[whitePieces] ^= (1ULL << s) | (1ULL << o);
                BB->bits[whiteRooks] ^= (1ULL << s) | (1ULL << o);

                BB->hash ^= ZOBRIST_TABLE[o][3] ^ ZOBRIST_TABLE[s][3];
                BB->pieceList[s] = 0;
                BB->pieceList[o] = 4;
                BB->whitePositionOpening += rookEvalWhiteOpeningPosition[o] - rookEvalWhiteOpeningPosition[s];
                BB->whitePositionEndgame += rookEvalWhiteEndgamePosition[o] - rookEvalWhiteEndgamePosition[s];
            }
            if (BB->whiteCastleQueenSide) {
                BB->whiteCastleQueenSide = false;
                BB->hash ^= castlingRights[3];
            }
            if (BB->whiteCastleKingSide) {
                BB->whiteCastleKingSide = false;
                BB->hash ^= castlingRights[2];
            }

        } else {
            // any other piece
            BB->bits[BB->pieceList[t] - 1] ^= ((fromBit) | (toBit));
            BB->hash ^= ZOBRIST_TABLE[f][BB->pieceList[t] - 1] ^ ZOBRIST_TABLE[t][BB->pieceList[t] - 1];
            BB->whitePositionOpening += whitePositionTables[OPENING][BB->pieceList[t] - 1][t] - whitePositionTables[OPENING][BB->pieceList[t] - 1][f];
            BB->whitePositionEndgame += whitePositionTables[ENDGAME][BB->pieceList[t] - 1][t] - whitePositionTables[ENDGAME][BB->pieceList[t] - 1][f];
        }

        if (BB->enPassantSquare >= 0) {
            // if there was an en passant square, remove it from the hash
            BB->hash ^= ZOBRIST_TABLE[BB->enPassantSquare][12];
            BB->enPassantSquare = -1;
        }

        if (mCrEp(move) && ((BB->bits[blackPawns] & (1ULL << (t + 1))) || (BB->bits[blackPawns] & (1ULL << (t - 1))))) {
            // if the move creates an en passant square and it it captureable, add it to the hash and set the bit
            BB->enPassantSquare = t - 8;
            BB->hash ^= ZOBRIST_TABLE[t - 8][12];
        }

    } else {

        // Black move
        if (t == BB->enPassantSquare && BB->pieceList[t] == -1) {

            BB->bits[whitePawns] ^= (1ULL << (t + 8));
            BB->bits[whitePieces] ^= (1ULL << (t + 8));
            BB->bits[allPieces] ^= (1ULL << (t + 8));
            BB->hash ^= ZOBRIST_TABLE[t + 8][0];
            BB->pieceList[t + 8] = 0;
            BB->whitePositionOpening -= pawnEvalWhiteOpeningPosition[t + 8];
            BB->whitePositionEndgame -= pawnEvalWhiteEndgamePosition[t + 8];
            BB->whiteMaterial -= PAWNVALUE;

        } else if (undo->capturedPiece > 0) {

            BB->bits[whitePieces] ^= (toBit);
            BB->bits[undo->capturedPiece - 1] ^= (toBit);
            BB->hash ^= ZOBRIST_TABLE[t][undo->capturedPiece - 1];
            BB->whiteMaterial -= materialValues[undo->capturedPiece - 1];
            BB->whitePositionOpening -= whitePositionTables[OPENING][undo->capturedPiece - 1][t];
            BB->whitePositionEndgame -= whitePositionTables[ENDGAME][undo->capturedPiece - 1][t];

            if (BB->whiteCastleQueenSide && t == 7) {

                BB->whiteCastleQueenSide = false;
                BB->hash ^= castlingRights[3];

            } else if (BB->whiteCastleKingSide && t == 0) {

                BB->whiteCastleKingSide = false;
                BB->hash ^= castlingRights[2];
            }
        }
        
        BB->bits[blackPieces] ^= (fromBit) | (toBit);

        if (BB->pieceList[t] == -1) {
            // black pawn move
            BB->bits[blackPawns] ^= (fromBit);
            BB->hash ^= ZOBRIST_TABLE[f][6];
            BB->blackPositionOpening += pawnEvalWhiteOpeningPosition[f ^ 56];
            BB->blackPositionEndgame += pawnEvalWhiteEndgamePosition[f ^ 56];

            if (t <= 7) {
                BB->bits[mPromo(move) + 6] ^= (toBit);
                BB->hash ^= ZOBRIST_TABLE[t][mPromo(move) + 6];
                BB->pieceList[t] = -(mPromo(move)) - 1;
                BB->blackMaterial -= materialValues[mPromo(move)] - PAWNVALUE;
                BB->blackPositionOpening -= whitePositionTables[OPENING][mPromo(move)][t ^ 56];
                BB->blackPositionEndgame -= whitePositionTables[ENDGAME][mPromo(move)][t ^ 56];
            
            } else {
                BB->bits[blackPawns] ^= (toBit);
                BB->hash ^= ZOBRIST_TABLE[t][6];
                BB->blackPositionOpening -= pawnEvalWhiteOpeningPosition[t ^ 56];
                BB->blackPositionEndgame -= pawnEvalWhiteEndgamePosition[t ^ 56];
            }
        
        } else if (BB->pieceList[t] == -4) {

            BB->bits[blackRooks] ^= ((toBit) | (fromBit));
            BB->hash ^= ZOBRIST_TABLE[t][9] ^ ZOBRIST_TABLE[f][9];
            BB->blackPositionOpening -= rookEvalWhiteOpeningPosition[t ^ 56] - rookEvalWhiteOpeningPosition[f ^ 56];
            BB->blackPositionEndgame -= rookEvalWhiteEndgamePosition[t ^ 56] - rookEvalWhiteEndgamePosition[f ^ 56];

            if (BB->blackCastleQueenSide && f == 63) {

                BB->blackCastleQueenSide = false;
                BB->hash ^= castlingRights[1];

            } else if (BB->blackCastleKingSide && f == 56) {

                BB->blackCastleKingSide = false;
                BB->hash ^= castlingRights[0];
            }

        } else if (BB->pieceList[t] == -6) {

            BB->bits[blackKing] ^= ((toBit) | (fromBit));
            BB->hash ^= ZOBRIST_TABLE[t][11] ^ ZOBRIST_TABLE[f][11];
            BB->blackPositionOpening -= kingEvalWhiteOpeningPosition[t ^ 56] - kingEvalWhiteOpeningPosition[f ^ 56];
            BB->blackPositionEndgame -= kingEvalWhiteEndgamePosition[t ^ 56] - kingEvalWhiteEndgamePosition[f ^ 56];

            if (mCastle(move)) {
                int s = 63 - ((mCastle(move) == KINGSIDE) * 7);
                int o = 60 - ((mCastle(move) == KINGSIDE) * 2);
                // if (mCastle(move) == KINGSIDE) {
                //     s = 56; o = 58;
                // } else {
                //     s = 63; o = 60;
                // }
                BB->bits[allPieces] ^= (1ULL << s) | (1ULL << o);
                BB->bits[blackPieces] ^= (1ULL << s) | (1ULL << o);
                BB->bits[blackRooks] ^= (1ULL << s) | (1ULL << o);
                BB->hash ^= ZOBRIST_TABLE[o][9] ^ ZOBRIST_TABLE[s][9];
                BB->pieceList[s] = 0; BB->pieceList[o] = -4;
                BB->blackPositionOpening -= rookEvalWhiteOpeningPosition[o ^ 56] - rookEvalWhiteOpeningPosition[s ^ 56];
                BB->blackPositionEndgame -= rookEvalWhiteEndgamePosition[o ^ 56] - rookEvalWhiteEndgamePosition[s ^ 56];
            }
            if (BB->blackCastleQueenSide) {

                BB->blackCastleQueenSide = false;
                BB->hash ^= castlingRights[1];
            }
            if (BB->blackCastleKingSide) {

                BB->blackCastleKingSide = false;
                BB->hash ^= castlingRights[0];
            }
        } else {

            BB->bits[-(BB->pieceList[t]) + 5] ^= ((toBit) | (fromBit));
            BB->hash ^= ZOBRIST_TABLE[t][-(BB->pieceList[t]) + 5] ^ ZOBRIST_TABLE[f][-(BB->pieceList[t]) + 5];
            BB->blackPositionOpening -= whitePositionTables[OPENING][-(BB->pieceList[t]) - 1][t ^ 56] - whitePositionTables[OPENING][-(BB->pieceList[t]) - 1][f ^ 56];
            BB->blackPositionEndgame -= whitePositionTables[ENDGAME][-(BB->pieceList[t]) - 1][t ^ 56] - whitePositionTables[ENDGAME][-(BB->pieceList[t]) - 1][f ^ 56];
        }

        if (BB->enPassantSquare >= 0) {
            BB->hash ^= ZOBRIST_TABLE[BB->enPassantSquare][12];
            BB->enPassantSquare = -1;
        }

        if (mCrEp(move) && ((BB->bits[whitePawns] & (1ULL << (t + 1))) || (BB->bits[whitePawns] & (1ULL << (t - 1))))) {
            BB->enPassantSquare = (t + 8);
            BB->hash ^= ZOBRIST_TABLE[t + 8][12];
        }
    }
    BB->color = -BB->color;
    searchHistory[ply] = BB->hash;
}

void undoMove(move_t move, struct bitboards_t *BB, struct undo_t *undo) {

    ply--;

    u8 t = mTo(move);
    u8 f = mFrom(move);

    u64 toBit = 1ULL << t;
    u64 fromBit = 1ULL << f;

    BB->bits[allPieces] ^= ((fromBit) | (toBit));
    BB->hash ^= whiteToMove;
    BB->color = -BB->color;

    BB->pieceList[f] = BB->pieceList[t];
    BB->pieceList[t] = undo->capturedPiece;

    if (BB->color == WHITE) {
        if (mPromo(move)) {
            BB->pieceList[f] = 1;
        }

        if (t == undo->enPassantSquare && BB->pieceList[f] == 1) {

            BB->bits[blackPawns] ^= (1ULL << (t - 8));
            BB->bits[blackPieces] ^= (1ULL << (t - 8));
            BB->bits[allPieces] ^= (1ULL << (t - 8));

            BB->hash ^= ZOBRIST_TABLE[(t - 8)][6];
            BB->pieceList[t - 8] = -1;

            BB->blackMaterial -= PAWNVALUE;
            BB->blackPositionOpening -= pawnEvalWhiteOpeningPosition[(t - 8) ^ 56];
            BB->blackPositionEndgame -= pawnEvalWhiteEndgamePosition[(t - 8) ^ 56];

        } else if (undo->capturedPiece < 0) {

            BB->bits[blackPieces] ^= toBit;
            BB->bits[allPieces] ^= toBit;

            BB->bits[-(undo->capturedPiece) + 5] ^= (toBit);
            BB->hash ^= ZOBRIST_TABLE[t][-(undo->capturedPiece) + 5];
            BB->blackMaterial -= materialValues[-(undo->capturedPiece) - 1];
            BB->blackPositionOpening -= whitePositionTables[OPENING][-(undo->capturedPiece) - 1][t ^ 56];
            BB->blackPositionEndgame -= whitePositionTables[ENDGAME][-(undo->capturedPiece) - 1][t ^ 56];
        }
        
        BB->bits[whitePieces] ^= ((fromBit) | (toBit));

        if (BB->pieceList[f] == 1) {

            BB->bits[whitePawns] ^= (fromBit);
            BB->hash ^= ZOBRIST_TABLE[f][0];
            BB->whitePositionOpening += pawnEvalWhiteOpeningPosition[f];
            BB->whitePositionEndgame += pawnEvalWhiteEndgamePosition[f];

            if (t >= 56) {

                BB->bits[mPromo(move)] ^= (toBit);
                BB->hash ^= ZOBRIST_TABLE[t][mPromo(move)];
                BB->whiteMaterial -= materialValues[mPromo(move)] - PAWNVALUE;
                BB->whitePositionOpening -= whitePositionTables[OPENING][mPromo(move)][t];
                BB->whitePositionEndgame -= whitePositionTables[ENDGAME][mPromo(move)][t];

            } else {

                BB->bits[whitePawns] ^= (toBit);
                BB->hash ^= ZOBRIST_TABLE[t][0];
                BB->whitePositionOpening -= pawnEvalWhiteOpeningPosition[t];
                BB->whitePositionEndgame -= pawnEvalWhiteEndgamePosition[t];
            }

        } else if (BB->pieceList[f] == 6) {

            BB->bits[whiteKing] ^= ((fromBit) | (toBit));
            BB->hash ^= ZOBRIST_TABLE[f][5] ^ ZOBRIST_TABLE[t][5];
            BB->whitePositionOpening -= kingEvalWhiteOpeningPosition[t] - kingEvalWhiteOpeningPosition[f];
            BB->whitePositionEndgame -= kingEvalWhiteEndgamePosition[t] - kingEvalWhiteEndgamePosition[f];

            if (mCastle(move)) {

                int s = 7 - ((mCastle(move) == KINGSIDE) * 7);
                int o = 4 - ((mCastle(move) == KINGSIDE) * 2);

                BB->bits[allPieces] ^= (1ULL << s) | (1ULL << o);
                BB->bits[whitePieces] ^= (1ULL << s) | (1ULL << o);
                BB->bits[whiteRooks] ^= (1ULL << s) | (1ULL << o);

                BB->hash ^= ZOBRIST_TABLE[o][3] ^ ZOBRIST_TABLE[s][3];
                BB->pieceList[s] = 4;
                BB->pieceList[o] = 0;

                BB->whitePositionOpening -= rookEvalWhiteOpeningPosition[o] - rookEvalWhiteOpeningPosition[s];
                BB->whitePositionEndgame -= rookEvalWhiteEndgamePosition[o] - rookEvalWhiteEndgamePosition[s];
            }
        } else {

            BB->bits[BB->pieceList[f] - 1] ^= ((fromBit) | (toBit));
            BB->hash ^= ZOBRIST_TABLE[f][BB->pieceList[f] - 1] ^ ZOBRIST_TABLE[t][BB->pieceList[f] - 1];
            BB->whitePositionOpening -= whitePositionTables[OPENING][BB->pieceList[f] - 1][t] - whitePositionTables[OPENING][BB->pieceList[f] - 1][f];
            BB->whitePositionEndgame -= whitePositionTables[ENDGAME][BB->pieceList[f] - 1][t] - whitePositionTables[ENDGAME][BB->pieceList[f] - 1][f];
        }
        
        if (BB->enPassantSquare >= 0) {
            // if there is an en passant square, remove it
            BB->hash ^= ZOBRIST_TABLE[BB->enPassantSquare][12];
            BB->enPassantSquare = -1;
        }

        if (undo->enPassantSquare >= 0) {
            // if there was an en passant square, add it back
            BB->hash ^= ZOBRIST_TABLE[undo->enPassantSquare][12];
            BB->enPassantSquare = undo->enPassantSquare;
        }

    } else {

        if (mPromo(move)) {
            BB->pieceList[f] = -1;
        }

        if (t == undo->enPassantSquare && BB->pieceList[f] == -1) {

            BB->bits[whitePawns] ^= (1ULL << (t + 8));
            BB->bits[whitePieces] ^= (1ULL << (t + 8));
            BB->bits[allPieces] ^= (1ULL << (t + 8));
            BB->hash ^= ZOBRIST_TABLE[t + 8][0];
            BB->pieceList[t + 8] = 1;
            BB->whiteMaterial += PAWNVALUE;
            BB->whitePositionOpening += pawnEvalWhiteOpeningPosition[t + 8];
            BB->whitePositionEndgame += pawnEvalWhiteEndgamePosition[t + 8];

        } else if (undo->capturedPiece > 0) {
            BB->bits[whitePieces] ^= toBit;
            BB->bits[allPieces] ^= toBit;

            BB->bits[undo->capturedPiece - 1] ^= toBit;
            BB->hash ^= ZOBRIST_TABLE[t][undo->capturedPiece - 1];
            BB->whiteMaterial += materialValues[undo->capturedPiece - 1];
            BB->whitePositionOpening += whitePositionTables[OPENING][undo->capturedPiece - 1][t];
            BB->whitePositionEndgame += whitePositionTables[ENDGAME][undo->capturedPiece - 1][t];
        }
        
        BB->bits[blackPieces] ^= (fromBit) | (toBit);

        if (BB->pieceList[f] == -1) {

            BB->bits[blackPawns] ^= (fromBit);
            BB->hash ^= ZOBRIST_TABLE[f][6];
            BB->blackPositionOpening -= pawnEvalWhiteOpeningPosition[f ^ 56];
            BB->blackPositionEndgame -= pawnEvalWhiteEndgamePosition[f ^ 56];

            if (t <= 7) {
                BB->bits[mPromo(move) + 6] ^= (toBit);
                BB->hash ^= ZOBRIST_TABLE[t][mPromo(move) + 6];
                BB->blackMaterial += materialValues[mPromo(move)] - PAWNVALUE;
                BB->blackPositionOpening += whitePositionTables[OPENING][mPromo(move)][t ^ 56];
                BB->blackPositionEndgame += whitePositionTables[ENDGAME][mPromo(move)][t ^ 56];
                
            } else {
                BB->bits[blackPawns] ^= (toBit);
                BB->hash ^= ZOBRIST_TABLE[t][6];
                BB->blackPositionOpening += pawnEvalWhiteOpeningPosition[t ^ 56];
                BB->blackPositionEndgame += pawnEvalWhiteEndgamePosition[t ^ 56];
            }

        } else if (BB->pieceList[f] == -6) {

            BB->bits[blackKing] ^= ((toBit) | (fromBit));
            BB->hash ^= ZOBRIST_TABLE[t][11] ^ ZOBRIST_TABLE[f][11];
            BB->blackPositionOpening += kingEvalWhiteOpeningPosition[t ^ 56] - kingEvalWhiteOpeningPosition[f ^ 56];
            BB->blackPositionEndgame += kingEvalWhiteEndgamePosition[t ^ 56] - kingEvalWhiteEndgamePosition[f ^ 56];

            if (mCastle(move)) {
                int s = 63 - ((mCastle(move) == KINGSIDE) * 7);
                int o = 60 - ((mCastle(move) == KINGSIDE) * 2);

                BB->bits[allPieces] ^= (1ULL << s) | (1ULL << o);
                BB->bits[blackPieces] ^= (1ULL << s) | (1ULL << o);
                BB->bits[blackRooks] ^= (1ULL << s) | (1ULL << o);
                BB->hash ^= ZOBRIST_TABLE[o][9] ^ ZOBRIST_TABLE[s][9];
                BB->pieceList[s] = -4;
                BB->pieceList[o] = 0;
                BB->blackPositionOpening += rookEvalWhiteOpeningPosition[o ^ 56] - rookEvalWhiteOpeningPosition[s ^ 56];
                BB->blackPositionEndgame += rookEvalWhiteEndgamePosition[o ^ 56] - rookEvalWhiteEndgamePosition[s ^ 56];
            }

        } else {

            BB->bits[-(BB->pieceList[f]) + 5] ^= ((toBit) | (fromBit));
            BB->hash ^= ZOBRIST_TABLE[t][-(BB->pieceList[f]) + 5] ^ ZOBRIST_TABLE[f][-(BB->pieceList[f]) + 5];
            BB->blackPositionOpening += whitePositionTables[OPENING][-(BB->pieceList[f]) - 1][t ^ 56] - whitePositionTables[OPENING][-(BB->pieceList[f]) - 1][f ^ 56];
            BB->blackPositionEndgame += whitePositionTables[ENDGAME][-(BB->pieceList[f]) - 1][t ^ 56] - whitePositionTables[ENDGAME][-(BB->pieceList[f]) - 1][f ^ 56];
        }

        if (BB->enPassantSquare >= 0) {
            BB->hash ^= ZOBRIST_TABLE[BB->enPassantSquare][12];
            BB->enPassantSquare = -1;
        }

        if (undo->enPassantSquare >= 0) {
            BB->hash ^= ZOBRIST_TABLE[undo->enPassantSquare][12];
            BB->enPassantSquare = undo->enPassantSquare;
        }

    }

    if (undo->whiteCastleQueenSide && !BB->whiteCastleQueenSide) {
        BB->whiteCastleQueenSide = true;
        BB->hash ^= castlingRights[3];
    }
    if (undo->whiteCastleKingSide && !BB->whiteCastleKingSide) {
        BB->whiteCastleKingSide = true;
        BB->hash ^= castlingRights[2];
    }
    if (undo->blackCastleQueenSide && !BB->blackCastleQueenSide) {
        BB->blackCastleQueenSide = true;
        BB->hash ^= castlingRights[1];
    }
    if (undo->blackCastleKingSide && !BB->blackCastleKingSide) {
        BB->blackCastleKingSide = true;
        BB->hash ^= castlingRights[0];
    }
}

move_t buildMove(char *move, struct bitboards_t *BB) {
    // make a useable move struct from a string in the form of e2e4
    move_t m;
    u8 from = 7 - (move[0] - 'a') + (move[1] - '1') * 8;
    u8 to =  7 - (move[2] - 'a') + (move[3] - '1') * 8;

    u8 castle = 0;
    u8 createsEnPassant = 0;
    char promotingChar;
    u8 promotesTo = 0;

    if (((BB->bits[whitePawns] & bit(from)) != 0) || ((BB->bits[blackPawns] & bit(from)) != 0)) {
        if (abs(from - to) == 16) {
            createsEnPassant = 1;
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
    } else if (((BB->bits[whiteKing] & bit(from)) != 0) || ((BB->bits[blackKing] & bit(from)) != 0)) {
        if (abs(from - to) == 2) {
            if (from > to) {
                castle = 1;
            } else {
                castle = 2;
            }
        }
    }

    m = MOVE(from, to, promotesTo, castle, createsEnPassant);

    return m;
}

u64 doMoveLight(move_t move, struct bitboards_t *BB) {
    // only the hash is important here
    u64 hash = BB->hash;

    u8 t = mTo(move);
    u8 f = mFrom(move);

    u64 toBit = 1ULL << t;
    u64 fromBit = 1ULL << f;

    hash ^= whiteToMove;

    if (BB->color == WHITE) {
        if (t == BB->enPassantSquare && BB->pieceList[f] == 1) {
            hash ^= ZOBRIST_TABLE[(t - 8)][6];

        } else if (BB->pieceList[t] < 0) {
            hash ^= ZOBRIST_TABLE[t][-(BB->pieceList[t]) + 5];
        }

        if (BB->pieceList[f] == 1) {
            hash ^= ZOBRIST_TABLE[f][0];
            if (t >= 56) {
                hash ^= ZOBRIST_TABLE[t][mPromo(move)];
            } else {
                hash ^= ZOBRIST_TABLE[t][0];
            }
        } else if (BB->pieceList[f] == 4) {
            hash ^= ZOBRIST_TABLE[f][3] ^ ZOBRIST_TABLE[t][3];
            if (BB->whiteCastleQueenSide && f == 7) {
                hash ^= castlingRights[3];
            } else if (BB->whiteCastleKingSide && f == 0) {
                hash ^= castlingRights[2];
            }
        } else if (BB->pieceList[f] == 6) {
            hash ^= ZOBRIST_TABLE[f][5] ^ ZOBRIST_TABLE[t][5];
            if (mCastle(move)) {
                int s;
                int o;
                if (mCastle(move) == KINGSIDE) {s = 0; o = 2;} else {s = 7; o = 4;}
                    hash ^= ZOBRIST_TABLE[o][3] ^ ZOBRIST_TABLE[s][3];
                }
            if (BB->whiteCastleQueenSide) {
                hash ^= castlingRights[3];
            }
            if (BB->whiteCastleKingSide) {
                hash ^= castlingRights[2];
            }
        } else {
            hash ^= ZOBRIST_TABLE[f][BB->pieceList[f] - 1] ^ ZOBRIST_TABLE[t][BB->pieceList[f] - 1];
        }

        if (BB->enPassantSquare >= 0) {
            hash ^= ZOBRIST_TABLE[BB->enPassantSquare][12];
        }

        if (mCrEp(move) && ((BB->bits[blackPawns] & (1ULL << (t + 1))) || (BB->bits[blackPawns] & (1ULL << (t - 1))))) {
            hash ^= ZOBRIST_TABLE[t - 8][12];
        }

    } else {
        // Black move
        if (t == BB->enPassantSquare && BB->pieceList[f] == -1) {
            hash ^= ZOBRIST_TABLE[t + 8][0];

        } else if (BB->pieceList[t] > 0) {
            hash ^= ZOBRIST_TABLE[t][BB->pieceList[t] - 1];
        }

        if (BB->pieceList[f] == -1) {

            hash ^= ZOBRIST_TABLE[f][6];
            if (t <= 7) {
                hash ^= ZOBRIST_TABLE[t][mPromo(move) + 6];
            } else {
                hash ^= ZOBRIST_TABLE[t][6];
            }
        } else if (BB->pieceList[f] == -4) {
            hash ^= ZOBRIST_TABLE[t][9] ^ ZOBRIST_TABLE[f][9];
            if (BB->blackCastleQueenSide && f == 63) {
                hash ^= castlingRights[1];
            } else if (BB->blackCastleKingSide && f == 56) {
                hash ^= castlingRights[0];
            }
        } else if (BB->pieceList[f] == -6) {

            hash ^= ZOBRIST_TABLE[t][11] ^ ZOBRIST_TABLE[f][11];
            if (mCastle(move)) {
                int s;
                int o;
                if (mCastle(move) == KINGSIDE) {s = 56; o = 58;} else {s = 63; o = 60;}
                    hash ^= ZOBRIST_TABLE[o][9] ^ ZOBRIST_TABLE[s][9];
                }
            if (BB->blackCastleQueenSide) {
                hash ^= castlingRights[1];
            }
            if (BB->blackCastleKingSide) {
                hash ^= castlingRights[0];
            }
        } else {
            hash ^= ZOBRIST_TABLE[t][-(BB->pieceList[f]) + 5] ^ ZOBRIST_TABLE[f][-(BB->pieceList[f]) + 5];
        }

        if (BB->enPassantSquare >= 0) {
            hash ^= ZOBRIST_TABLE[BB->enPassantSquare][12]; BB->enPassantSquare = -1;
        }

        if (mCrEp(move) && ((BB->bits[whitePawns] & (1ULL << (t + 1))) || (BB->bits[whitePawns] & (1ULL << (t - 1))))) {
            BB->enPassantSquare = (t + 8); hash ^= ZOBRIST_TABLE[t + 8][12];
        }
    }
    
    return hash;
}