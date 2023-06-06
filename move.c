#include "move.h"

void doMove(struct move_t *move, struct bitboards_t *BB, struct undo_t *undo) {

    moveCalls++;

    undo->capturedPiece = BB->pieceList[move->to];
    undo->enPassantSquare = BB->enPassantSquare;

    u64 toBit = 1ULL << move->to;
    u64 fromBit = 1ULL << move->from;
    // flip the from bit and only flip the to bit if its not a capture
    BB->allPieces ^= ((fromBit) | ((toBit) & ~BB->allPieces));
    // update the hash
    BB->hash ^= whiteToMove;
    // update the piece list
    BB->pieceList[move->from] = 0;
    BB->pieceList[move->to] = (int) ((move->pieceType + 1) * (BB->color ? 1 : -1));
    // now for the specific piece bitboards
    if (BB->color) {
        undo->castleKingSide = BB->whiteCastleKingSide; 
        undo->castleQueenSide = BB->whiteCastleQueenSide;

        if (move->isEnPassantCapture) {
            // remove the captured pawn
            BB->blackPawns ^= (1ULL << (move->to - 8)); BB->blackPieces ^= (1ULL << (move->to - 8)); BB->allPieces ^= (1ULL << (move->to - 8));

            BB->hash ^= ZOBRIST_TABLE[(move->to - 8)][6];
            BB->pieceList[move->to - 8] = 0;
            // also update the eval
            BB->blackEval -= pawnEvalBlack[move->to - 8];

        } else if (BB->blackPieces & (toBit)) {
            // capture move
            // here we dont need to remove the bit from the occupied squares, because the capturing piece is already there
            BB->blackPieces ^= (toBit);

            if (BB->blackPawns & (toBit)) {
                BB->blackPawns ^= (toBit); BB->hash ^= ZOBRIST_TABLE[move->to][6]; BB->blackEval -= pawnEvalBlack[move->to];
    
            } else if (BB->blackKnights & (toBit)) {
                BB->blackKnights ^= (toBit); BB->hash ^= ZOBRIST_TABLE[move->to][7]; BB->blackEval -= knightEvalBlack[move->to];
    
            } else if (BB->blackBishops & (toBit)) {
                BB->blackBishops ^= (toBit); BB->hash ^= ZOBRIST_TABLE[move->to][8]; BB->blackEval -= bishopEvalBlack[move->to];
    
            } else if (BB->blackRooks & (toBit)) {
                BB->blackRooks ^= (toBit); BB->hash ^= ZOBRIST_TABLE[move->to][9]; BB->blackEval -= rookEvalBlack[move->to];

            } else if (BB->blackQueens & (toBit)) {
                BB->blackQueens ^= (toBit); BB->hash ^= ZOBRIST_TABLE[move->to][10]; BB->blackEval -= queenEvalBlack[move->to];
    
            } else if (BB->blackKing & (toBit)) {
                BB->blackKing ^= (toBit); BB->hash ^= ZOBRIST_TABLE[move->to][11]; BB->blackEval -= kingEvalBlack[move->to];
                // in theory this should never happen because the search will stop if one player has no moves
            }
        }
        
        BB->whitePieces ^= ((fromBit) | (toBit));

        if (move->pieceType == 0) { // pawn move
            BB->whitePawns ^= (fromBit); BB->hash ^= ZOBRIST_TABLE[move->from][0]; BB->whiteEval -= pawnEvalWhite[move->from];
            if (move->to >= 56) {
                // pawn promotion
                if (move->promotesTo == 4) {
                    BB->whiteQueens ^= (toBit); BB->hash ^= ZOBRIST_TABLE[move->to][4]; BB->pieceList[move->to] = 5; BB->whiteEval += queenEvalWhite[move->to];
                } else if (move->promotesTo == 3) {
                    BB->whiteRooks ^= (toBit); BB->hash ^= ZOBRIST_TABLE[move->to][3]; BB->pieceList[move->to] = 4; BB->whiteEval += rookEvalWhite[move->to];
                } else if (move->promotesTo == 2) {
                    BB->whiteBishops ^= (toBit); BB->hash ^= ZOBRIST_TABLE[move->to][2]; BB->pieceList[move->to] = 3; BB->whiteEval += bishopEvalWhite[move->to];
                } else if (move->promotesTo == 1) {
                    BB->whiteKnights ^= (toBit); BB->hash ^= ZOBRIST_TABLE[move->to][1]; BB->pieceList[move->to] = 2; BB->whiteEval += knightEvalWhite[move->to];
                }
            } else {
                BB->whitePawns ^= (toBit); BB->hash ^= ZOBRIST_TABLE[move->to][0]; BB->whiteEval += pawnEvalWhite[move->to];
            }
        }
        else if (move->pieceType == 1) {// knight move
            BB->whiteKnights ^= ((fromBit) | (toBit)); BB->hash ^= ZOBRIST_TABLE[move->from][1] ^ ZOBRIST_TABLE[move->to][1]; BB->whiteEval += knightEvalWhite[move->to] - knightEvalWhite[move->from];

        } else if (move->pieceType == 2) { // bishop move
            BB->whiteBishops ^= ((fromBit) | (toBit)); BB->hash ^= ZOBRIST_TABLE[move->from][2] ^ ZOBRIST_TABLE[move->to][2]; BB->whiteEval += bishopEvalWhite[move->to] - bishopEvalWhite[move->from];

        } else if (move->pieceType == 3) { // rook move
            BB->whiteRooks ^= ((fromBit) | (toBit)); BB->hash ^= ZOBRIST_TABLE[move->from][3] ^ ZOBRIST_TABLE[move->to][3]; BB->whiteEval += rookEvalWhite[move->to] - rookEvalWhite[move->from];
            if (BB->whiteCastleQueenSide && move->from == 7) {
                // if the rook moves from the a1 square, then the white queen side castle is no longer possible
                BB->whiteCastleQueenSide = false; BB->hash ^= castlingRights[3];
            } else if (BB->whiteCastleKingSide && move->from == 0) {
                BB->whiteCastleKingSide = false; BB->hash ^= castlingRights[2];
            }
        } else if (move->pieceType == 4) { // queen move
            BB->whiteQueens ^= ((fromBit) | (toBit)); BB->hash ^= ZOBRIST_TABLE[move->from][4] ^ ZOBRIST_TABLE[move->to][4]; BB->whiteEval += queenEvalWhite[move->to] - queenEvalWhite[move->from];

        } else if (move->pieceType == 5) { // king move
            BB->whiteKing ^= ((fromBit) | (toBit)); BB->hash ^= ZOBRIST_TABLE[move->from][5] ^ ZOBRIST_TABLE[move->to][5];
            if (move->castle) {
                // castle move
                int s;
                int o;
                if (move->castle == KINGSIDE) {s = 0; o = 2;} else {s = 7; o = 4;}
                    // put the rook on its new sqaure
                    BB->allPieces ^= (1ULL << s) | (1ULL << o); BB->whitePieces ^= (1ULL << s) | (1ULL << o); BB->whiteRooks ^= (1ULL << s) | (1ULL << o);

                    BB->hash ^= ZOBRIST_TABLE[o][3] ^ ZOBRIST_TABLE[s][3]; BB->pieceList[s] = 0; BB->pieceList[o] = 4; BB->whiteEval += rookEvalWhite[o] - rookEvalWhite[s];
                }
            if (BB->whiteCastleQueenSide) {
                BB->whiteCastleQueenSide = false; BB->hash ^= castlingRights[3];
            }
            if (BB->whiteCastleKingSide) {
                BB->whiteCastleKingSide = false; BB->hash ^= castlingRights[2];
            }
        }
        
        if (BB->enPassantSquare) {
            // if there was an en passant square, remove it from the hash
            BB->hash ^= ZOBRIST_TABLE[lsb(BB->enPassantSquare)][12]; BB->enPassantSquare = 0;
        }

        if (move->createsEnPassant && ((BB->blackPawns & (1ULL << (move->to + 1))) || (BB->blackPawns & (1ULL << (move->to - 1))))) {
            // if the move creates an en passant square and it it captureable, add it to the hash and set the bit
            BB->enPassantSquare ^= 1ULL << (move->to - 8); BB->hash ^= ZOBRIST_TABLE[move->to - 8][12];
        }

    } else {

        undo->castleKingSide = BB->blackCastleKingSide;
        undo->castleQueenSide = BB->blackCastleQueenSide;
        // Black move
        if (move->isEnPassantCapture) {
            BB->whitePawns ^= (1ULL << (move->to + 8)); BB->whitePieces ^= (1ULL << (move->to + 8)); BB->allPieces ^= (1ULL << (move->to + 8));
            BB->hash ^= ZOBRIST_TABLE[move->to + 8][0]; BB->pieceList[move->to + 8] = 0; BB->whiteEval -= pawnEvalWhite[move->to + 8];

        } else if (BB->whitePieces & (toBit)) {
            BB->whitePieces ^= (toBit);

            if (BB->whitePawns & (toBit)) {
                BB->whitePawns ^= (toBit); BB->hash ^= ZOBRIST_TABLE[move->to][0]; BB->whiteEval -= pawnEvalWhite[move->to];
    
            } else if (BB->whiteKnights & (toBit)) {
                BB->whiteKnights ^= (toBit); BB->hash ^= ZOBRIST_TABLE[move->to][1]; BB->whiteEval -= knightEvalWhite[move->to];
    
            } else if (BB->whiteBishops & (toBit)) {
                BB->whiteBishops ^= (toBit); BB->hash ^= ZOBRIST_TABLE[move->to][2]; BB->whiteEval -= bishopEvalWhite[move->to];
    
            } else if (BB->whiteRooks & (toBit)) {
                BB->whiteRooks ^= (toBit); BB->hash ^= ZOBRIST_TABLE[move->to][3]; BB->whiteEval -= rookEvalWhite[move->to];
    
            } else if (BB->whiteQueens & (toBit)) {
                BB->whiteQueens ^= (toBit); BB->hash ^= ZOBRIST_TABLE[move->to][4]; BB->whiteEval -= queenEvalWhite[move->to];
    
            } else if (BB->whiteKing & (toBit)) {
                BB->whiteKing ^= (toBit); BB->hash ^= ZOBRIST_TABLE[move->to][5]; BB->whiteEval -= kingEvalWhite[move->to];
            }
        }
        
        BB->blackPieces ^= (fromBit) | (toBit);

        if (move->pieceType == 0) {
            BB->blackPawns ^= (fromBit); BB->hash ^= ZOBRIST_TABLE[move->from][6]; BB->blackEval -= pawnEvalBlack[move->from];
            if (move->to <= 7) {
                if (move->promotesTo == 4) {
                    BB->blackQueens ^= (toBit); BB->hash ^= ZOBRIST_TABLE[move->to][10]; BB->pieceList[move->to] = -5; BB->blackEval += queenEvalBlack[move->to];
                } else if (move->promotesTo == 3) {
                    BB->blackRooks ^= (toBit); BB->hash ^= ZOBRIST_TABLE[move->to][9]; BB->pieceList[move->to] = -4; BB->blackEval += rookEvalBlack[move->to];
                } else if (move->promotesTo == 2) {
                    BB->blackBishops ^= (toBit); BB->hash ^= ZOBRIST_TABLE[move->to][8]; BB->pieceList[move->to] = -3; BB->blackEval += bishopEvalBlack[move->to];
                } else if (move->promotesTo == 1) {
                    BB->blackKnights ^= (toBit); BB->hash ^= ZOBRIST_TABLE[move->to][7]; BB->pieceList[move->to] = -2; BB->blackEval += knightEvalBlack[move->to];
                }
            } else {
                BB->blackPawns ^= (toBit); BB->hash ^= ZOBRIST_TABLE[move->to][6]; BB->blackEval += pawnEvalBlack[move->to];
            }
        }
        else if (move->pieceType == 1) {
            BB->blackKnights ^= ((toBit) | (fromBit)); BB->hash ^= ZOBRIST_TABLE[move->to][7] ^ ZOBRIST_TABLE[move->from][7]; BB->blackEval += knightEvalBlack[move->to] - knightEvalBlack[move->from];

        } else if (move->pieceType == 2) {
            BB->blackBishops ^= ((toBit) | (fromBit)); BB->hash ^= ZOBRIST_TABLE[move->to][8] ^ ZOBRIST_TABLE[move->from][8]; BB->blackEval += bishopEvalBlack[move->to] - bishopEvalBlack[move->from];

        } else if (move->pieceType == 3) {
            BB->blackRooks ^= ((toBit) | (fromBit)); BB->hash ^= ZOBRIST_TABLE[move->to][9] ^ ZOBRIST_TABLE[move->from][9]; BB->blackEval += rookEvalBlack[move->to] - rookEvalBlack[move->from];
            if (BB->blackCastleQueenSide && move->from == 63) {
                BB->blackCastleQueenSide = false; BB->hash ^= castlingRights[1];
            } else if (BB->blackCastleKingSide && move->from == 56) {
                BB->blackCastleKingSide = false; BB->hash ^= castlingRights[0];
            }
        } else if (move->pieceType == 4) {
            BB->blackQueens ^= ((toBit) | (fromBit)); BB->hash ^= ZOBRIST_TABLE[move->to][10] ^ ZOBRIST_TABLE[move->from][10]; BB->blackEval += queenEvalBlack[move->to] - queenEvalBlack[move->from];

        } else if (move->pieceType == 5) {
            BB->blackKing ^= ((toBit) | (fromBit)); BB->hash ^= ZOBRIST_TABLE[move->to][11] ^ ZOBRIST_TABLE[move->from][11];
            if (move->castle) {
                int s;
                int o;
                if (move->castle == KINGSIDE) {s = 56; o = 58;} else {s = 63; o = 60;}

                    BB->allPieces ^= (1ULL << s) | (1ULL << o); BB->blackPieces ^= (1ULL << s) | (1ULL << o); BB->blackRooks ^= (1ULL << s) | (1ULL << o);
                    BB->hash ^= ZOBRIST_TABLE[o][9] ^ ZOBRIST_TABLE[s][9]; BB->pieceList[s] = 0; BB->pieceList[o] = -4; BB->blackEval += rookEvalBlack[o] - rookEvalBlack[s];
                }
            if (BB->blackCastleQueenSide) {
                BB->blackCastleQueenSide = false; BB->hash ^= castlingRights[1];
            }
            if (BB->blackCastleKingSide) {
                BB->blackCastleKingSide = false; BB->hash ^= castlingRights[0];
            }
        }

        if (BB->enPassantSquare) {
            BB->hash ^= ZOBRIST_TABLE[lsb(BB->enPassantSquare)][12]; BB->enPassantSquare = 0;
        }

        if (move->createsEnPassant && ((BB->whitePawns & (1ULL << (move->to + 1))) || (BB->whitePawns & (1ULL << (move->to - 1))))) {
            BB->enPassantSquare ^= (1ULL << (move->to + 8)); BB->hash ^= ZOBRIST_TABLE[move->to + 8][12];
        }
    }
    BB->color = !BB->color;
}

