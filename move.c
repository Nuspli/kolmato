#include "move.h"

void doMove(move_t move, struct bitboards_t *BB, struct undo_t *undo) {

    moveCalls++;
    searchMovesPlayed++;

    undo->capturedPiece = BB->pieceList[mTo(move)];
    undo->enPassantSquare = BB->enPassantSquare;

    u64 toBit = 1ULL << mTo(move);
    u64 fromBit = 1ULL << mFrom(move);
    // flip the from bit and only flip the to bit if its not a capture
    BB->bits[allPieces] ^= ((fromBit) | ((toBit) & ~BB->bits[allPieces]));
    // update the hash
    BB->hash ^= whiteToMove;
    // update the piece list
    BB->pieceList[mTo(move)] = BB->pieceList[mFrom(move)];
    BB->pieceList[mFrom(move)] = 0;
    // now for the specific piece bitboards
    if (BB->color) {
        undo->castleKingSide = BB->whiteCastleKingSide; 
        undo->castleQueenSide = BB->whiteCastleQueenSide;

        if (mIsEp(move)) {
            // remove the captured pawn
            BB->bits[blackPawns] ^= (1ULL << (mTo(move) - 8)); BB->bits[blackPieces] ^= (1ULL << (mTo(move) - 8)); BB->bits[allPieces] ^= (1ULL << (mTo(move) - 8));

            BB->hash ^= ZOBRIST_TABLE[(mTo(move) - 8)][6];
            BB->pieceList[mTo(move) - 8] = 0;
            // also update the eval
            BB->blackEvalOpening -= pawnEvalBlackOpening[mTo(move) - 8];
            BB->blackEvalEndgame -= pawnEvalBlackEndgame[mTo(move) - 8];

        } else if (undo->capturedPiece < 0) {
            // capture move
            // here we dont need to remove the bit from the occupied squares, because the capturing piece is already there
            BB->bits[blackPieces] ^= (toBit);

            BB->bits[-(undo->capturedPiece) + 5] ^= (toBit); BB->hash ^= ZOBRIST_TABLE[mTo(move)][-(undo->capturedPiece) + 5];
            BB->blackEvalOpening -= blackEvalTables[OPENING][-(undo->capturedPiece) - 1][mTo(move)];
            BB->blackEvalEndgame -= blackEvalTables[ENDGAME][-(undo->capturedPiece) - 1][mTo(move)];
        }
        
        BB->bits[whitePieces] ^= ((fromBit) | (toBit));

        if (BB->pieceList[mTo(move)] == 1) {
            BB->bits[whitePawns] ^= (fromBit); BB->hash ^= ZOBRIST_TABLE[mFrom(move)][0];
            BB->whiteEvalOpening -= pawnEvalWhiteOpening[mFrom(move)];
            BB->whiteEvalEndgame -= pawnEvalWhiteEndgame[mFrom(move)];
            if (mTo(move) >= 56) {
                // pawn promotion
                BB->bits[mPromo(move)] ^= (toBit); BB->hash ^= ZOBRIST_TABLE[mTo(move)][mPromo(move)]; BB->pieceList[mTo(move)] = mPromo(move) + 1;
                BB->whiteEvalOpening += whiteEvalTables[OPENING][mPromo(move)][mTo(move)];
                BB->whiteEvalEndgame += whiteEvalTables[ENDGAME][mPromo(move)][mTo(move)];
                
            } else {
                BB->bits[whitePawns] ^= (toBit); BB->hash ^= ZOBRIST_TABLE[mTo(move)][0];
                BB->whiteEvalOpening += pawnEvalWhiteOpening[mTo(move)];
                BB->whiteEvalEndgame += pawnEvalWhiteEndgame[mTo(move)];
            }

        } else if (BB->pieceList[mTo(move)] == 4) {

            BB->bits[whiteRooks] ^= ((fromBit) | (toBit)); BB->hash ^= ZOBRIST_TABLE[mFrom(move)][3] ^ ZOBRIST_TABLE[mTo(move)][3];
            BB->whiteEvalOpening += rookEvalWhiteOpening[mTo(move)] - rookEvalWhiteOpening[mFrom(move)];
            BB->whiteEvalEndgame += rookEvalWhiteEndgame[mTo(move)] - rookEvalWhiteEndgame[mFrom(move)];
            if (BB->whiteCastleQueenSide && mFrom(move) == 7) {
                // if the rook moves from the a1 square, then the white queen side castle is no longer possible
                BB->whiteCastleQueenSide = false; BB->hash ^= castlingRights[3];
            } else if (BB->whiteCastleKingSide && mFrom(move) == 0) {
                BB->whiteCastleKingSide = false; BB->hash ^= castlingRights[2];
            }

        } else if (BB->pieceList[mTo(move)] == 6) {

            BB->bits[whiteKing] ^= ((fromBit) | (toBit)); BB->hash ^= ZOBRIST_TABLE[mFrom(move)][5] ^ ZOBRIST_TABLE[mTo(move)][5];
            BB->whiteEvalOpening += kingEvalWhiteOpening[mTo(move)] - kingEvalWhiteOpening[mFrom(move)];
            BB->whiteEvalEndgame += kingEvalWhiteEndgame[mTo(move)] - kingEvalWhiteEndgame[mFrom(move)];
            if (mCastle(move)) {
                // castle move
                int s;
                int o;
                if (mCastle(move) == KINGSIDE) {s = 0; o = 2;} else {s = 7; o = 4;}
                    // put the rook on its new sqaure
                    BB->bits[allPieces] ^= (1ULL << s) | (1ULL << o); BB->bits[whitePieces] ^= (1ULL << s) | (1ULL << o); BB->bits[whiteRooks] ^= (1ULL << s) | (1ULL << o);

                    BB->hash ^= ZOBRIST_TABLE[o][3] ^ ZOBRIST_TABLE[s][3]; BB->pieceList[s] = 0; BB->pieceList[o] = 4;
                    BB->whiteEvalOpening += rookEvalWhiteOpening[o] - rookEvalWhiteOpening[s];
                    BB->whiteEvalEndgame += rookEvalWhiteEndgame[o] - rookEvalWhiteEndgame[s];
                }
            if (BB->whiteCastleQueenSide) {
                BB->whiteCastleQueenSide = false; BB->hash ^= castlingRights[3];
            }
            if (BB->whiteCastleKingSide) {
                BB->whiteCastleKingSide = false; BB->hash ^= castlingRights[2];
            }

        } else {

            BB->bits[BB->pieceList[mTo(move)] - 1] ^= ((fromBit) | (toBit)); BB->hash ^= ZOBRIST_TABLE[mFrom(move)][BB->pieceList[mTo(move)] - 1] ^ ZOBRIST_TABLE[mTo(move)][BB->pieceList[mTo(move)] - 1];
            BB->whiteEvalOpening += whiteEvalTables[OPENING][BB->pieceList[mTo(move)] - 1][mTo(move)] - whiteEvalTables[OPENING][BB->pieceList[mTo(move)] - 1][mFrom(move)];
            BB->whiteEvalEndgame += whiteEvalTables[ENDGAME][BB->pieceList[mTo(move)] - 1][mTo(move)] - whiteEvalTables[ENDGAME][BB->pieceList[mTo(move)] - 1][mFrom(move)];
        }

        if (BB->enPassantSquare >= 0) {
            // if there was an en passant square, remove it from the hash
            BB->hash ^= ZOBRIST_TABLE[BB->enPassantSquare][12]; BB->enPassantSquare = -1;
        }

        if (mCrEp(move) && ((BB->bits[blackPawns] & (1ULL << (mTo(move) + 1))) || (BB->bits[blackPawns] & (1ULL << (mTo(move) - 1))))) {
            // if the move creates an en passant square and it it captureable, add it to the hash and set the bit
            BB->enPassantSquare = mTo(move) - 8; BB->hash ^= ZOBRIST_TABLE[mTo(move) - 8][12];
        }

    } else {

        undo->castleKingSide = BB->blackCastleKingSide;
        undo->castleQueenSide = BB->blackCastleQueenSide;
        // Black move
        if (mIsEp(move)) {
            BB->bits[whitePawns] ^= (1ULL << (mTo(move) + 8)); BB->bits[whitePieces] ^= (1ULL << (mTo(move) + 8)); BB->bits[allPieces] ^= (1ULL << (mTo(move) + 8));
            BB->hash ^= ZOBRIST_TABLE[mTo(move) + 8][0]; BB->pieceList[mTo(move) + 8] = 0;
            BB->whiteEvalOpening -= pawnEvalWhiteOpening[mTo(move) + 8];
            BB->whiteEvalEndgame -= pawnEvalWhiteEndgame[mTo(move) + 8];

        } else if (undo->capturedPiece > 0) {
            BB->bits[whitePieces] ^= (toBit);

            BB->bits[undo->capturedPiece - 1] ^= (toBit); BB->hash ^= ZOBRIST_TABLE[mTo(move)][undo->capturedPiece - 1];
            BB->whiteEvalOpening -= whiteEvalTables[OPENING][undo->capturedPiece - 1][mTo(move)];
            BB->whiteEvalEndgame -= whiteEvalTables[ENDGAME][undo->capturedPiece - 1][mTo(move)];
        }
        
        BB->bits[blackPieces] ^= (fromBit) | (toBit);

        if (BB->pieceList[mTo(move)] == -1) {

            BB->bits[blackPawns] ^= (fromBit); BB->hash ^= ZOBRIST_TABLE[mFrom(move)][6];
            BB->blackEvalOpening -= pawnEvalBlackOpening[mFrom(move)];
            BB->blackEvalEndgame -= pawnEvalBlackEndgame[mFrom(move)];
            if (mTo(move) <= 7) {
                BB->bits[mPromo(move) + 6] ^= (toBit); BB->hash ^= ZOBRIST_TABLE[mTo(move)][mPromo(move) + 6]; BB->pieceList[mTo(move)] = -(mPromo(move)) - 1;
                BB->blackEvalOpening += blackEvalTables[OPENING][mPromo(move)][mTo(move)];
                BB->blackEvalEndgame += blackEvalTables[ENDGAME][mPromo(move)][mTo(move)];
            
            } else {
                BB->bits[blackPawns] ^= (toBit); BB->hash ^= ZOBRIST_TABLE[mTo(move)][6];
                BB->blackEvalOpening += pawnEvalBlackOpening[mTo(move)];
                BB->blackEvalEndgame += pawnEvalBlackEndgame[mTo(move)];
            }
        
        } else if (BB->pieceList[mTo(move)] == -4) {

            BB->bits[blackRooks] ^= ((toBit) | (fromBit)); BB->hash ^= ZOBRIST_TABLE[mTo(move)][9] ^ ZOBRIST_TABLE[mFrom(move)][9];
            BB->blackEvalOpening += rookEvalBlackOpening[mTo(move)] - rookEvalBlackOpening[mFrom(move)];
            BB->blackEvalEndgame += rookEvalBlackEndgame[mTo(move)] - rookEvalBlackEndgame[mFrom(move)];
            if (BB->blackCastleQueenSide && mFrom(move) == 63) {
                BB->blackCastleQueenSide = false; BB->hash ^= castlingRights[1];
            } else if (BB->blackCastleKingSide && mFrom(move) == 56) {
                BB->blackCastleKingSide = false; BB->hash ^= castlingRights[0];
            }

        } else if (BB->pieceList[mTo(move)] == -6) {

            BB->bits[blackKing] ^= ((toBit) | (fromBit)); BB->hash ^= ZOBRIST_TABLE[mTo(move)][11] ^ ZOBRIST_TABLE[mFrom(move)][11];
            BB->blackEvalOpening += kingEvalBlackOpening[mTo(move)] - kingEvalBlackOpening[mFrom(move)];
            BB->blackEvalEndgame += kingEvalBlackEndgame[mTo(move)] - kingEvalBlackEndgame[mFrom(move)];
            if (mCastle(move)) {
                int s;
                int o;
                if (mCastle(move) == KINGSIDE) {s = 56; o = 58;} else {s = 63; o = 60;}

                    BB->bits[allPieces] ^= (1ULL << s) | (1ULL << o); BB->bits[blackPieces] ^= (1ULL << s) | (1ULL << o); BB->bits[blackRooks] ^= (1ULL << s) | (1ULL << o);
                    BB->hash ^= ZOBRIST_TABLE[o][9] ^ ZOBRIST_TABLE[s][9]; BB->pieceList[s] = 0; BB->pieceList[o] = -4;
                    BB->blackEvalOpening += rookEvalBlackOpening[o] - rookEvalBlackOpening[s];
                    BB->blackEvalEndgame += rookEvalBlackEndgame[o] - rookEvalBlackEndgame[s];
                }
            if (BB->blackCastleQueenSide) {
                BB->blackCastleQueenSide = false; BB->hash ^= castlingRights[1];
            }
            if (BB->blackCastleKingSide) {
                BB->blackCastleKingSide = false; BB->hash ^= castlingRights[0];
            }
        } else {

            BB->bits[-(BB->pieceList[mTo(move)]) + 5] ^= ((toBit) | (fromBit)); BB->hash ^= ZOBRIST_TABLE[mTo(move)][-(BB->pieceList[mTo(move)]) + 5] ^ ZOBRIST_TABLE[mFrom(move)][-(BB->pieceList[mTo(move)]) + 5];
            BB->blackEvalOpening += blackEvalTables[OPENING][-(BB->pieceList[mTo(move)]) - 1][mTo(move)] - blackEvalTables[OPENING][-(BB->pieceList[mTo(move)]) - 1][mFrom(move)];
            BB->blackEvalEndgame += blackEvalTables[ENDGAME][-(BB->pieceList[mTo(move)]) - 1][mTo(move)] - blackEvalTables[ENDGAME][-(BB->pieceList[mTo(move)]) - 1][mFrom(move)];
        }

        if (BB->enPassantSquare >= 0) {
            BB->hash ^= ZOBRIST_TABLE[BB->enPassantSquare][12]; BB->enPassantSquare = -1;
        }

        if (mCrEp(move) && ((BB->bits[whitePawns] & (1ULL << (mTo(move) + 1))) || (BB->bits[whitePawns] & (1ULL << (mTo(move) - 1))))) {
            BB->enPassantSquare = (mTo(move) + 8); BB->hash ^= ZOBRIST_TABLE[mTo(move) + 8][12];
        }
    }
    BB->color = !BB->color;
}

