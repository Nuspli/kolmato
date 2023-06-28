#include "move.h"

void doMove(move_t move, struct bitboards_t *BB, struct undo_t *undo) {

    moveCalls++;
    searchMovesPlayed++;

    undo->capturedPiece = BB->pieceList[mTo(move)];
    undo->enPassantSquare = BB->enPassantSquare;

    u64 toBit = 1ULL << mTo(move);
    u64 fromBit = 1ULL << mFrom(move);
    // flip the from bit and only flip the to bit if its not a capture
    BB->allPieces ^= ((fromBit) | ((toBit) & ~BB->allPieces));
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
            BB->blackPawns ^= (1ULL << (mTo(move) - 8)); BB->blackPieces ^= (1ULL << (mTo(move) - 8)); BB->allPieces ^= (1ULL << (mTo(move) - 8));

            BB->hash ^= ZOBRIST_TABLE[(mTo(move) - 8)][6];
            BB->pieceList[mTo(move) - 8] = 0;
            // also update the eval
            BB->blackEvalOpening -= pawnEvalBlackOpening[mTo(move) - 8];
            BB->blackEvalEndgame -= pawnEvalBlackEndgame[mTo(move) - 8];

        } else if (BB->blackPieces & (toBit)) {
            // capture move
            // here we dont need to remove the bit from the occupied squares, because the capturing piece is already there
            BB->blackPieces ^= (toBit);

            if (BB->blackPawns & (toBit)) {
                BB->blackPawns ^= (toBit); BB->hash ^= ZOBRIST_TABLE[mTo(move)][6];
                BB->blackEvalOpening -= pawnEvalBlackOpening[mTo(move)];
                BB->blackEvalEndgame -= pawnEvalBlackEndgame[mTo(move)];
    
            } else if (BB->blackKnights & (toBit)) {
                BB->blackKnights ^= (toBit); BB->hash ^= ZOBRIST_TABLE[mTo(move)][7];
                BB->blackEvalOpening -= knightEvalBlackOpening[mTo(move)];
                BB->blackEvalEndgame -= knightEvalBlackEndgame[mTo(move)];
    
            } else if (BB->blackBishops & (toBit)) {
                BB->blackBishops ^= (toBit); BB->hash ^= ZOBRIST_TABLE[mTo(move)][8];
                BB->blackEvalOpening -= bishopEvalBlackOpening[mTo(move)];
                BB->blackEvalEndgame -= bishopEvalBlackEndgame[mTo(move)];
    
            } else if (BB->blackRooks & (toBit)) {
                BB->blackRooks ^= (toBit); BB->hash ^= ZOBRIST_TABLE[mTo(move)][9];
                BB->blackEvalOpening -= rookEvalBlackOpening[mTo(move)];
                BB->blackEvalEndgame -= rookEvalBlackEndgame[mTo(move)];

            } else if (BB->blackQueens & (toBit)) {
                BB->blackQueens ^= (toBit); BB->hash ^= ZOBRIST_TABLE[mTo(move)][10];
                BB->blackEvalOpening -= queenEvalBlackOpening[mTo(move)];
                BB->blackEvalEndgame -= queenEvalBlackEndgame[mTo(move)];
    
            } else if (BB->blackKing & (toBit)) {
                BB->blackKing ^= (toBit); BB->hash ^= ZOBRIST_TABLE[mTo(move)][11];
                BB->blackEvalOpening -= kingEvalBlackOpening[mTo(move)];
                BB->blackEvalEndgame -= kingEvalBlackEndgame[mTo(move)];
            }
        }
        
        BB->whitePieces ^= ((fromBit) | (toBit));

        switch (BB->pieceList[mTo(move)]) {
            case 1:
                BB->whitePawns ^= (fromBit); BB->hash ^= ZOBRIST_TABLE[mFrom(move)][0];
                BB->whiteEvalOpening -= pawnEvalWhiteOpening[mFrom(move)];
                BB->whiteEvalEndgame -= pawnEvalWhiteEndgame[mFrom(move)];
                if (mTo(move) >= 56) {
                    // pawn promotion
                    switch (mPromo(move)) {
                        case 4:
                            BB->whiteQueens ^= (toBit); BB->hash ^= ZOBRIST_TABLE[mTo(move)][4]; BB->pieceList[mTo(move)] = 5;
                            BB->whiteEvalOpening += queenEvalWhiteOpening[mTo(move)];
                            BB->whiteEvalEndgame += queenEvalWhiteEndgame[mTo(move)];
                            break;
                        case 3:
                            BB->whiteRooks ^= (toBit); BB->hash ^= ZOBRIST_TABLE[mTo(move)][3]; BB->pieceList[mTo(move)] = 4;
                            BB->whiteEvalOpening += rookEvalWhiteOpening[mTo(move)];
                            BB->whiteEvalEndgame += rookEvalWhiteEndgame[mTo(move)];
                            break;
                        case 2:
                            BB->whiteBishops ^= (toBit); BB->hash ^= ZOBRIST_TABLE[mTo(move)][2]; BB->pieceList[mTo(move)] = 3;
                            BB->whiteEvalOpening += bishopEvalWhiteOpening[mTo(move)];
                            BB->whiteEvalEndgame += bishopEvalWhiteEndgame[mTo(move)];
                            break;
                        case 1:
                            BB->whiteKnights ^= (toBit); BB->hash ^= ZOBRIST_TABLE[mTo(move)][1]; BB->pieceList[mTo(move)] = 2;
                            BB->whiteEvalOpening += knightEvalWhiteOpening[mTo(move)];
                            BB->whiteEvalEndgame += knightEvalWhiteEndgame[mTo(move)];
                            break;
                    }
                } else {
                    BB->whitePawns ^= (toBit); BB->hash ^= ZOBRIST_TABLE[mTo(move)][0];
                    BB->whiteEvalOpening += pawnEvalWhiteOpening[mTo(move)];
                    BB->whiteEvalEndgame += pawnEvalWhiteEndgame[mTo(move)];
                }
                break;

            case 2:
                BB->whiteKnights ^= ((fromBit) | (toBit)); BB->hash ^= ZOBRIST_TABLE[mFrom(move)][1] ^ ZOBRIST_TABLE[mTo(move)][1];
                BB->whiteEvalOpening += knightEvalWhiteOpening[mTo(move)] - knightEvalWhiteOpening[mFrom(move)];
                BB->whiteEvalEndgame += knightEvalWhiteEndgame[mTo(move)] - knightEvalWhiteEndgame[mFrom(move)];
                break;

            case 3:
                BB->whiteBishops ^= ((fromBit) | (toBit)); BB->hash ^= ZOBRIST_TABLE[mFrom(move)][2] ^ ZOBRIST_TABLE[mTo(move)][2];
                BB->whiteEvalOpening += bishopEvalWhiteOpening[mTo(move)] - bishopEvalWhiteOpening[mFrom(move)];
                BB->whiteEvalEndgame += bishopEvalWhiteEndgame[mTo(move)] - bishopEvalWhiteEndgame[mFrom(move)];
                break;

            case 4:
                BB->whiteRooks ^= ((fromBit) | (toBit)); BB->hash ^= ZOBRIST_TABLE[mFrom(move)][3] ^ ZOBRIST_TABLE[mTo(move)][3];
                BB->whiteEvalOpening += rookEvalWhiteOpening[mTo(move)] - rookEvalWhiteOpening[mFrom(move)];
                BB->whiteEvalEndgame += rookEvalWhiteEndgame[mTo(move)] - rookEvalWhiteEndgame[mFrom(move)];
                if (BB->whiteCastleQueenSide && mFrom(move) == 7) {
                    // if the rook moves from the a1 square, then the white queen side castle is no longer possible
                    BB->whiteCastleQueenSide = false; BB->hash ^= castlingRights[3];
                } else if (BB->whiteCastleKingSide && mFrom(move) == 0) {
                    BB->whiteCastleKingSide = false; BB->hash ^= castlingRights[2];
                }
                break;

            case 5:
                BB->whiteQueens ^= ((fromBit) | (toBit)); BB->hash ^= ZOBRIST_TABLE[mFrom(move)][4] ^ ZOBRIST_TABLE[mTo(move)][4];
                BB->whiteEvalOpening += queenEvalWhiteOpening[mTo(move)] - queenEvalWhiteOpening[mFrom(move)];
                BB->whiteEvalEndgame += queenEvalWhiteEndgame[mTo(move)] - queenEvalWhiteEndgame[mFrom(move)];
                break;

            case 6:
                BB->whiteKing ^= ((fromBit) | (toBit)); BB->hash ^= ZOBRIST_TABLE[mFrom(move)][5] ^ ZOBRIST_TABLE[mTo(move)][5];
                BB->whiteEvalOpening += kingEvalWhiteOpening[mTo(move)] - kingEvalWhiteOpening[mFrom(move)];
                BB->whiteEvalEndgame += kingEvalWhiteEndgame[mTo(move)] - kingEvalWhiteEndgame[mFrom(move)];
                if (mCastle(move)) {
                    // castle move
                    int s;
                    int o;
                    if (mCastle(move) == KINGSIDE) {s = 0; o = 2;} else {s = 7; o = 4;}
                        // put the rook on its new sqaure
                        BB->allPieces ^= (1ULL << s) | (1ULL << o); BB->whitePieces ^= (1ULL << s) | (1ULL << o); BB->whiteRooks ^= (1ULL << s) | (1ULL << o);

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
                break;
        }

        if (BB->enPassantSquare >= 0) {
            // if there was an en passant square, remove it from the hash
            BB->hash ^= ZOBRIST_TABLE[BB->enPassantSquare][12]; BB->enPassantSquare = -1;
        }

        if (mCrEp(move) && ((BB->blackPawns & (1ULL << (mTo(move) + 1))) || (BB->blackPawns & (1ULL << (mTo(move) - 1))))) {
            // if the move creates an en passant square and it it captureable, add it to the hash and set the bit
            BB->enPassantSquare = mTo(move) - 8; BB->hash ^= ZOBRIST_TABLE[mTo(move) - 8][12];
        }

    } else {

        undo->castleKingSide = BB->blackCastleKingSide;
        undo->castleQueenSide = BB->blackCastleQueenSide;
        // Black move
        if (mIsEp(move)) {
            BB->whitePawns ^= (1ULL << (mTo(move) + 8)); BB->whitePieces ^= (1ULL << (mTo(move) + 8)); BB->allPieces ^= (1ULL << (mTo(move) + 8));
            BB->hash ^= ZOBRIST_TABLE[mTo(move) + 8][0]; BB->pieceList[mTo(move) + 8] = 0;
            BB->whiteEvalOpening -= pawnEvalWhiteOpening[mTo(move) + 8];
            BB->whiteEvalEndgame -= pawnEvalWhiteEndgame[mTo(move) + 8];

        } else if (BB->whitePieces & (toBit)) {
            BB->whitePieces ^= (toBit);

            if (BB->whitePawns & (toBit)) {
                BB->whitePawns ^= (toBit); BB->hash ^= ZOBRIST_TABLE[mTo(move)][0];
                BB->whiteEvalOpening -= pawnEvalWhiteOpening[mTo(move)];
                BB->whiteEvalEndgame -= pawnEvalWhiteEndgame[mTo(move)];
    
            } else if (BB->whiteKnights & (toBit)) {
                BB->whiteKnights ^= (toBit); BB->hash ^= ZOBRIST_TABLE[mTo(move)][1];
                BB->whiteEvalOpening -= knightEvalWhiteOpening[mTo(move)];
                BB->whiteEvalEndgame -= knightEvalWhiteEndgame[mTo(move)];
    
            } else if (BB->whiteBishops & (toBit)) {
                BB->whiteBishops ^= (toBit); BB->hash ^= ZOBRIST_TABLE[mTo(move)][2];
                BB->whiteEvalOpening -= bishopEvalWhiteOpening[mTo(move)];
                BB->whiteEvalEndgame -= bishopEvalWhiteEndgame[mTo(move)];
    
            } else if (BB->whiteRooks & (toBit)) {
                BB->whiteRooks ^= (toBit); BB->hash ^= ZOBRIST_TABLE[mTo(move)][3];
                BB->whiteEvalOpening -= rookEvalWhiteOpening[mTo(move)];
                BB->whiteEvalEndgame -= rookEvalWhiteEndgame[mTo(move)];
    
            } else if (BB->whiteQueens & (toBit)) {
                BB->whiteQueens ^= (toBit); BB->hash ^= ZOBRIST_TABLE[mTo(move)][4];
                BB->whiteEvalOpening -= queenEvalWhiteOpening[mTo(move)];
                BB->whiteEvalEndgame -= queenEvalWhiteEndgame[mTo(move)];
    
            } else if (BB->whiteKing & (toBit)) {
                BB->whiteKing ^= (toBit); BB->hash ^= ZOBRIST_TABLE[mTo(move)][5];
                BB->whiteEvalOpening -= kingEvalWhiteOpening[mTo(move)];
                BB->whiteEvalEndgame -= kingEvalWhiteEndgame[mTo(move)];
            }
        }
        
        BB->blackPieces ^= (fromBit) | (toBit);

        switch (BB->pieceList[mTo(move)]) {
            case -1:
                BB->blackPawns ^= (fromBit); BB->hash ^= ZOBRIST_TABLE[mFrom(move)][6];
                BB->blackEvalOpening -= pawnEvalBlackOpening[mFrom(move)];
                BB->blackEvalEndgame -= pawnEvalBlackEndgame[mFrom(move)];
                if (mTo(move) <= 7) {
                    switch (mPromo(move)) {
                        case 4:
                            BB->blackQueens ^= (toBit); BB->hash ^= ZOBRIST_TABLE[mTo(move)][10]; BB->pieceList[mTo(move)] = -5;
                            BB->blackEvalOpening += queenEvalBlackOpening[mTo(move)];
                            BB->blackEvalEndgame += queenEvalBlackEndgame[mTo(move)];
                            break;
                        case 3:
                            BB->blackRooks ^= (toBit); BB->hash ^= ZOBRIST_TABLE[mTo(move)][9]; BB->pieceList[mTo(move)] = -4;
                            BB->blackEvalOpening += rookEvalBlackOpening[mTo(move)];
                            BB->blackEvalEndgame += rookEvalBlackEndgame[mTo(move)];
                            break;
                        case 2:
                            BB->blackBishops ^= (toBit); BB->hash ^= ZOBRIST_TABLE[mTo(move)][8]; BB->pieceList[mTo(move)] = -3;
                            BB->blackEvalOpening += bishopEvalBlackOpening[mTo(move)];
                            BB->blackEvalEndgame += bishopEvalBlackEndgame[mTo(move)];
                            break;
                        case 1:
                            BB->blackKnights ^= (toBit); BB->hash ^= ZOBRIST_TABLE[mTo(move)][7]; BB->pieceList[mTo(move)] = -2;
                            BB->blackEvalOpening += knightEvalBlackOpening[mTo(move)];
                            BB->blackEvalEndgame += knightEvalBlackEndgame[mTo(move)];
                            break;
                    }
                } else {
                    BB->blackPawns ^= (toBit); BB->hash ^= ZOBRIST_TABLE[mTo(move)][6];
                    BB->blackEvalOpening += pawnEvalBlackOpening[mTo(move)];
                    BB->blackEvalEndgame += pawnEvalBlackEndgame[mTo(move)];
                }
                break;

            case -2:
                BB->blackKnights ^= ((toBit) | (fromBit)); BB->hash ^= ZOBRIST_TABLE[mTo(move)][7] ^ ZOBRIST_TABLE[mFrom(move)][7];
                BB->blackEvalOpening += knightEvalBlackOpening[mTo(move)] - knightEvalBlackOpening[mFrom(move)];
                BB->blackEvalEndgame += knightEvalBlackEndgame[mTo(move)] - knightEvalBlackEndgame[mFrom(move)];
                break;

            case -3:
                BB->blackBishops ^= ((toBit) | (fromBit)); BB->hash ^= ZOBRIST_TABLE[mTo(move)][8] ^ ZOBRIST_TABLE[mFrom(move)][8];
                BB->blackEvalOpening += bishopEvalBlackOpening[mTo(move)] - bishopEvalBlackOpening[mFrom(move)];
                BB->blackEvalEndgame += bishopEvalBlackEndgame[mTo(move)] - bishopEvalBlackEndgame[mFrom(move)];
                break;

            case -4:
                BB->blackRooks ^= ((toBit) | (fromBit)); BB->hash ^= ZOBRIST_TABLE[mTo(move)][9] ^ ZOBRIST_TABLE[mFrom(move)][9];
                BB->blackEvalOpening += rookEvalBlackOpening[mTo(move)] - rookEvalBlackOpening[mFrom(move)];
                BB->blackEvalEndgame += rookEvalBlackEndgame[mTo(move)] - rookEvalBlackEndgame[mFrom(move)];
                if (BB->blackCastleQueenSide && mFrom(move) == 63) {
                    BB->blackCastleQueenSide = false; BB->hash ^= castlingRights[1];
                } else if (BB->blackCastleKingSide && mFrom(move) == 56) {
                    BB->blackCastleKingSide = false; BB->hash ^= castlingRights[0];
                }
                break;

            case -5:
                BB->blackQueens ^= ((toBit) | (fromBit)); BB->hash ^= ZOBRIST_TABLE[mTo(move)][10] ^ ZOBRIST_TABLE[mFrom(move)][10];
                BB->blackEvalOpening += queenEvalBlackOpening[mTo(move)] - queenEvalBlackOpening[mFrom(move)];
                BB->blackEvalEndgame += queenEvalBlackEndgame[mTo(move)] - queenEvalBlackEndgame[mFrom(move)];
                break;

            case -6:
                BB->blackKing ^= ((toBit) | (fromBit)); BB->hash ^= ZOBRIST_TABLE[mTo(move)][11] ^ ZOBRIST_TABLE[mFrom(move)][11];
                BB->blackEvalOpening += kingEvalBlackOpening[mTo(move)] - kingEvalBlackOpening[mFrom(move)];
                BB->blackEvalEndgame += kingEvalBlackEndgame[mTo(move)] - kingEvalBlackEndgame[mFrom(move)];
                if (mCastle(move)) {
                    int s;
                    int o;
                    if (mCastle(move) == KINGSIDE) {s = 56; o = 58;} else {s = 63; o = 60;}

                        BB->allPieces ^= (1ULL << s) | (1ULL << o); BB->blackPieces ^= (1ULL << s) | (1ULL << o); BB->blackRooks ^= (1ULL << s) | (1ULL << o);
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
                break;
        }

        if (BB->enPassantSquare >= 0) {
            BB->hash ^= ZOBRIST_TABLE[BB->enPassantSquare][12]; BB->enPassantSquare = -1;
        }

        if (mCrEp(move) && ((BB->whitePawns & (1ULL << (mTo(move) + 1))) || (BB->whitePawns & (1ULL << (mTo(move) - 1))))) {
            BB->enPassantSquare = (mTo(move) + 8); BB->hash ^= ZOBRIST_TABLE[mTo(move) + 8][12];
        }
    }
    BB->color = !BB->color;
}