void undoMove(struct move_t *move, struct bitboards_t *BB, struct undo_t *undo) {

    u64 toBit = 1ULL << move->to;
    u64 fromBit = 1ULL << move->from;

    BB->allPieces ^= ((fromBit) | (toBit));
    BB->hash ^= whiteToMove;
    BB->color = !BB->color;
    BB->pieceList[move->to] = undo->capturedPiece;
    BB->pieceList[move->from] = (int) ((move->pieceType + 1) * (BB->color ? 1 : -1));

    if (BB->color) {
        if (move->isEnPassantCapture) {
            BB->blackPawns ^= (1ULL << (move->to - 8)); BB->blackPieces ^= (1ULL << (move->to - 8)); BB->allPieces ^= (1ULL << (move->to - 8));

            BB->hash ^= ZOBRIST_TABLE[(move->to - 8)][6]; BB->pieceList[move->to - 8] = -1; BB->blackEval += pawnEvalBlack[move->to - 8];

        } else if (undo->capturedPiece) {
            BB->blackPieces ^= toBit; BB->allPieces ^= toBit;

            switch (undo->capturedPiece) {
                case -1:
                    BB->blackPawns ^= (toBit); BB->hash ^= ZOBRIST_TABLE[move->to][6]; BB->blackEval += pawnEvalBlack[move->to];
                    break;
                case -2:
                    BB->blackKnights ^= (toBit); BB->hash ^= ZOBRIST_TABLE[move->to][7]; BB->blackEval += knightEvalBlack[move->to];
                    break;
                case -3:
                    BB->blackBishops ^= (toBit); BB->hash ^= ZOBRIST_TABLE[move->to][8]; BB->blackEval += bishopEvalBlack[move->to];
                    break;
                case -4:
                    BB->blackRooks ^= (toBit); BB->hash ^= ZOBRIST_TABLE[move->to][9]; BB->blackEval += rookEvalBlack[move->to];
                    break;
                case -5:
                    BB->blackQueens ^= (toBit); BB->hash ^= ZOBRIST_TABLE[move->to][10]; BB->blackEval += queenEvalBlack[move->to];
                    break;
                case -6:
                    BB->blackKing ^= (toBit); BB->hash ^= ZOBRIST_TABLE[move->to][11];
                    break;
            }
        }
        
        BB->whitePieces ^= ((fromBit) | (toBit));

        if (move->pieceType == 0) { // pawn move
            BB->whitePawns ^= (fromBit); BB->hash ^= ZOBRIST_TABLE[move->from][0]; BB->whiteEval += pawnEvalWhite[move->from];
            if (move->to >= 56) {
                // pawn promotion
                if (move->promotesTo == 4) {
                    BB->whiteQueens ^= (toBit); BB->hash ^= ZOBRIST_TABLE[move->to][4]; BB->whiteEval -= queenEvalWhite[move->to];
                } else if (move->promotesTo == 3) {
                    BB->whiteRooks ^= (toBit); BB->hash ^= ZOBRIST_TABLE[move->to][3]; BB->whiteEval -= rookEvalWhite[move->to];
                } else if (move->promotesTo == 2) {
                    BB->whiteBishops ^= (toBit); BB->hash ^= ZOBRIST_TABLE[move->to][2]; BB->whiteEval -= bishopEvalWhite[move->to];
                } else if (move->promotesTo == 1) {
                    BB->whiteKnights ^= (toBit); BB->hash ^= ZOBRIST_TABLE[move->to][1]; BB->whiteEval -= knightEvalWhite[move->to];
                }
            } else {
                BB->whitePawns ^= (toBit); BB->hash ^= ZOBRIST_TABLE[move->to][0]; BB->whiteEval -= pawnEvalWhite[move->to];
            }
        }
        else if (move->pieceType == 1) {// knight move
            BB->whiteKnights ^= ((fromBit) | (toBit)); BB->hash ^= ZOBRIST_TABLE[move->from][1] ^ ZOBRIST_TABLE[move->to][1]; BB->whiteEval -= knightEvalWhite[move->to] - knightEvalWhite[move->from];

        } else if (move->pieceType == 2) { // bishop move
            BB->whiteBishops ^= ((fromBit) | (toBit)); BB->hash ^= ZOBRIST_TABLE[move->from][2] ^ ZOBRIST_TABLE[move->to][2]; BB->whiteEval -= bishopEvalWhite[move->to] - bishopEvalWhite[move->from];

        } else if (move->pieceType == 3) { // rook move
            BB->whiteRooks ^= ((fromBit) | (toBit)); BB->hash ^= ZOBRIST_TABLE[move->from][3] ^ ZOBRIST_TABLE[move->to][3]; BB->whiteEval -= rookEvalWhite[move->to] - rookEvalWhite[move->from];
            
        } else if (move->pieceType == 4) { // queen move
            BB->whiteQueens ^= ((fromBit) | (toBit)); BB->hash ^= ZOBRIST_TABLE[move->from][4] ^ ZOBRIST_TABLE[move->to][4]; BB->whiteEval -= queenEvalWhite[move->to] - queenEvalWhite[move->from];

        } else if (move->pieceType == 5) { // king move
            BB->whiteKing ^= ((fromBit) | (toBit)); BB->hash ^= ZOBRIST_TABLE[move->from][5] ^ ZOBRIST_TABLE[move->to][5];
            if (move->castle) {
                int s;
                int o;
                if (move->castle == KINGSIDE) {s = 0; o = 2;} else {s = 7; o = 4;}
                    // put the rook on its new sqaure
                    BB->allPieces ^= (1ULL << s) | (1ULL << o); BB->whitePieces ^= (1ULL << s) | (1ULL << o); BB->whiteRooks ^= (1ULL << s) | (1ULL << o);

                    BB->hash ^= ZOBRIST_TABLE[o][3] ^ ZOBRIST_TABLE[s][3]; BB->pieceList[s] = 4; BB->pieceList[o] = 0; BB->whiteEval -= rookEvalWhite[o] - rookEvalWhite[s];
            }
        }
        
        if (BB->enPassantSquare) {
            // if there is an en passant square, remove it
            BB->hash ^= ZOBRIST_TABLE[lsb(BB->enPassantSquare)][12]; BB->enPassantSquare = 0;
        }

        if (undo->enPassantSquare) {
            // if there was an en passant square, add it back
            BB->hash ^= ZOBRIST_TABLE[lsb(undo->enPassantSquare)][12]; BB->enPassantSquare = undo->enPassantSquare;
        }
        
        if (undo->castleQueenSide && !BB->whiteCastleQueenSide) {
            BB->whiteCastleQueenSide = true; BB->hash ^= castlingRights[3];
        } 
        if (undo->castleKingSide && !BB->whiteCastleKingSide) {
            BB->whiteCastleKingSide = true; BB->hash ^= castlingRights[2];
        }

    } else {
        // Black move
        if (move->isEnPassantCapture) {
            BB->whitePawns ^= (1ULL << (move->to + 8)); BB->whitePieces ^= (1ULL << (move->to + 8)); BB->allPieces ^= (1ULL << (move->to + 8));
            BB->hash ^= ZOBRIST_TABLE[move->to + 8][0]; BB->pieceList[move->to + 8] = 1; BB->whiteEval += pawnEvalWhite[move->to + 8];

        } else if (undo->capturedPiece) {
            BB->whitePieces ^= toBit; BB->allPieces ^= toBit;

            switch (undo->capturedPiece) {
                case 1:
                    BB->whitePawns ^= toBit; BB->hash ^= ZOBRIST_TABLE[move->to][0]; BB->whiteEval += pawnEvalWhite[move->to];
                    break;
                case 2:
                    BB->whiteKnights ^= toBit; BB->hash ^= ZOBRIST_TABLE[move->to][1]; BB->whiteEval += knightEvalWhite[move->to];
                    break;
                case 3:
                    BB->whiteBishops ^= toBit; BB->hash ^= ZOBRIST_TABLE[move->to][2]; BB->whiteEval += bishopEvalWhite[move->to];
                    break;
                case 4:
                    BB->whiteRooks ^= toBit; BB->hash ^= ZOBRIST_TABLE[move->to][3]; BB->whiteEval += rookEvalWhite[move->to];
                    break;
                case 5:
                    BB->whiteQueens ^= toBit; BB->hash ^= ZOBRIST_TABLE[move->to][4]; BB->whiteEval += queenEvalWhite[move->to];
                    break;
                case 6:
                    BB->whiteKing ^= toBit; BB->hash ^= ZOBRIST_TABLE[move->to][5];
                    break;
            }
        }
        
        BB->blackPieces ^= (fromBit) | (toBit);

        if (move->pieceType == 0) {
            BB->blackPawns ^= (fromBit); BB->hash ^= ZOBRIST_TABLE[move->from][6]; BB->blackEval += pawnEvalBlack[move->from];
            if (move->to <= 7) {
                if (move->promotesTo == 4) {
                    BB->blackQueens ^= (toBit); BB->hash ^= ZOBRIST_TABLE[move->to][10]; BB->blackEval -= queenEvalBlack[move->to];
                } else if (move->promotesTo == 3) {
                    BB->blackRooks ^= (toBit); BB->hash ^= ZOBRIST_TABLE[move->to][9]; BB->blackEval -= rookEvalBlack[move->to];
                } else if (move->promotesTo == 2) {
                    BB->blackBishops ^= (toBit); BB->hash ^= ZOBRIST_TABLE[move->to][8]; BB->blackEval -= bishopEvalBlack[move->to];
                } else if (move->promotesTo == 1) {
                    BB->blackKnights ^= (toBit); BB->hash ^= ZOBRIST_TABLE[move->to][7]; BB->blackEval -= knightEvalBlack[move->to];
                }
            } else {
                BB->blackPawns ^= (toBit); BB->hash ^= ZOBRIST_TABLE[move->to][6]; BB->blackEval -= pawnEvalBlack[move->to];
            }
        }
        else if (move->pieceType == 1) {
            BB->blackKnights ^= ((toBit) | (fromBit)); BB->hash ^= ZOBRIST_TABLE[move->to][7] ^ ZOBRIST_TABLE[move->from][7]; BB->blackEval -= knightEvalBlack[move->to] - knightEvalBlack[move->from];

        } else if (move->pieceType == 2) {
            BB->blackBishops ^= ((toBit) | (fromBit)); BB->hash ^= ZOBRIST_TABLE[move->to][8] ^ ZOBRIST_TABLE[move->from][8]; BB->blackEval -= bishopEvalBlack[move->to] - bishopEvalBlack[move->from];

        } else if (move->pieceType == 3) {
            BB->blackRooks ^= ((toBit) | (fromBit)); BB->hash ^= ZOBRIST_TABLE[move->to][9] ^ ZOBRIST_TABLE[move->from][9]; BB->blackEval -= rookEvalBlack[move->to] - rookEvalBlack[move->from];

        } else if (move->pieceType == 4) {
            BB->blackQueens ^= ((toBit) | (fromBit)); BB->hash ^= ZOBRIST_TABLE[move->to][10] ^ ZOBRIST_TABLE[move->from][10]; BB->blackEval -= queenEvalBlack[move->to] - queenEvalBlack[move->from];

        } else if (move->pieceType == 5) {
            BB->blackKing ^= ((toBit) | (fromBit)); BB->hash ^= ZOBRIST_TABLE[move->to][11] ^ ZOBRIST_TABLE[move->from][11];
            if (move->castle) {
                int s;
                int o;
                if (move->castle == KINGSIDE) {s = 56; o = 58;} else {s = 63; o = 60;}

                    BB->allPieces ^= (1ULL << s) | (1ULL << o); BB->blackPieces ^= (1ULL << s) | (1ULL << o); BB->blackRooks ^= (1ULL << s) | (1ULL << o);
                    BB->hash ^= ZOBRIST_TABLE[o][9] ^ ZOBRIST_TABLE[s][9]; BB->pieceList[s] = -4; BB->pieceList[o] = 0; BB->blackEval -= rookEvalBlack[o] - rookEvalBlack[s];
            }
        }

        if (BB->enPassantSquare) {
            BB->hash ^= ZOBRIST_TABLE[lsb(BB->enPassantSquare)][12]; BB->enPassantSquare = 0;
        }

        if (undo->enPassantSquare) {
            BB->hash ^= ZOBRIST_TABLE[lsb(undo->enPassantSquare)][12]; BB->enPassantSquare = undo->enPassantSquare;
        }

        if (undo->castleQueenSide && !BB->blackCastleQueenSide) {
            BB->blackCastleQueenSide = true; BB->hash ^= castlingRights[1];
        }
        if (undo->castleKingSide && !BB->blackCastleKingSide) {
            BB->blackCastleKingSide = true; BB->hash ^= castlingRights[0];
        }
    }
}

