#include "move.h"

struct bitboards_t doMove(struct move_t move, struct bitboards_t bitboards) {
    // todo: maybe using pointers to the bitboards and undo the move later is faster?

    moveCalls++;
    u64 toBit = 1ULL << move.to;
    u64 fromBit = 1ULL << move.from;
    // flip the from bit and only flip the to bit if its not a capture
    bitboards.allPieces ^= ((fromBit) | ((toBit) & ~bitboards.allPieces));
    // update the hash
    bitboards.hash ^= whiteToMove;
    // update the piece list
    bitboards.pieceList[move.from] = 0;
    bitboards.pieceList[move.to] = (int) ((move.pieceType + 1) * (bitboards.color ? 1 : -1));

    // now for the specific piece bitboards
    if (bitboards.color) {
        if (move.isEnPassantCapture) {
            // remove the captured pawn
            bitboards.blackPawns ^= (1ULL << (move.to - 8));
            bitboards.blackPieces ^= (1ULL << (move.to - 8));
            bitboards.allPieces ^= (1ULL << (move.to - 8));

            bitboards.hash ^= ZOBRIST_TABLE[(move.to - 8)][6];
            bitboards.pieceList[move.to - 8] = 0;
            // also update the eval
            bitboards.blackEval -= pawnEvalBlack[move.to - 8];

        } else if (bitboards.blackPieces & (toBit)) {
            // capture move
            // here we dont need to remove the bit from the occupied squares, because the capturing piece is already there
            bitboards.blackPieces ^= (toBit);

            if (bitboards.blackPawns & (toBit)) {
                bitboards.blackPawns ^= (toBit);
                bitboards.hash ^= ZOBRIST_TABLE[move.to][6];
                bitboards.blackEval -= pawnEvalBlack[move.to];
    
            } else if (bitboards.blackKnights & (toBit)) {
                bitboards.blackKnights ^= (toBit);
                bitboards.hash ^= ZOBRIST_TABLE[move.to][7];
                bitboards.blackEval -= knightEvalBlack[move.to];
    
            } else if (bitboards.blackBishops & (toBit)) {
                bitboards.blackBishops ^= (toBit);
                bitboards.hash ^= ZOBRIST_TABLE[move.to][8];
                bitboards.blackEval -= bishopEvalBlack[move.to];
    
            } else if (bitboards.blackRooks & (toBit)) {
                bitboards.blackRooks ^= (toBit);
                bitboards.hash ^= ZOBRIST_TABLE[move.to][9];
                bitboards.blackEval -= rookEvalBlack[move.to];
    
            } else if (bitboards.blackQueens & (toBit)) {
                bitboards.blackQueens ^= (toBit);
                bitboards.hash ^= ZOBRIST_TABLE[move.to][10];
                bitboards.blackEval -= queenEvalBlack[move.to];
    
            } else if (bitboards.blackKing & (toBit)) {
                bitboards.blackKing ^= (toBit);
                // in theory this should never happen because the search will stop if one player has no moves
                bitboards.hash ^= ZOBRIST_TABLE[move.to][11];
                bitboards.blackEval -= kingEvalBlack[move.to];
            }
            // remove the captured bit from all the boards its part of
        }
        
        bitboards.whitePieces ^= ((fromBit) | (toBit));

        if (move.pieceType == 0) { // pawn move
            bitboards.whitePawns ^= (fromBit);
            bitboards.hash ^= ZOBRIST_TABLE[move.from][0];
            bitboards.whiteEval -= pawnEvalWhite[move.from];
            if (move.to >= 56) {
                // pawn promotion
                if (move.promotesTo == 4) {
                    bitboards.whiteQueens ^= (toBit);
                    bitboards.hash ^= ZOBRIST_TABLE[move.to][4];
                    bitboards.pieceList[move.to] = 5;
                    bitboards.whiteEval += queenEvalWhite[move.to];
                } else if (move.promotesTo == 3) {
                    bitboards.whiteRooks ^= (toBit);
                    bitboards.hash ^= ZOBRIST_TABLE[move.to][3];
                    bitboards.pieceList[move.to] = 4;
                    bitboards.whiteEval += rookEvalWhite[move.to];
                } else if (move.promotesTo == 2) {
                    bitboards.whiteBishops ^= (toBit);
                    bitboards.hash ^= ZOBRIST_TABLE[move.to][2];
                    bitboards.pieceList[move.to] = 3;
                    bitboards.whiteEval += bishopEvalWhite[move.to];
                } else if (move.promotesTo == 1) {
                    bitboards.whiteKnights ^= (toBit);
                    bitboards.hash ^= ZOBRIST_TABLE[move.to][1];
                    bitboards.pieceList[move.to] = 2;
                    bitboards.whiteEval += knightEvalWhite[move.to];
                }
            } else {
                bitboards.whitePawns ^= (toBit);
                bitboards.hash ^= ZOBRIST_TABLE[move.to][0];
                bitboards.whiteEval += pawnEvalWhite[move.to];
            }
        }
        else if (move.pieceType == 1) {// knight move
            bitboards.whiteKnights ^= ((fromBit) | (toBit));
            bitboards.hash ^= ZOBRIST_TABLE[move.from][1] ^ ZOBRIST_TABLE[move.to][1];
            bitboards.whiteEval += knightEvalWhite[move.to] - knightEvalWhite[move.from];

        } else if (move.pieceType == 2) { // bishop move
            bitboards.whiteBishops ^= ((fromBit) | (toBit));
            bitboards.hash ^= ZOBRIST_TABLE[move.from][2] ^ ZOBRIST_TABLE[move.to][2];
            bitboards.whiteEval += bishopEvalWhite[move.to] - bishopEvalWhite[move.from];

        } else if (move.pieceType == 3) { // rook move
            bitboards.whiteRooks ^= ((fromBit) | (toBit));
            bitboards.hash ^= ZOBRIST_TABLE[move.from][3] ^ ZOBRIST_TABLE[move.to][3];
            bitboards.whiteEval += rookEvalWhite[move.to] - rookEvalWhite[move.from];
            if (bitboards.whiteCastleQueenSide && move.from == 7) {
                // if the rook moves from the a1 square, then the white queen side castle is no longer possible
                bitboards.whiteCastleQueenSide = false;
                bitboards.hash ^= castlingRights[3];
            } else if (bitboards.whiteCastleKingSide && move.from == 0) {
                bitboards.whiteCastleKingSide = false;
                bitboards.hash ^= castlingRights[2];
            }
        } else if (move.pieceType == 4) { // queen move
            bitboards.whiteQueens ^= ((fromBit) | (toBit));
            bitboards.hash ^= ZOBRIST_TABLE[move.from][4] ^ ZOBRIST_TABLE[move.to][4];
            bitboards.whiteEval += queenEvalWhite[move.to] - queenEvalWhite[move.from];

        } else if (move.pieceType == 5) { // king move
            bitboards.whiteKing ^= ((fromBit) | (toBit));
            bitboards.hash ^= ZOBRIST_TABLE[move.from][5] ^ ZOBRIST_TABLE[move.to][5];
            if (move.castle) {
                // castle move
                int s;
                int o;
                if (move.castle == KINGSIDE) {s = 0; o = 2;} else {s = 7; o = 4;}
                    // put the rook on its new sqaure
                    bitboards.allPieces ^= (1ULL << s) | (1ULL << o);
                    bitboards.whitePieces ^= (1ULL << s) | (1ULL << o);
                    bitboards.whiteRooks ^= (1ULL << s) | (1ULL << o);

                    bitboards.hash ^= ZOBRIST_TABLE[o][3] ^ ZOBRIST_TABLE[s][3];
                    bitboards.pieceList[s] = 0;
                    bitboards.pieceList[o] = 4;
                    bitboards.whiteEval += rookEvalWhite[o] - rookEvalWhite[s];
                }
            if (bitboards.whiteCastleQueenSide) {
                bitboards.whiteCastleQueenSide = false;
                bitboards.hash ^= castlingRights[3];
            }
            if (bitboards.whiteCastleKingSide) {
                bitboards.whiteCastleKingSide = false;
                bitboards.hash ^= castlingRights[2];
            }
        }
        
        if (bitboards.enPassantSquare) {
            // if there was an en passant square, remove it from the hash
            bitboards.hash ^= ZOBRIST_TABLE[lsb(bitboards.enPassantSquare)][12];
            bitboards.enPassantSquare = 0;
        }

        if (move.createsEnPassant && ((bitboards.blackPawns & (1ULL << (toBit + 1))) || (bitboards.blackPawns & (1ULL << (toBit - 1))))) {
            // if the move creates an en passant square and it it captureable, add it to the hash and set the bit
            bitboards.enPassantSquare ^= 1ULL << (move.to - 8);
            bitboards.hash ^= ZOBRIST_TABLE[move.to - 8][12];
        }

    } else {
        // Black move
        if (move.isEnPassantCapture) {
            bitboards.whitePawns ^= (1ULL << (move.to + 8));
            bitboards.whitePieces ^= (1ULL << (move.to + 8));
            bitboards.allPieces ^= (1ULL << (move.to + 8));
            bitboards.hash ^= ZOBRIST_TABLE[move.to + 8][0];
            bitboards.pieceList[move.to + 8] = 0;
            bitboards.whiteEval -= pawnEvalWhite[move.to + 8];

        } else if (bitboards.whitePieces & (toBit)) {
            bitboards.whitePieces ^= (toBit);

            if (bitboards.whitePawns & (toBit)) {
                bitboards.whitePawns ^= (toBit);
                bitboards.hash ^= ZOBRIST_TABLE[move.to][0];
                bitboards.whiteEval -= pawnEvalWhite[move.to];
    
            } else if (bitboards.whiteKnights & (toBit)) {
                bitboards.whiteKnights ^= (toBit);
                bitboards.hash ^= ZOBRIST_TABLE[move.to][1];
                bitboards.whiteEval -= knightEvalWhite[move.to];
    
            } else if (bitboards.whiteBishops & (toBit)) {
                bitboards.whiteBishops ^= (toBit);
                bitboards.hash ^= ZOBRIST_TABLE[move.to][2];
                bitboards.whiteEval -= bishopEvalWhite[move.to];
    
            } else if (bitboards.whiteRooks & (toBit)) {
                bitboards.whiteRooks ^= (toBit);
                bitboards.hash ^= ZOBRIST_TABLE[move.to][3];
                bitboards.whiteEval -= rookEvalWhite[move.to];
    
            } else if (bitboards.whiteQueens & (toBit)) {
                bitboards.whiteQueens ^= (toBit);
                bitboards.hash ^= ZOBRIST_TABLE[move.to][4];
                bitboards.whiteEval -= queenEvalWhite[move.to];
    
            } else if (bitboards.whiteKing & (toBit)) {
                bitboards.whiteKing ^= (toBit);
                bitboards.hash ^= ZOBRIST_TABLE[move.to][5];
                bitboards.whiteEval -= kingEvalWhite[move.to];
            }
        }
        
        bitboards.blackPieces ^= (fromBit) | (toBit);

        if (move.pieceType == 0) {
            bitboards.blackPawns ^= (fromBit);
            bitboards.hash ^= ZOBRIST_TABLE[move.from][6];
            bitboards.blackEval -= pawnEvalBlack[move.from];
            if (move.to <= 7) {
                if (move.promotesTo == 4) {
                    bitboards.blackQueens ^= (toBit);
                    bitboards.hash ^= ZOBRIST_TABLE[move.to][10];
                    bitboards.pieceList[move.to] = -5;
                    bitboards.blackEval += queenEvalBlack[move.to];
                } else if (move.promotesTo == 3) {
                    bitboards.blackRooks ^= (toBit);
                    bitboards.hash ^= ZOBRIST_TABLE[move.to][9];
                    bitboards.pieceList[move.to] = -4;
                    bitboards.blackEval += rookEvalBlack[move.to];
                } else if (move.promotesTo == 2) {
                    bitboards.blackBishops ^= (toBit);
                    bitboards.hash ^= ZOBRIST_TABLE[move.to][8];
                    bitboards.pieceList[move.to] = -3;
                    bitboards.blackEval += bishopEvalBlack[move.to];
                } else if (move.promotesTo == 1) {
                    bitboards.blackKnights ^= (toBit);
                    bitboards.hash ^= ZOBRIST_TABLE[move.to][7];
                    bitboards.pieceList[move.to] = -2;
                    bitboards.blackEval += knightEvalBlack[move.to];
                }
            } else {
                bitboards.blackPawns ^= (toBit);
                bitboards.hash ^= ZOBRIST_TABLE[move.to][6];
                bitboards.blackEval += pawnEvalBlack[move.to];
            }
        }
        else if (move.pieceType == 1) {
            bitboards.blackKnights ^= ((toBit) | (fromBit));
            bitboards.hash ^= ZOBRIST_TABLE[move.to][7] ^ ZOBRIST_TABLE[move.from][7];
            bitboards.blackEval += knightEvalBlack[move.to] - knightEvalBlack[move.from];

        } else if (move.pieceType == 2) {
            bitboards.blackBishops ^= ((toBit) | (fromBit));
            bitboards.hash ^= ZOBRIST_TABLE[move.to][8] ^ ZOBRIST_TABLE[move.from][8];
            bitboards.blackEval += bishopEvalBlack[move.to] - bishopEvalBlack[move.from];

        } else if (move.pieceType == 3) {
            bitboards.blackRooks ^= ((toBit) | (fromBit));
            bitboards.hash ^= ZOBRIST_TABLE[move.to][9] ^ ZOBRIST_TABLE[move.from][9];
            bitboards.blackEval += rookEvalBlack[move.to] - rookEvalBlack[move.from];
            if (bitboards.blackCastleQueenSide && move.from == 31) {
                bitboards.blackCastleQueenSide = false;
                bitboards.hash ^= castlingRights[1];
            } else if (bitboards.blackCastleKingSide && move.from == 24) {
                bitboards.blackCastleKingSide = false;
                bitboards.hash ^= castlingRights[0];
            }
        } else if (move.pieceType == 4) {
            bitboards.blackQueens ^= ((toBit) | (fromBit));
            bitboards.hash ^= ZOBRIST_TABLE[move.to][10] ^ ZOBRIST_TABLE[move.from][10];
            bitboards.blackEval += queenEvalBlack[move.to] - queenEvalBlack[move.from];

        } else if (move.pieceType == 5) {
            bitboards.blackKing ^= ((toBit) | (fromBit));
            bitboards.hash ^= ZOBRIST_TABLE[move.to][11] ^ ZOBRIST_TABLE[move.from][11];
            if (move.castle) {
                int s;
                int o;
                if (move.castle == KINGSIDE) {s = 56; o = 58;} else {s = 63; o = 60;}

                    bitboards.allPieces ^= (1ULL << s) | (1ULL << o);
                    bitboards.blackPieces ^= (1ULL << s) | (1ULL << o);
                    bitboards.blackRooks ^= (1ULL << s) | (1ULL << o);
                    bitboards.hash ^= ZOBRIST_TABLE[o][9] ^ ZOBRIST_TABLE[s][9];
                    bitboards.pieceList[s] = 0;
                    bitboards.pieceList[o] = -4;
                    bitboards.blackEval += rookEvalBlack[o] - rookEvalBlack[s];
                }
            if (bitboards.blackCastleQueenSide) {
                bitboards.blackCastleQueenSide = false;
                bitboards.hash ^= castlingRights[1];
            }
            if (bitboards.blackCastleKingSide) {
                bitboards.blackCastleKingSide = false;
                bitboards.hash ^= castlingRights[0];
            }
        }

        if (bitboards.enPassantSquare) {
            bitboards.hash ^= ZOBRIST_TABLE[lsb(bitboards.enPassantSquare)][12];
            bitboards.enPassantSquare = 0;
        }

        if (move.createsEnPassant && ((bitboards.whitePawns & (1ULL << (move.to + 1))) || (bitboards.whitePawns & (1ULL << (move.to - 1))))) {
            bitboards.enPassantSquare ^= (1ULL << (move.to + 8));
            bitboards.hash ^= ZOBRIST_TABLE[move.to + 8][12];
        }
    }
    bitboards.color = !bitboards.color;