void undoMove(move_t move, struct bitboards_t *BB, struct undo_t *undo) {

    searchMovesPlayed--;

    u64 toBit = 1ULL << mTo(move);
    u64 fromBit = 1ULL << mFrom(move);

    BB->bits[allPieces] ^= ((fromBit) | (toBit));
    BB->hash ^= whiteToMove;
    BB->color = !BB->color;

    BB->pieceList[mFrom(move)] = BB->pieceList[mTo(move)];
    BB->pieceList[mTo(move)] = undo->capturedPiece;

    if (BB->color) {
        if (mPromo(move)) {
            BB->pieceList[mFrom(move)] = 1;
        }

        if (mIsEp(move)) {
            BB->bits[blackPawns] ^= (1ULL << (mTo(move) - 8)); BB->bits[blackPieces] ^= (1ULL << (mTo(move) - 8)); BB->bits[allPieces] ^= (1ULL << (mTo(move) - 8));

            BB->hash ^= ZOBRIST_TABLE[(mTo(move) - 8)][6]; BB->pieceList[mTo(move) - 8] = -1;
            BB->blackEvalOpening += pawnEvalBlackOpening[mTo(move) - 8];
            BB->blackEvalEndgame += pawnEvalBlackEndgame[mTo(move) - 8];

        } else if (undo->capturedPiece < 0) {
            BB->bits[blackPieces] ^= toBit; BB->bits[allPieces] ^= toBit;

            BB->bits[-(undo->capturedPiece) + 5] ^= (toBit); BB->hash ^= ZOBRIST_TABLE[mTo(move)][-(undo->capturedPiece) + 5];
            BB->blackEvalOpening += blackEvalTables[OPENING][-(undo->capturedPiece) - 1][mTo(move)];
            BB->blackEvalEndgame += blackEvalTables[ENDGAME][-(undo->capturedPiece) - 1][mTo(move)];
        }
        
        BB->bits[whitePieces] ^= ((fromBit) | (toBit));

        if (BB->pieceList[mFrom(move)] == 1) {

            BB->bits[whitePawns] ^= (fromBit); BB->hash ^= ZOBRIST_TABLE[mFrom(move)][0];
            BB->whiteEvalOpening += pawnEvalWhiteOpening[mFrom(move)];
            BB->whiteEvalEndgame += pawnEvalWhiteEndgame[mFrom(move)];
            if (mTo(move) >= 56) {
                BB->bits[mPromo(move)] ^= (toBit); BB->hash ^= ZOBRIST_TABLE[mTo(move)][mPromo(move)];
                BB->whiteEvalOpening -= whiteEvalTables[OPENING][mPromo(move)][mTo(move)];
                BB->whiteEvalEndgame -= whiteEvalTables[ENDGAME][mPromo(move)][mTo(move)];

            } else {
                BB->bits[whitePawns] ^= (toBit); BB->hash ^= ZOBRIST_TABLE[mTo(move)][0];
                BB->whiteEvalOpening -= pawnEvalWhiteOpening[mTo(move)];
                BB->whiteEvalEndgame -= pawnEvalWhiteEndgame[mTo(move)];
            }

        } else if (BB->pieceList[mFrom(move)] == 6) {

            BB->bits[whiteKing] ^= ((fromBit) | (toBit)); BB->hash ^= ZOBRIST_TABLE[mFrom(move)][5] ^ ZOBRIST_TABLE[mTo(move)][5];
            BB->whiteEvalOpening -= kingEvalWhiteOpening[mTo(move)] - kingEvalWhiteOpening[mFrom(move)];
            BB->whiteEvalEndgame -= kingEvalWhiteEndgame[mTo(move)] - kingEvalWhiteEndgame[mFrom(move)];
            if (mCastle(move)) {
                int s;
                int o;
                if (mCastle(move) == KINGSIDE) {s = 0; o = 2;} else {s = 7; o = 4;}
                    // put the rook on its new sqaure
                    BB->bits[allPieces] ^= (1ULL << s) | (1ULL << o); BB->bits[whitePieces] ^= (1ULL << s) | (1ULL << o); BB->bits[whiteRooks] ^= (1ULL << s) | (1ULL << o);

                    BB->hash ^= ZOBRIST_TABLE[o][3] ^ ZOBRIST_TABLE[s][3]; BB->pieceList[s] = 4; BB->pieceList[o] = 0;
                    BB->whiteEvalOpening -= rookEvalWhiteOpening[o] - rookEvalWhiteOpening[s];
                    BB->whiteEvalEndgame -= rookEvalWhiteEndgame[o] - rookEvalWhiteEndgame[s];
            }
        } else {

            BB->bits[BB->pieceList[mFrom(move)] - 1] ^= ((fromBit) | (toBit)); BB->hash ^= ZOBRIST_TABLE[mFrom(move)][BB->pieceList[mFrom(move)] - 1] ^ ZOBRIST_TABLE[mTo(move)][BB->pieceList[mFrom(move)] - 1];
            BB->whiteEvalOpening -= whiteEvalTables[OPENING][BB->pieceList[mFrom(move)] - 1][mTo(move)] - whiteEvalTables[OPENING][BB->pieceList[mFrom(move)] - 1][mFrom(move)];
            BB->whiteEvalEndgame -= whiteEvalTables[ENDGAME][BB->pieceList[mFrom(move)] - 1][mTo(move)] - whiteEvalTables[ENDGAME][BB->pieceList[mFrom(move)] - 1][mFrom(move)];

        }
        
        if (BB->enPassantSquare >= 0) {
            // if there is an en passant square, remove it
            BB->hash ^= ZOBRIST_TABLE[BB->enPassantSquare][12]; BB->enPassantSquare = -1;
        }

        if (undo->enPassantSquare >= 0) {
            // if there was an en passant square, add it back
            BB->hash ^= ZOBRIST_TABLE[undo->enPassantSquare][12]; BB->enPassantSquare = undo->enPassantSquare;
        }
        
        if (undo->castleQueenSide && !BB->whiteCastleQueenSide) {
            BB->whiteCastleQueenSide = true; BB->hash ^= castlingRights[3];
        }

        if (undo->castleKingSide && !BB->whiteCastleKingSide) {
            BB->whiteCastleKingSide = true; BB->hash ^= castlingRights[2];
        }

    } else {
        if (mPromo(move)) {
            BB->pieceList[mFrom(move)] = -1;
        }

        if (mIsEp(move)) {
            BB->bits[whitePawns] ^= (1ULL << (mTo(move) + 8)); BB->bits[whitePieces] ^= (1ULL << (mTo(move) + 8)); BB->bits[allPieces] ^= (1ULL << (mTo(move) + 8));
            BB->hash ^= ZOBRIST_TABLE[mTo(move) + 8][0]; BB->pieceList[mTo(move) + 8] = 1;
            BB->whiteEvalOpening += pawnEvalWhiteOpening[mTo(move) + 8];
            BB->whiteEvalEndgame += pawnEvalWhiteEndgame[mTo(move) + 8];

        } else if (undo->capturedPiece > 0) {
            BB->bits[whitePieces] ^= toBit; BB->bits[allPieces] ^= toBit;

            BB->bits[undo->capturedPiece - 1] ^= toBit; BB->hash ^= ZOBRIST_TABLE[mTo(move)][undo->capturedPiece - 1];
            BB->whiteEvalOpening += whiteEvalTables[OPENING][undo->capturedPiece - 1][mTo(move)];
            BB->whiteEvalEndgame += whiteEvalTables[ENDGAME][undo->capturedPiece - 1][mTo(move)];
        }
        
        BB->bits[blackPieces] ^= (fromBit) | (toBit);

        if (BB->pieceList[mFrom(move)] == -1) {

            BB->bits[blackPawns] ^= (fromBit); BB->hash ^= ZOBRIST_TABLE[mFrom(move)][6];
            BB->blackEvalOpening += pawnEvalBlackOpening[mFrom(move)];
            BB->blackEvalEndgame += pawnEvalBlackEndgame[mFrom(move)];
            if (mTo(move) <= 7) {
                BB->bits[mPromo(move) + 6] ^= (toBit); BB->hash ^= ZOBRIST_TABLE[mTo(move)][mPromo(move) + 6];
                BB->blackEvalOpening -= blackEvalTables[OPENING][mPromo(move)][mTo(move)];
                BB->blackEvalEndgame -= blackEvalTables[ENDGAME][mPromo(move)][mTo(move)];
                
            } else {
                BB->bits[blackPawns] ^= (toBit); BB->hash ^= ZOBRIST_TABLE[mTo(move)][6];
                BB->blackEvalOpening -= pawnEvalBlackOpening[mTo(move)];
                BB->blackEvalEndgame -= pawnEvalBlackEndgame[mTo(move)];
            }

        } else if (BB->pieceList[mFrom(move)] == -6) {

            BB->bits[blackKing] ^= ((toBit) | (fromBit)); BB->hash ^= ZOBRIST_TABLE[mTo(move)][11] ^ ZOBRIST_TABLE[mFrom(move)][11];
            BB->blackEvalOpening -= kingEvalBlackOpening[mTo(move)] - kingEvalBlackOpening[mFrom(move)];
            BB->blackEvalEndgame -= kingEvalBlackEndgame[mTo(move)] - kingEvalBlackEndgame[mFrom(move)];
            if (mCastle(move)) {
                int s;
                int o;
                if (mCastle(move) == KINGSIDE) {s = 56; o = 58;} else {s = 63; o = 60;}

                BB->bits[allPieces] ^= (1ULL << s) | (1ULL << o); BB->bits[blackPieces] ^= (1ULL << s) | (1ULL << o); BB->bits[blackRooks] ^= (1ULL << s) | (1ULL << o);
                BB->hash ^= ZOBRIST_TABLE[o][9] ^ ZOBRIST_TABLE[s][9]; BB->pieceList[s] = -4; BB->pieceList[o] = 0;
                BB->blackEvalOpening -= rookEvalBlackOpening[o] - rookEvalBlackOpening[s];
                BB->blackEvalEndgame -= rookEvalBlackEndgame[o] - rookEvalBlackEndgame[s];
            }
        } else {
            BB->bits[-(BB->pieceList[mFrom(move)]) + 5] ^= ((toBit) | (fromBit)); BB->hash ^= ZOBRIST_TABLE[mTo(move)][-(BB->pieceList[mFrom(move)]) + 5] ^ ZOBRIST_TABLE[mFrom(move)][-(BB->pieceList[mFrom(move)]) + 5];
            BB->blackEvalOpening -= blackEvalTables[OPENING][-(BB->pieceList[mFrom(move)]) - 1][mTo(move)] - blackEvalTables[OPENING][-(BB->pieceList[mFrom(move)]) - 1][mFrom(move)];
            BB->blackEvalEndgame -= blackEvalTables[ENDGAME][-(BB->pieceList[mFrom(move)]) - 1][mTo(move)] - blackEvalTables[ENDGAME][-(BB->pieceList[mFrom(move)]) - 1][mFrom(move)];
        }

        if (BB->enPassantSquare >= 0) {
            BB->hash ^= ZOBRIST_TABLE[BB->enPassantSquare][12]; BB->enPassantSquare = -1;
        }

        if (undo->enPassantSquare >= 0) {
            BB->hash ^= ZOBRIST_TABLE[undo->enPassantSquare][12]; BB->enPassantSquare = undo->enPassantSquare;
        }

        if (undo->castleQueenSide && !BB->blackCastleQueenSide) {
            BB->blackCastleQueenSide = true; BB->hash ^= castlingRights[1];
        }
        if (undo->castleKingSide && !BB->blackCastleKingSide) {
            BB->blackCastleKingSide = true; BB->hash ^= castlingRights[0];
        }
    }
}