struct undo_t *doNullMove(struct bitboards_t *BB) {
    // currently not in use
    struct undo_t *undo;

    undo->capturedPiece = 0;
    undo->castleKingSide = 0;
    undo->castleQueenSide = 0;
    undo->enPassantSquare = BB->enPassantSquare;

    BB->hash ^= whiteToMove;
    BB->color = !BB->color;
    if (BB->enPassantSquare) {
            BB->hash ^= ZOBRIST_TABLE[lsb(BB->enPassantSquare)][12];
        }
    BB->enPassantSquare = 0;
    return undo;
}

void undoNullMove(struct bitboards_t *BB, struct undo_t *undo) {
    BB->hash ^= whiteToMove;
    BB->color = !BB->color;
    if (undo->enPassantSquare) {
        BB->hash ^= ZOBRIST_TABLE[lsb(BB->enPassantSquare)][12];
    }
    BB->enPassantSquare = undo->enPassantSquare;
}

struct move_t *buildMove(char *move, struct bitboards_t *BB) {
    // make a useable move struct from a string in the form of e2e4
    struct move_t *m;
    int from = 7 - (move[0] - 'a') + (move[1] - '1') * 8;
    int to =  7 - (move[2] - 'a') + (move[3] - '1') * 8;

    int pieceType = 0;
    int castle = 0;
    int isEnPassantCapture = 0;
    int createsEnPassant = 0;
    char promotingChar;
    int promotesTo = 0;