    return bitboards;
}

struct bitboards_t doNullMove(struct bitboards_t bitboards) {
    // currently not in use
    bitboards.hash ^= whiteToMove;
    bitboards.color = !bitboards.color;
    if (bitboards.enPassantSquare) {
            bitboards.hash ^= ZOBRIST_TABLE[lsb(bitboards.enPassantSquare)][12];
        }
    bitboards.enPassantSquare = 0;
    return bitboards;
}

struct move_t buildMove(char *move, struct bitboards_t bitboards) {
    // make a useable move struct from a string in the form of e2e4
    struct move_t m;
    int from = 7 - (move[0] - 'a') + (move[1] - '1') * 8;
    int to =  7 - (move[2] - 'a') + (move[3] - '1') * 8;
    
    int pieceType = 0;
    int castle = 0;
    int isEnPassantCapture = 0;
    int createsEnPassant = 0;
    char promotingChar;
    int promotesTo = 0;

    if (((bitboards.whitePawns & bit(from)) != 0) || ((bitboards.blackPawns & bit(from)) != 0)) {
        pieceType = 0;
        if (abs(from - to) == 16) {
            createsEnPassant = 1;
        } else if (bitboards.enPassantSquare & bit(to)) {
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
    } else if (((bitboards.whiteKnights & bit(from)) != 0) || ((bitboards.blackKnights & bit(from)) != 0)) {
        pieceType = 1;
    } else if (((bitboards.whiteBishops & bit(from)) != 0) || ((bitboards.blackBishops & bit(from)) != 0)) {
        pieceType = 2;
    } else if (((bitboards.whiteRooks & bit(from)) != 0) || ((bitboards.blackRooks & bit(from)) != 0)) {
        pieceType = 3;
    } else if (((bitboards.whiteQueens & bit(from)) != 0) || ((bitboards.blackQueens & bit(from)) != 0)) {
        pieceType = 4;
    } else if (((bitboards.whiteKing & bit(from)) != 0) || ((bitboards.blackKing & bit(from)) != 0)) {
        pieceType = 5;
        if (abs(from - to) == 2) {
            if (from > to) {
                castle = 1;
            } else {
                castle = 2;
            }
        }
    }

    m.from = from;
    m.to = to;
    m.pieceType = pieceType;
    m.castle = castle;
    m.isEnPassantCapture = isEnPassantCapture;
    m.createsEnPassant = createsEnPassant;
    m.promotesTo = promotesTo;

    return m;
}