void undoMove(move_t move, struct bitboards_t *BB, struct undo_t *undo) {

    searchMovesPlayed--;

    u64 toBit = 1ULL << mTo(move);
    u64 fromBit = 1ULL << mFrom(move);

    BB->allPieces ^= ((fromBit) | (toBit));
    BB->hash ^= whiteToMove;
    BB->color = !BB->color;

    BB->pieceList[mFrom(move)] = BB->pieceList[mTo(move)];
    BB->pieceList[mTo(move)] = undo->capturedPiece;

    if (BB->color) {
        if (mPromo(move)) {
            BB->pieceList[mFrom(move)] = 1;
        }

        if (mIsEp(move)) {
            BB->blackPawns ^= (1ULL << (mTo(move) - 8)); BB->blackPieces ^= (1ULL << (mTo(move) - 8)); BB->allPieces ^= (1ULL << (mTo(move) - 8));

            BB->hash ^= ZOBRIST_TABLE[(mTo(move) - 8)][6]; BB->pieceList[mTo(move) - 8] = -1;
            BB->blackEvalOpening += pawnEvalBlackOpening[mTo(move) - 8];
            BB->blackEvalEndgame += pawnEvalBlackEndgame[mTo(move) - 8];

        } else if (undo->capturedPiece) {
            BB->blackPieces ^= toBit; BB->allPieces ^= toBit;

            switch (undo->capturedPiece) {
                case -1:
                    BB->blackPawns ^= (toBit); BB->hash ^= ZOBRIST_TABLE[mTo(move)][6];
                    BB->blackEvalOpening += pawnEvalBlackOpening[mTo(move)];
                    BB->blackEvalEndgame += pawnEvalBlackEndgame[mTo(move)];
                    break;
                case -2:
                    BB->blackKnights ^= (toBit); BB->hash ^= ZOBRIST_TABLE[mTo(move)][7];
                    BB->blackEvalOpening += knightEvalBlackOpening[mTo(move)];
                    BB->blackEvalEndgame += knightEvalBlackEndgame[mTo(move)];
                    break;
                case -3:
                    BB->blackBishops ^= (toBit); BB->hash ^= ZOBRIST_TABLE[mTo(move)][8];
                    BB->blackEvalOpening += bishopEvalBlackOpening[mTo(move)];
                    BB->blackEvalEndgame += bishopEvalBlackEndgame[mTo(move)];
                    break;
                case -4:
                    BB->blackRooks ^= (toBit); BB->hash ^= ZOBRIST_TABLE[mTo(move)][9];
                    BB->blackEvalOpening += rookEvalBlackOpening[mTo(move)];
                    BB->blackEvalEndgame += rookEvalBlackEndgame[mTo(move)];
                    break;
                case -5:
                    BB->blackQueens ^= (toBit); BB->hash ^= ZOBRIST_TABLE[mTo(move)][10];
                    BB->blackEvalOpening += queenEvalBlackOpening[mTo(move)];
                    BB->blackEvalEndgame += queenEvalBlackEndgame[mTo(move)];
                    break;
                case -6:
                    BB->blackKing ^= (toBit); BB->hash ^= ZOBRIST_TABLE[mTo(move)][11];
                    BB->blackEvalOpening += kingEvalBlackOpening[mTo(move)];
                    BB->blackEvalEndgame += kingEvalBlackEndgame[mTo(move)];
                    break;
            }
        }
        
        BB->whitePieces ^= ((fromBit) | (toBit));

        switch (BB->pieceList[mFrom(move)]) {
            case 1:
                BB->whitePawns ^= (fromBit); BB->hash ^= ZOBRIST_TABLE[mFrom(move)][0];
                BB->whiteEvalOpening += pawnEvalWhiteOpening[mFrom(move)];
                BB->whiteEvalEndgame += pawnEvalWhiteEndgame[mFrom(move)];
                if (mTo(move) >= 56) {
                    switch (mPromo(move)) {
                        case 4:
                            BB->whiteQueens ^= (toBit); BB->hash ^= ZOBRIST_TABLE[mTo(move)][4];
                            BB->whiteEvalOpening -= queenEvalWhiteOpening[mTo(move)];
                            BB->whiteEvalEndgame -= queenEvalWhiteEndgame[mTo(move)];
                            break;
                        case 3:
                            BB->whiteRooks ^= (toBit); BB->hash ^= ZOBRIST_TABLE[mTo(move)][3];
                            BB->whiteEvalOpening -= rookEvalWhiteOpening[mTo(move)];
                            BB->whiteEvalEndgame -= rookEvalWhiteEndgame[mTo(move)];
                            break;
                        case 2:
                            BB->whiteBishops ^= (toBit); BB->hash ^= ZOBRIST_TABLE[mTo(move)][2];
                            BB->whiteEvalOpening -= bishopEvalWhiteOpening[mTo(move)];
                            BB->whiteEvalEndgame -= bishopEvalWhiteEndgame[mTo(move)];
                            break;
                        case 1:
                            BB->whiteKnights ^= (toBit); BB->hash ^= ZOBRIST_TABLE[mTo(move)][1];
                            BB->whiteEvalOpening -= knightEvalWhiteOpening[mTo(move)];
                            BB->whiteEvalEndgame -= knightEvalWhiteEndgame[mTo(move)];
                            break;
                    }
                } else {
                    BB->whitePawns ^= (toBit); BB->hash ^= ZOBRIST_TABLE[mTo(move)][0];
                    BB->whiteEvalOpening -= pawnEvalWhiteOpening[mTo(move)];
                    BB->whiteEvalEndgame -= pawnEvalWhiteEndgame[mTo(move)];
                }
                break;

            case 2:
                BB->whiteKnights ^= ((fromBit) | (toBit)); BB->hash ^= ZOBRIST_TABLE[mFrom(move)][1] ^ ZOBRIST_TABLE[mTo(move)][1];
                BB->whiteEvalOpening -= knightEvalWhiteOpening[mTo(move)] - knightEvalWhiteOpening[mFrom(move)];
                BB->whiteEvalEndgame -= knightEvalWhiteEndgame[mTo(move)] - knightEvalWhiteEndgame[mFrom(move)];
                break;

            case 3:
                BB->whiteBishops ^= ((fromBit) | (toBit)); BB->hash ^= ZOBRIST_TABLE[mFrom(move)][2] ^ ZOBRIST_TABLE[mTo(move)][2];
                BB->whiteEvalOpening -= bishopEvalWhiteOpening[mTo(move)] - bishopEvalWhiteOpening[mFrom(move)];
                BB->whiteEvalEndgame -= bishopEvalWhiteEndgame[mTo(move)] - bishopEvalWhiteEndgame[mFrom(move)];
                break;

            case 4:
                BB->whiteRooks ^= ((fromBit) | (toBit)); BB->hash ^= ZOBRIST_TABLE[mFrom(move)][3] ^ ZOBRIST_TABLE[mTo(move)][3];
                BB->whiteEvalOpening -= rookEvalWhiteOpening[mTo(move)] - rookEvalWhiteOpening[mFrom(move)];
                BB->whiteEvalEndgame -= rookEvalWhiteEndgame[mTo(move)] - rookEvalWhiteEndgame[mFrom(move)];
                break;

            case 5:
                BB->whiteQueens ^= ((fromBit) | (toBit)); BB->hash ^= ZOBRIST_TABLE[mFrom(move)][4] ^ ZOBRIST_TABLE[mTo(move)][4];
                BB->whiteEvalOpening -= queenEvalWhiteOpening[mTo(move)] - queenEvalWhiteOpening[mFrom(move)];
                BB->whiteEvalEndgame -= queenEvalWhiteEndgame[mTo(move)] - queenEvalWhiteEndgame[mFrom(move)];
                break;

            case 6:
                BB->whiteKing ^= ((fromBit) | (toBit)); BB->hash ^= ZOBRIST_TABLE[mFrom(move)][5] ^ ZOBRIST_TABLE[mTo(move)][5];
                BB->whiteEvalOpening -= kingEvalWhiteOpening[mTo(move)] - kingEvalWhiteOpening[mFrom(move)];
                BB->whiteEvalEndgame -= kingEvalWhiteEndgame[mTo(move)] - kingEvalWhiteEndgame[mFrom(move)];
                if (mCastle(move)) {
                    int s;
                    int o;
                    if (mCastle(move) == KINGSIDE) {s = 0; o = 2;} else {s = 7; o = 4;}
                        // put the rook on its new sqaure
                        BB->allPieces ^= (1ULL << s) | (1ULL << o); BB->whitePieces ^= (1ULL << s) | (1ULL << o); BB->whiteRooks ^= (1ULL << s) | (1ULL << o);

                        BB->hash ^= ZOBRIST_TABLE[o][3] ^ ZOBRIST_TABLE[s][3]; BB->pieceList[s] = 4; BB->pieceList[o] = 0;
                        BB->whiteEvalOpening -= rookEvalWhiteOpening[o] - rookEvalWhiteOpening[s];
                        BB->whiteEvalEndgame -= rookEvalWhiteEndgame[o] - rookEvalWhiteEndgame[s];
                }
                break;
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
            BB->whitePawns ^= (1ULL << (mTo(move) + 8)); BB->whitePieces ^= (1ULL << (mTo(move) + 8)); BB->allPieces ^= (1ULL << (mTo(move) + 8));
            BB->hash ^= ZOBRIST_TABLE[mTo(move) + 8][0]; BB->pieceList[mTo(move) + 8] = 1;
            BB->whiteEvalOpening += pawnEvalWhiteOpening[mTo(move) + 8];
            BB->whiteEvalEndgame += pawnEvalWhiteEndgame[mTo(move) + 8];

        } else if (undo->capturedPiece) {
            BB->whitePieces ^= toBit; BB->allPieces ^= toBit;

            switch (undo->capturedPiece) {
                case 1:
                    BB->whitePawns ^= toBit; BB->hash ^= ZOBRIST_TABLE[mTo(move)][0];
                    BB->whiteEvalOpening += pawnEvalWhiteOpening[mTo(move)];
                    BB->whiteEvalEndgame += pawnEvalWhiteEndgame[mTo(move)];
                    break;
                case 2:
                    BB->whiteKnights ^= toBit; BB->hash ^= ZOBRIST_TABLE[mTo(move)][1];
                    BB->whiteEvalOpening += knightEvalWhiteOpening[mTo(move)];
                    BB->whiteEvalEndgame += knightEvalWhiteEndgame[mTo(move)];
                    break;
                case 3:
                    BB->whiteBishops ^= toBit; BB->hash ^= ZOBRIST_TABLE[mTo(move)][2];
                    BB->whiteEvalOpening += bishopEvalWhiteOpening[mTo(move)];
                    BB->whiteEvalEndgame += bishopEvalWhiteEndgame[mTo(move)];
                    break;
                case 4:
                    BB->whiteRooks ^= toBit; BB->hash ^= ZOBRIST_TABLE[mTo(move)][3];
                    BB->whiteEvalOpening += rookEvalWhiteOpening[mTo(move)];
                    BB->whiteEvalEndgame += rookEvalWhiteEndgame[mTo(move)];
                    break;
                case 5:
                    BB->whiteQueens ^= toBit; BB->hash ^= ZOBRIST_TABLE[mTo(move)][4];
                    BB->whiteEvalOpening += queenEvalWhiteOpening[mTo(move)];
                    BB->whiteEvalEndgame += queenEvalWhiteEndgame[mTo(move)];
                    break;
                case 6:
                    BB->whiteKing ^= toBit; BB->hash ^= ZOBRIST_TABLE[mTo(move)][5];
                    BB->whiteEvalOpening += kingEvalWhiteOpening[mTo(move)];
                    BB->whiteEvalEndgame += kingEvalWhiteEndgame[mTo(move)];
                    break;
            }
        }
        
        BB->blackPieces ^= (fromBit) | (toBit);

        switch (BB->pieceList[mFrom(move)]) {
            case -1:
                BB->blackPawns ^= (fromBit); BB->hash ^= ZOBRIST_TABLE[mFrom(move)][6];
                BB->blackEvalOpening += pawnEvalBlackOpening[mFrom(move)];
                BB->blackEvalEndgame += pawnEvalBlackEndgame[mFrom(move)];
                if (mTo(move) <= 7) {
                    if (mPromo(move) == 4) {
                        BB->blackQueens ^= (toBit); BB->hash ^= ZOBRIST_TABLE[mTo(move)][10];
                        BB->blackEvalOpening -= queenEvalBlackOpening[mTo(move)];
                        BB->blackEvalEndgame -= queenEvalBlackEndgame[mTo(move)];
                    } else if (mPromo(move) == 3) {
                        BB->blackRooks ^= (toBit); BB->hash ^= ZOBRIST_TABLE[mTo(move)][9];
                        BB->blackEvalOpening -= rookEvalBlackOpening[mTo(move)];
                        BB->blackEvalEndgame -= rookEvalBlackEndgame[mTo(move)];
                    } else if (mPromo(move) == 2) {
                        BB->blackBishops ^= (toBit); BB->hash ^= ZOBRIST_TABLE[mTo(move)][8];
                        BB->blackEvalOpening -= bishopEvalBlackOpening[mTo(move)];
                        BB->blackEvalEndgame -= bishopEvalBlackEndgame[mTo(move)];
                    } else if (mPromo(move) == 1) {
                        BB->blackKnights ^= (toBit); BB->hash ^= ZOBRIST_TABLE[mTo(move)][7];
                        BB->blackEvalOpening -= knightEvalBlackOpening[mTo(move)];
                        BB->blackEvalEndgame -= knightEvalBlackEndgame[mTo(move)];
                    }
                } else {
                    BB->blackPawns ^= (toBit); BB->hash ^= ZOBRIST_TABLE[mTo(move)][6];
                    BB->blackEvalOpening -= pawnEvalBlackOpening[mTo(move)];
                    BB->blackEvalEndgame -= pawnEvalBlackEndgame[mTo(move)];
                }
                break;
            
            case -2:
                BB->blackKnights ^= ((toBit) | (fromBit)); BB->hash ^= ZOBRIST_TABLE[mTo(move)][7] ^ ZOBRIST_TABLE[mFrom(move)][7];
                BB->blackEvalOpening -= knightEvalBlackOpening[mTo(move)] - knightEvalBlackOpening[mFrom(move)];
                BB->blackEvalEndgame -= knightEvalBlackEndgame[mTo(move)] - knightEvalBlackEndgame[mFrom(move)];
                break;

            case -3:
                BB->blackBishops ^= ((toBit) | (fromBit)); BB->hash ^= ZOBRIST_TABLE[mTo(move)][8] ^ ZOBRIST_TABLE[mFrom(move)][8];
                BB->blackEvalOpening -= bishopEvalBlackOpening[mTo(move)] - bishopEvalBlackOpening[mFrom(move)];
                BB->blackEvalEndgame -= bishopEvalBlackEndgame[mTo(move)] - bishopEvalBlackEndgame[mFrom(move)];
                break;

            case -4:
                BB->blackRooks ^= ((toBit) | (fromBit)); BB->hash ^= ZOBRIST_TABLE[mTo(move)][9] ^ ZOBRIST_TABLE[mFrom(move)][9];
                BB->blackEvalOpening -= rookEvalBlackOpening[mTo(move)] - rookEvalBlackOpening[mFrom(move)];
                BB->blackEvalEndgame -= rookEvalBlackEndgame[mTo(move)] - rookEvalBlackEndgame[mFrom(move)];
                break;

            case -5:
                BB->blackQueens ^= ((toBit) | (fromBit)); BB->hash ^= ZOBRIST_TABLE[mTo(move)][10] ^ ZOBRIST_TABLE[mFrom(move)][10];
                BB->blackEvalOpening -= queenEvalBlackOpening[mTo(move)] - queenEvalBlackOpening[mFrom(move)];
                BB->blackEvalEndgame -= queenEvalBlackEndgame[mTo(move)] - queenEvalBlackEndgame[mFrom(move)];
                break;

            case -6:
                BB->blackKing ^= ((toBit) | (fromBit)); BB->hash ^= ZOBRIST_TABLE[mTo(move)][11] ^ ZOBRIST_TABLE[mFrom(move)][11];
                BB->blackEvalOpening -= kingEvalBlackOpening[mTo(move)] - kingEvalBlackOpening[mFrom(move)];
                BB->blackEvalEndgame -= kingEvalBlackEndgame[mTo(move)] - kingEvalBlackEndgame[mFrom(move)];
                if (mCastle(move)) {
                    int s;
                    int o;
                    if (mCastle(move) == KINGSIDE) {s = 56; o = 58;} else {s = 63; o = 60;}

                    BB->allPieces ^= (1ULL << s) | (1ULL << o); BB->blackPieces ^= (1ULL << s) | (1ULL << o); BB->blackRooks ^= (1ULL << s) | (1ULL << o);
                    BB->hash ^= ZOBRIST_TABLE[o][9] ^ ZOBRIST_TABLE[s][9]; BB->pieceList[s] = -4; BB->pieceList[o] = 0;
                    BB->blackEvalOpening -= rookEvalBlackOpening[o] - rookEvalBlackOpening[s];
                    BB->blackEvalEndgame -= rookEvalBlackEndgame[o] - rookEvalBlackEndgame[s];
                }
                break;
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

    if (((BB->whitePawns & bit(from)) != 0) || ((BB->blackPawns & bit(from)) != 0)) {
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
    } else if (((BB->whiteKing & bit(from)) != 0) || ((BB->blackKing & bit(from)) != 0)) {
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

        } else if (BB->blackPieces & (toBit)) {

            if (BB->blackPawns & (toBit)) {
                hash ^= ZOBRIST_TABLE[toSquare][6];
    
            } else if (BB->blackKnights & (toBit)) {
                hash ^= ZOBRIST_TABLE[toSquare][7];
    
            } else if (BB->blackBishops & (toBit)) {
                hash ^= ZOBRIST_TABLE[toSquare][8];

            } else if (BB->blackRooks & (toBit)) {
                hash ^= ZOBRIST_TABLE[toSquare][9];

            } else if (BB->blackQueens & (toBit)) {
                hash ^= ZOBRIST_TABLE[toSquare][10];
    
            } else if (BB->blackKing & (toBit)) {
                hash ^= ZOBRIST_TABLE[toSquare][11];
            }
        }

        switch (BB->pieceList[fromSquare]) {
            case 1:
                hash ^= ZOBRIST_TABLE[fromSquare][0];
                if (toSquare >= 56) {
                    // pawn promotion
                    switch (mPromo(move)) {
                        case 4:
                            hash ^= ZOBRIST_TABLE[toSquare][4];
                            break;
                        case 3:
                            hash ^= ZOBRIST_TABLE[toSquare][3];
                            break;
                        case 2:
                            hash ^= ZOBRIST_TABLE[toSquare][2];
                            break;
                        case 1:
                            hash ^= ZOBRIST_TABLE[toSquare][1];
                            break;
                    }
                } else {
                    hash ^= ZOBRIST_TABLE[toSquare][0];
                }
                break;

            case 2:
                hash ^= ZOBRIST_TABLE[fromSquare][1] ^ ZOBRIST_TABLE[toSquare][1];
                break;

            case 3:
                hash ^= ZOBRIST_TABLE[fromSquare][2] ^ ZOBRIST_TABLE[toSquare][2];
                break;

            case 4:
                hash ^= ZOBRIST_TABLE[fromSquare][3] ^ ZOBRIST_TABLE[toSquare][3];
                if (BB->whiteCastleQueenSide && fromSquare == 7) {
                    hash ^= castlingRights[3];
                } else if (BB->whiteCastleKingSide && fromSquare == 0) {
                    hash ^= castlingRights[2];
                }
                break;

            case 5:
                hash ^= ZOBRIST_TABLE[fromSquare][4] ^ ZOBRIST_TABLE[toSquare][4];
                break;

            case 6:
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
                break;
        }

        if (BB->enPassantSquare >= 0) {
            hash ^= ZOBRIST_TABLE[BB->enPassantSquare][12];
        }

        if (mCrEp(move) && ((BB->blackPawns & (1ULL << (toSquare + 1))) || (BB->blackPawns & (1ULL << (toSquare - 1))))) {
            hash ^= ZOBRIST_TABLE[toSquare - 8][12];
        }

    } else {
        // Black move
        if (mIsEp(move)) {
            hash ^= ZOBRIST_TABLE[toSquare + 8][0];

        } else if (BB->whitePieces & (toBit)) {

            if (BB->whitePawns & (toBit)) {
                hash ^= ZOBRIST_TABLE[toSquare][0];
    
            } else if (BB->whiteKnights & (toBit)) {
                hash ^= ZOBRIST_TABLE[toSquare][1];

            } else if (BB->whiteBishops & (toBit)) {
                hash ^= ZOBRIST_TABLE[toSquare][2];

            } else if (BB->whiteRooks & (toBit)) {
                hash ^= ZOBRIST_TABLE[toSquare][3];

            } else if (BB->whiteQueens & (toBit)) {
                hash ^= ZOBRIST_TABLE[toSquare][4];

            } else if (BB->whiteKing & (toBit)) {
                hash ^= ZOBRIST_TABLE[toSquare][5];
            }
        }

        switch (BB->pieceList[fromSquare]) {
            case -1:
                hash ^= ZOBRIST_TABLE[fromSquare][6];
                if (toSquare <= 7) {
                    switch (mPromo(move)) {
                        case 4:
                            BB->blackQueens ^= (toBit); hash ^= ZOBRIST_TABLE[toSquare][10];;
                            break;
                        case 3:
                            BB->blackRooks ^= (toBit); hash ^= ZOBRIST_TABLE[toSquare][9];;
                            break;
                        case 2:
                            BB->blackBishops ^= (toBit); hash ^= ZOBRIST_TABLE[toSquare][8];;
                            break;
                        case 1:
                            BB->blackKnights ^= (toBit); hash ^= ZOBRIST_TABLE[toSquare][7];;
                            break;
                    }
                } else {
                    hash ^= ZOBRIST_TABLE[toSquare][6];
                }
                break;

            case -2:
                hash ^= ZOBRIST_TABLE[toSquare][7] ^ ZOBRIST_TABLE[fromSquare][7];
                break;

            case -3:
                hash ^= ZOBRIST_TABLE[toSquare][8] ^ ZOBRIST_TABLE[fromSquare][8];
                break;

            case -4:
                hash ^= ZOBRIST_TABLE[toSquare][9] ^ ZOBRIST_TABLE[fromSquare][9];
                if (BB->blackCastleQueenSide && fromSquare == 63) {
                    hash ^= castlingRights[1];
                } else if (BB->blackCastleKingSide && fromSquare == 56) {
                    hash ^= castlingRights[0];
                }
                break;

            case -5:
                hash ^= ZOBRIST_TABLE[toSquare][10] ^ ZOBRIST_TABLE[fromSquare][10];
                break;

            case -6:
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
                break;
        }

        if (BB->enPassantSquare >= 0) {
            hash ^= ZOBRIST_TABLE[BB->enPassantSquare][12]; BB->enPassantSquare = -1;
        }

        if (mCrEp(move) && ((BB->whitePawns & (1ULL << (toSquare + 1))) || (BB->whitePawns & (1ULL << (toSquare - 1))))) {
            BB->enPassantSquare = (toSquare + 8); hash ^= ZOBRIST_TABLE[toSquare + 8][12];
        }
    }
    
    return hash;
}