    if (((BB->whitePawns & bit(from)) != 0) || ((BB->blackPawns & bit(from)) != 0)) {
        pieceType = 0;
        if (abs(from - to) == 16) {
            createsEnPassant = 1;
        } else if (BB->enPassantSquare & bit(to)) {
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
    } else if (((BB->whiteKnights & bit(from)) != 0) || ((BB->blackKnights & bit(from)) != 0)) {
        pieceType = 1;
    } else if (((BB->whiteBishops & bit(from)) != 0) || ((BB->blackBishops & bit(from)) != 0)) {
        pieceType = 2;
    } else if (((BB->whiteRooks & bit(from)) != 0) || ((BB->blackRooks & bit(from)) != 0)) {
        pieceType = 3;
    } else if (((BB->whiteQueens & bit(from)) != 0) || ((BB->blackQueens & bit(from)) != 0)) {
        pieceType = 4;
    } else if (((BB->whiteKing & bit(from)) != 0) || ((BB->blackKing & bit(from)) != 0)) {
        pieceType = 5;
        if (abs(from - to) == 2) {
            if (from > to) {
                castle = 1;
            } else {
                castle = 2;
            }
        }
    }

    m->from = from;
    m->to = to;
    m->pieceType = pieceType;
    m->castle = castle;
    m->isEnPassantCapture = isEnPassantCapture;
    m->createsEnPassant = createsEnPassant;
    m->promotesTo = promotesTo;

    return m;
}