move_t buildMove(char *move, struct bitboards_t *BB) {
    // make a useable move struct from a string in the form of e2e4
    move_t m;
    int from = 7 - (move[0] - 'a') + (move[1] - '1') * 8;
    int to =  7 - (move[2] - 'a') + (move[3] - '1') * 8;

    int castle = 0;
    int isEnPassantCapture = 0;
    int createsEnPassant = 0;
    char promotingChar;
    int promotesTo = 0;

    if (((BB->bits[whitePawns] & bit(from)) != 0) || ((BB->bits[blackPawns] & bit(from)) != 0)) {
        if (abs(from - to) == 16) {
            createsEnPassant = 1;
        } else if (BB->enPassantSquare == to) {
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
    } else if (((BB->bits[whiteKing] & bit(from)) != 0) || ((BB->bits[blackKing] & bit(from)) != 0)) {
        if (abs(from - to) == 2) {
            if (from > to) {
                castle = 1;
            } else {
                castle = 2;
            }
        }
    }

    m = MOVE(from, to, promotesTo, castle, isEnPassantCapture, createsEnPassant);

    return m;
}

u64 doMoveLight(move_t move, struct bitboards_t *BB) {
    // only the hash is important here
    u64 hash = BB->hash;

    u64 toBit = 1ULL << mTo(move);
    u64 fromBit = 1ULL << mFrom(move);

    u8 fromSquare = mFrom(move);
    u8 toSquare = mTo(move);
    hash ^= whiteToMove;

    if (BB->color) {
        if (mIsEp(move)) {
            hash ^= ZOBRIST_TABLE[(toSquare - 8)][6];

        } else if (BB->pieceList[toSquare] < 0) {
            hash ^= ZOBRIST_TABLE[toSquare][-(BB->pieceList[toSquare]) + 5];
        }

        if (BB->pieceList[fromSquare] == 1) {
            hash ^= ZOBRIST_TABLE[fromSquare][0];
            if (toSquare >= 56) {
                hash ^= ZOBRIST_TABLE[toSquare][mPromo(move)];
            } else {
                hash ^= ZOBRIST_TABLE[toSquare][0];
            }
        } else if (BB->pieceList[fromSquare] == 4) {
            hash ^= ZOBRIST_TABLE[fromSquare][3] ^ ZOBRIST_TABLE[toSquare][3];
            if (BB->whiteCastleQueenSide && fromSquare == 7) {
                hash ^= castlingRights[3];
            } else if (BB->whiteCastleKingSide && fromSquare == 0) {
                hash ^= castlingRights[2];
            }
        } else if (BB->pieceList[fromSquare] == 6) {
            hash ^= ZOBRIST_TABLE[fromSquare][5] ^ ZOBRIST_TABLE[toSquare][5];
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
            hash ^= ZOBRIST_TABLE[fromSquare][BB->pieceList[fromSquare] - 1] ^ ZOBRIST_TABLE[toSquare][BB->pieceList[fromSquare] - 1];
        }

        if (BB->enPassantSquare >= 0) {
            hash ^= ZOBRIST_TABLE[BB->enPassantSquare][12];
        }

        if (mCrEp(move) && ((BB->bits[blackPawns] & (1ULL << (toSquare + 1))) || (BB->bits[blackPawns] & (1ULL << (toSquare - 1))))) {
            hash ^= ZOBRIST_TABLE[toSquare - 8][12];
        }

    } else {
        // Black move
        if (mIsEp(move)) {
            hash ^= ZOBRIST_TABLE[toSquare + 8][0];

        } else if (BB->pieceList[toSquare] > 0) {
            hash ^= ZOBRIST_TABLE[toSquare][BB->pieceList[toSquare] - 1];
        }

        if (BB->pieceList[fromSquare] == -1) {

            hash ^= ZOBRIST_TABLE[fromSquare][6];
            if (toSquare <= 7) {
                hash ^= ZOBRIST_TABLE[toSquare][mPromo(move) + 6];
            } else {
                hash ^= ZOBRIST_TABLE[toSquare][6];
            }
        } else if (BB->pieceList[fromSquare] == -4) {
            hash ^= ZOBRIST_TABLE[toSquare][9] ^ ZOBRIST_TABLE[fromSquare][9];
            if (BB->blackCastleQueenSide && fromSquare == 63) {
                hash ^= castlingRights[1];
            } else if (BB->blackCastleKingSide && fromSquare == 56) {
                hash ^= castlingRights[0];
            }
        } else if (BB->pieceList[fromSquare] == -6) {

            hash ^= ZOBRIST_TABLE[toSquare][11] ^ ZOBRIST_TABLE[fromSquare][11];
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
            hash ^= ZOBRIST_TABLE[toSquare][-(BB->pieceList[fromSquare]) + 5] ^ ZOBRIST_TABLE[fromSquare][-(BB->pieceList[fromSquare]) + 5];
        }

        if (BB->enPassantSquare >= 0) {
            hash ^= ZOBRIST_TABLE[BB->enPassantSquare][12]; BB->enPassantSquare = -1;
        }

        if (mCrEp(move) && ((BB->bits[whitePawns] & (1ULL << (toSquare + 1))) || (BB->bits[whitePawns] & (1ULL << (toSquare - 1))))) {
            BB->enPassantSquare = (toSquare + 8); hash ^= ZOBRIST_TABLE[toSquare + 8][12];
        }
    }
    
    return hash;
}