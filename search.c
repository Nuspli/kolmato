#include "search.h"

bool equalBoards(struct bitboards_t *b1, struct bitboards_t *b2) {
    // was used for debugging the boards as pointers

    for (int i = 0; i < 64; i++) {
        if (b1->pieceList[i] != b2->pieceList[i]) {
            printf("piece list mismatch\n");
            printf("copy: [");
            for (int j = 0; j < 64; j++) {
                printf("%d, ", b1->pieceList[j]);
            }
            printf("\b]\n");
            printf("orig: [");
            for (int j = 0; j < 64; j++) {
                printf("%d, ", b2->pieceList[j]);
            }
            printf("\b]\n");
            return false;
        }
    }

    if (
        b1->color == b2->color &&
        b1->enPassantSquare == b2->enPassantSquare &&
        b1->whiteCastleKingSide == b2->whiteCastleKingSide &&
        b1->whiteCastleQueenSide == b2->whiteCastleQueenSide &&
        b1->blackCastleKingSide == b2->blackCastleKingSide &&
        b1->blackCastleQueenSide == b2->blackCastleQueenSide &&
        b1->whitePawns == b2->whitePawns &&
        b1->whiteKnights == b2->whiteKnights &&
        b1->whiteBishops == b2->whiteBishops &&
        b1->whiteRooks == b2->whiteRooks &&
        b1->whiteQueens == b2->whiteQueens &&
        b1->whiteKing == b2->whiteKing &&
        b1->blackPawns == b2->blackPawns &&
        b1->blackKnights == b2->blackKnights &&
        b1->blackBishops == b2->blackBishops &&
        b1->blackRooks == b2->blackRooks &&
        b1->blackQueens == b2->blackQueens &&
        b1->blackKing == b2->blackKing &&
        b1->allPieces == b2->allPieces &&
        b1->whitePieces == b2->whitePieces &&
        b1->blackPieces == b2->blackPieces &&
        b1->hash == b2->hash
    ) {
        return true;
    } else {
        if (b1->color != b2->color) {
            printf("color\n");
        }
        if (b1->enPassantSquare != b2->enPassantSquare) {
            printf("enPassantSquare\n");
        }
        if (b1->whiteCastleKingSide != b2->whiteCastleKingSide) {
            printf("whiteCastleKingSide\n");
        }
        if (b1->whiteCastleQueenSide != b2->whiteCastleQueenSide) {
            printf("whiteCastleQueenSide\n");
        }
        if (b1->blackCastleKingSide != b2->blackCastleKingSide) {
            printf("blackCastleKingSide\n");
        }
        if (b1->blackCastleQueenSide != b2->blackCastleQueenSide) {
            printf("blackCastleQueenSide\n");
        }
        if (b1->whitePawns != b2->whitePawns) {
            printf("whitePawns\n");
        }
        if (b1->whiteKnights != b2->whiteKnights) {
            printf("whiteKnights\n");
        }
        if (b1->whiteBishops != b2->whiteBishops) {
            printf("whiteBishops\n");
        }
        if (b1->whiteRooks != b2->whiteRooks) {
            printf("whiteRooks\n");
        }
        if (b1->whiteQueens != b2->whiteQueens) {
            printf("whiteQueens\n");
        }
        if (b1->whiteKing != b2->whiteKing) {
            printf("whiteKing\n");
        }
        if (b1->blackPawns != b2->blackPawns) {
            printf("blackPawns\n");
        }
        if (b1->blackKnights != b2->blackKnights) {
            printf("blackKnights\n");
        }
        if (b1->blackBishops != b2->blackBishops) {
            printf("blackBishops\n");
        }
        if (b1->blackRooks != b2->blackRooks) {
            printf("blackRooks\n");
        }
        if (b1->blackQueens != b2->blackQueens) {
            printf("blackQueens\n");
        }
        if (b1->blackKing != b2->blackKing) {
            printf("blackKing\n");
        }
        if (b1->allPieces != b2->allPieces) {
            printf("allPieces\n");
        }
        if (b1->whitePieces != b2->whitePieces) {
            printf("whitePieces\n");
        }
        if (b1->blackPieces != b2->blackPieces) {
            printf("blackPieces\n");
        }
        if (b1->hash != b2->hash) {
            printf("hash\n");
        }

        return false;
    }
}

void quickSortArrayDec(struct move_t moves[], int values[], int left, int right) {
    // sorts the moves decreasingly in order of their evaluation
    if (left < right) {
        // Choose pivot value
        int pivotValue = values[left + (right - left) / 2];

        // Partition the arrays
        int i = left, j = right;
        while (i <= j) {
            while (values[i] > pivotValue) i++;
            while (values[j] < pivotValue) j--;
            if (i <= j) {
                // Swap elements
                int tmpValue = values[i];
                values[i] = values[j];
                values[j] = tmpValue;
                struct move_t tmpStruct = moves[i];
                moves[i] = moves[j];
                moves[j] = tmpStruct;
                i++;
                j--;
            }
        }

        // Recursively sort the sub-arrays
        quickSortArrayDec(moves, values, left, j);
        quickSortArrayDec(moves, values, i, right);
    }
}

void quickSortArrayInc(struct move_t moves[], int values[], int left, int right) {
    // Sorts the moves increasingly in order of their evaluation
    if (left < right) {
        int pivotValue = values[left + (right - left) / 2];

        int i = left, j = right;
        while (i <= j) {
            while (values[i] < pivotValue) i++;
            while (values[j] > pivotValue) j--;
            if (i <= j) {
                int tmpValue = values[i];
                values[i] = values[j];
                values[j] = tmpValue;
                struct move_t tmpStruct = moves[i];
                moves[i] = moves[j];
                moves[j] = tmpStruct;
                i++;
                j--;
            }
        }

        quickSortArrayInc(moves, values, left, j);
        quickSortArrayInc(moves, values, i, right);
    }
}

void orderMoves(struct move_t *moves, bitboards_t *BITBOARDS, int numMoves) {
    move_t *tableMove = tableGetMove(transTable, BITBOARDS->hash);
    int values[numMoves];
    for (int i = 0; i < numMoves; i++) {
        values[i] = 0;
        int pieceMaterial = 0;
        if (tableMove != NULL && moves[i].from == tableMove->from && moves[i].to == tableMove->to) {
            values[i] = BITBOARDS->color ? INF : -INF;
        }
        else
        if (BITBOARDS->color) {
            values[i] += whiteEvalTables[moves[i].pieceType][moves[i].to] - whiteEvalTables[moves[i].pieceType][moves[i].from];
            pieceMaterial = pieceValues[moves[i].pieceType];
            if (BITBOARDS->pieceList[moves[i].to]) {
                values[i] -= blackEvalTables[BITBOARDS->pieceList[moves[i].to] * -1 - 1][moves[i].to];
                values[i] -= pieceMaterial;
            }

        } else {
            values[i] += blackEvalTables[moves[i].pieceType][moves[i].to] - blackEvalTables[moves[i].pieceType][moves[i].from];
            pieceMaterial = pieceValues[moves[i].pieceType];
            if (BITBOARDS->pieceList[moves[i].to]) {
                values[i] -= whiteEvalTables[BITBOARDS->pieceList[moves[i].to] - 1][moves[i].to];
                values[i] += pieceMaterial;
            }
        }
    }
    if (BITBOARDS->color) {
        quickSortArrayDec(moves, values, 0, numMoves-1);
    } else {
        quickSortArrayInc(moves, values, 0, numMoves-1);
    }
}

void orderCaptures(struct move_t *moves, bitboards_t *BITBOARDS, int numMoves) {

    move_t *tableMove = tableGetMove(quietTable, BITBOARDS->hash);
    int values[numMoves];
    for (int i = 0; i < numMoves; i++) {
        values[i] = 0;
        if (tableMove != NULL && moves[i].from == tableMove->from && moves[i].to == tableMove->to) {
            values[i] = BITBOARDS->color ? INF : -INF;
        }
        else
        if (BITBOARDS->color) {
            values[i] += whiteEvalTables[moves[i].pieceType][moves[i].to] - whiteEvalTables[moves[i].pieceType][moves[i].from];
            values[i] -= blackEvalTables[(BITBOARDS->pieceList[moves[i].to] * -1) - 1][moves[i].to];
            values[i] -= pieceValues[moves[i].pieceType];

        } else {
            
            values[i] += blackEvalTables[moves[i].pieceType][moves[i].to] - blackEvalTables[moves[i].pieceType][moves[i].from];
            if (moves[i].isEnPassantCapture) {
                values[i] -= whiteEvalTables[0][moves[i].to - 8];
            } else {
                values[i] -= whiteEvalTables[BITBOARDS->pieceList[moves[i].to] - 1][moves[i].to];
            }
            
            values[i] += pieceValues[moves[i].pieceType];

        }
        
    }

    if (BITBOARDS->color) {
        quickSortArrayDec(moves, values, 0, numMoves-1);
    } else {
        quickSortArrayInc(moves, values, 0, numMoves-1);
    }
}

int quiescenceSearch(struct bitboards_t *BITBOARDS, int alpha, int beta, int depth) {
    // searches the board for captures only using recursion

    if (canCaptureOpponentsKing(BITBOARDS)) {
        return INF;
    }

    quietVisits++;
    quietNodes++;

    int value;
    if (tableGetEntry(quietTable, BITBOARDS->hash, depth, &value, alpha, beta)) {
        quietTranspositions++;
        return value;
    }
    // start with the normal evaluation since nobody can be forced to capture
    // saving a function call here for more speed
    evalCalls++;

    int evalWhite = BITBOARDS->whiteEval;
    int evalBlack = BITBOARDS->blackEval;
    
    if ((evalWhite < 1000 || evalBlack > -1000) && bitCount(BITBOARDS->whitePawns | BITBOARDS->blackPawns) <= 1) {
        int distBetweenKings = abs((lsb(BITBOARDS->whiteKing) % 8) - (lsb(BITBOARDS->blackKing) % 8)) + abs((lsb(BITBOARDS->whiteKing) / 8) - (lsb(BITBOARDS->blackKing) / 8));
        if (evalWhite + evalBlack > 0) {
            evalWhite -= 42 * distBetweenKings;
            evalWhite -= kingEvalEnd[lsb(BITBOARDS->blackKing)];
        } else if (evalWhite + evalBlack < 0) {
            evalBlack += 42 * distBetweenKings;
            evalBlack += kingEvalEnd[lsb(BITBOARDS->whiteKing)];
        } else {
            return 0;
            // draw by insufficient material
        }
    } else {
        evalWhite += kingEvalWhite[lsb(BITBOARDS->whiteKing)];
        evalBlack += kingEvalBlack[lsb(BITBOARDS->blackKing)];
    }

    value = ((int)(((evalWhite + evalBlack) * 17800) / (evalWhite - evalBlack + 10000))) * (BITBOARDS->color ? 1 : -1);

    if (value >= beta) {
        tableSetEntry(quietTable, BITBOARDS->hash, depth, value, LOWERBOUND);
        return beta;
    }
    if (value > alpha) {
        alpha = value;
    }

    struct move_t moves[MAX_NUM_MOVES];
    int moveCount = 0;
    if (BITBOARDS->color) {
        moveCount = possiblecaptures(
            BITBOARDS->color, 
            BITBOARDS->allPieces, BITBOARDS->enPassantSquare, BITBOARDS->whitePieces, BITBOARDS->blackPieces, 
            BITBOARDS->whitePawns, BITBOARDS->whiteKnights, BITBOARDS->whiteBishops, BITBOARDS->whiteRooks, BITBOARDS->whiteQueens, BITBOARDS->whiteKing, 
            BITBOARDS->whiteCastleQueenSide, BITBOARDS->whiteCastleKingSide, &moves[0]
            );
    } else {
        moveCount = possiblecaptures(
            BITBOARDS->color, 
            BITBOARDS->allPieces, BITBOARDS->enPassantSquare, BITBOARDS->blackPieces, BITBOARDS->whitePieces, 
            BITBOARDS->blackPawns, BITBOARDS->blackKnights, BITBOARDS->blackBishops, BITBOARDS->blackRooks, BITBOARDS->blackQueens, BITBOARDS->blackKing, 
            BITBOARDS->blackCastleQueenSide, BITBOARDS->blackCastleKingSide, &moves[0]
            );
    }

    orderCaptures(&moves[0], BITBOARDS, moveCount);

    int flag = UPPERBOUND;
    
    for (int i = 0; i < moveCount; i++) {
        // do each of the moves

        struct undo_t undo;
        doMove(&moves[i], BITBOARDS, &undo);

        int value = -quiescenceSearch(BITBOARDS, -beta, -alpha, depth - 1);

        undoMove(&moves[i], BITBOARDS, &undo);
        
        // update alpha and try to prune
        if (value >= beta) {
            tableSetEntry(quietTable, BITBOARDS->hash, depth, value, LOWERBOUND);
            tableSetMove(quietTable, BITBOARDS->hash, depth, &moves[i]);
            return beta;
        }
        if (value > alpha) {
            alpha = value;
            flag = EXACT;
            tableSetMove(quietTable, BITBOARDS->hash, depth, &moves[i]);
        }
    }

    tableSetEntry(quietTable, BITBOARDS->hash, depth, alpha, flag);

    return alpha;
}

int negaMax(struct bitboards_t *BITBOARDS, int ply, int alpha, int beta, int depth) {

    if (canCaptureOpponentsKing(BITBOARDS)) {
        return INF;
    }

    nodes++;
    visits++;

    int value;
    if (tableGetEntry(transTable, BITBOARDS->hash, ply, &value, alpha, beta)) {
        transpositions++;
        return value;
    }

    if (ply <= 0) {
        quiescenceCalls++;
        return quiescenceSearch(BITBOARDS, alpha, beta, 0);
    }

    struct move_t moves[MAX_NUM_MOVES];
    int moveCount = 0;

    if (BITBOARDS->color) { // white
        moveCount = possiblemoves(
            BITBOARDS->color, 
            BITBOARDS->allPieces, BITBOARDS->enPassantSquare, BITBOARDS->whitePieces, BITBOARDS->blackPieces, 
            BITBOARDS->whitePawns, BITBOARDS->whiteKnights, BITBOARDS->whiteBishops, BITBOARDS->whiteRooks, BITBOARDS->whiteQueens, BITBOARDS->whiteKing, 
            BITBOARDS->whiteCastleQueenSide, BITBOARDS->whiteCastleKingSide, &moves[0]
            );
    } else { // black
        moveCount = possiblemoves(
            BITBOARDS->color, 
            BITBOARDS->allPieces, BITBOARDS->enPassantSquare, BITBOARDS->blackPieces, BITBOARDS->whitePieces, 
            BITBOARDS->blackPawns, BITBOARDS->blackKnights, BITBOARDS->blackBishops, BITBOARDS->blackRooks, BITBOARDS->blackQueens, BITBOARDS->blackKing, 
            BITBOARDS->blackCastleQueenSide, BITBOARDS->blackCastleKingSide, &moves[0]
            );
    }

    orderMoves(&moves[0], BITBOARDS, moveCount);

    bool hasLegalMoves = false;
    int flag = UPPERBOUND;
    for (int i = 0; i < moveCount; i++) {

        if (moves[i].castle && (isIllegalCastle(&moves[i], BITBOARDS) || isInCheck(BITBOARDS))) {
            continue;
        }

        struct undo_t undo;
        doMove(&moves[i], BITBOARDS, &undo);

        int value = -negaMax(BITBOARDS, ply - 1, -beta, -alpha, depth);

        undoMove(&moves[i], BITBOARDS, &undo);

        if (value > -INF) {
            hasLegalMoves = true;
        }

        if (value >= beta) {
            tableSetEntry(transTable, BITBOARDS->hash, ply, value, LOWERBOUND);
            tableSetMove(transTable, BITBOARDS->hash, ply, &moves[i]);
            return beta;
        }

        if (value > alpha) {
            alpha = value;
            flag = EXACT;
            tableSetMove(transTable, BITBOARDS->hash, ply, &moves[i]);
        }
    }

    if (!hasLegalMoves) {
        if (isInCheck(BITBOARDS)) {
            return -INF + ply;
        } else {
            return 0;
        }
    }

    tableSetEntry(transTable, BITBOARDS->hash, ply, alpha, flag);

    return alpha;
}

struct move_t iterDeepening(struct move_t *possible, struct bitboards_t *bitboards, int numMoves) {
    // find the best move using iterative deepening
    
    nodes = 0;
    quietNodes = 0;
    transpositions = 0;
    quietTranspositions = 0;

    struct move_t best;
    int values[numMoves];

    // ordering moves here is not necessary because at depth 0 the moves will be ordered by the orderMoves function
    clock_t start_time = clock();
    
    bool stopSearch = false;
    int d = 0;
    int s = bitboards->color ? -1 : 1;
    int bestScore = 0;

    printf("pseudo legal move amount: %d\n", numMoves);

    for (int ply = 0; ply < maxDepth; ply++) {
        int low = 25;
        int high = 25;
        while (1) {
            int alpha = bestScore - low;
            int beta = bestScore + high;
            printf("Depth: %d - \n", ply+1);
            for (int i = 0; i < numMoves; i++) {
                int moveEval;
                if (stopSearch) {
                    moveEval = 0;
                } else {
                    visits = 0;
                    quietVisits = 0;
                    printf("%s%s: ", notation[possible[i].from], notation[possible[i].to]);
                    if (possible[i].castle && (isIllegalCastle(&possible[i], bitboards) || isInCheck(bitboards))) {
                        printf("none\n");
                        values[i] = s * INF;
                        continue;
                    }
                    struct undo_t undo;
                    doMove(&possible[i], bitboards, &undo); // engine move
                    moveEval = s * negaMax(bitboards, ply, -INF, INF, maxDepth); // start search
                    undoMove(&possible[i], bitboards, &undo);
                    printf("%-10d quiet: %-10d eval: %d\n", visits, quietVisits, moveEval);
                }
                values[i] = moveEval;
                if (((double)(clock() - start_time)) / (CLOCKS_PER_SEC / 1000) > maxTime*1000) {
                    printf("time out\n");
                    break;
                }
                else if ((bitboards->color && (moveEval >= 99000)) || (!bitboards->color && (moveEval <= -99000))) {
                    printf("checkmate found\n");
                    return possible[i];
                    stopSearch = true;
                }
            }

            if (((double)(clock() - start_time)) / (CLOCKS_PER_SEC / 1000) > maxTime*1000) {
                break;
            }

            if (bitboards->color) {
                quickSortArrayDec(&possible[0], values, 0, numMoves-1);
            } else {
                quickSortArrayInc(&possible[0], values, 0, numMoves-1);
            }

            printf(">>> best move: from: %d  to: %d pieceType: %d value: %d\n", possible[0].from, possible[0].to, possible[0].pieceType, values[0]);
            bestScore = values[0];
            tableSetMove(transTable, bitboards->hash, ply, &possible[0]);
            d = ply;

            if (bestScore == alpha) {
                low *= 4;
                printf("repeating search...\n");
            }
            else if (bestScore == beta) {
                printf("repeating search...\n");
                high *= 4;
            }
            else {
                break;
            }
        }
        if (((double)(clock() - start_time)) / (CLOCKS_PER_SEC / 1000) > maxTime*1000) {
            break;
        }
    }

    printf("\ntranspositions found: %d (%d quiet)\n", transpositions, quietTranspositions);

    best = possible[0];
    
    if (values[0] == -100001 || values[0] == 100001) {
        // this happens on a position where the engine is already checkmated
        struct move_t move = {from: 255, to: 255, pieceType: 255, castle: 255, isEnPassantCapture: 255, createsEnPassant: 255, promotesTo: 255};
        return move;
    } else {
        return best;
    }
}

struct move_t getBookMove(u64 hash) {
    if (useBook && fullMoveCount < 20) {

        struct book_t page = bookEntries[hash % BOOK_SIZE];

        for (int i = 0; i < page.numEntries; i++) {
            if (page.entries[i].hash == hash) {
                printf("num moves: %d\n", page.entries[i].numMoves);
                quickSortArrayDec(page.entries[i].moves, page.entries[i].occourences, 0, page.entries[i].numMoves - 1);
                for (int j = 0; j < page.entries[i].numMoves; j++) {
                    printf("from: %d to: %d amount: %d\n", page.entries[i].moves[j].from, page.entries[i].moves[j].to, page.entries[i].occourences[j]);
                }
                // randomize the move selection
                int index;
                if (page.entries[i].numMoves > 3) {
                    // moves are weighted by occourences
                    if (rand() % 100 > 25) {
                        index = 0;
                    } else {
                        if (rand() % 100 > 25) {
                            index = 1;
                        } else {
                            index = 2;
                        }
                    }
                    printf("random index: %d\n", index);
                } else {
                    index = 0;
                }

                struct move_t entryMove = {
                    from: page.entries[i].moves[index].from, 
                    to: page.entries[i].moves[index].to, 
                    pieceType: page.entries[i].moves[index].pieceType, 
                    promotesTo: page.entries[i].moves[index].promotesTo, 
                    castle: page.entries[i].moves[index].castle, 
                    createsEnPassant: page.entries[i].moves[index].createsEnPassant, 
                    isEnPassantCapture: page.entries[i].moves[index].isEnPassantCapture
                    };
                return entryMove;
            }
        }

        printf("out of book\n");
    }

    struct move_t nullmove;
    nullmove.from = 255;
    nullmove.to = 255;
    nullmove.pieceType = 255;
    nullmove.promotesTo = 255;
    nullmove.castle = 255;
    nullmove.createsEnPassant = 255;
    nullmove.isEnPassantCapture = 255;
    
    return nullmove;
}

void engineMove() {
    // find and make the engine move
    clock_t start_time = clock();
    struct move_t best;
    // probe the book
    struct move_t bookMove = getBookMove(bitboards->hash);
    int numMoves = 0;
    if (bookMove.from != 255) {
        best = bookMove;
        printf("book move found!\n");
    } else {
        struct move_t possible[MAX_NUM_MOVES];
        if (bitboards->color) {
            numMoves = possiblemoves(
                bitboards->color, 
                bitboards->allPieces, bitboards->enPassantSquare, bitboards->whitePieces, bitboards->blackPieces, 
                bitboards->whitePawns, bitboards->whiteKnights, bitboards->whiteBishops, bitboards->whiteRooks, bitboards->whiteQueens, bitboards->whiteKing, 
                bitboards->whiteCastleQueenSide, bitboards->whiteCastleKingSide, &possible[0]
                );
        } else {
            numMoves = possiblemoves(
                bitboards->color, 
                bitboards->allPieces, bitboards->enPassantSquare, bitboards->blackPieces, bitboards->whitePieces, 
                bitboards->blackPawns, bitboards->blackKnights, bitboards->blackBishops, bitboards->blackRooks, bitboards->blackQueens, bitboards->blackKing, 
                bitboards->blackCastleQueenSide, bitboards->blackCastleKingSide, &possible[0]
                );
        }

        best = iterDeepening(possible, bitboards, numMoves);
    }

    if (best.from != 255) { // if there is a legal move
        updateFenClocks(best);
        struct undo_t undo;
        doMove(&best, bitboards, &undo);
        printf("final move: %s%s\n", notation[best.from], notation[best.to]);
        int s = bitboards->color ? 1 : -1;
        printf("quick evaluation: %d\n", s * quiescenceSearch(bitboards, -INF, INF, 0));
        printf("board after move\n");
        printBoard(bitboards);

        printf("nodes searched: %d (%d quiet)\n", nodes, quietNodes);
    } else {
        bitboards->color = !bitboards->color;
        if (canCaptureOpponentsKing(bitboards)) {
            printf("engine is mate\n");
        } else {
            printf("draw\n");
        }
        exit(0);
    }

    // log some stats
    clock_t end_time = clock();
    double elapsed_time = ((double)(end_time - start_time)) / (CLOCKS_PER_SEC / 1000); // Convert to milliseconds
    printf("Elapsed time: %.2f milliseconds\n", elapsed_time);
    printf("nodes per second: %d (%d quiet)\n", (int)(nodes/elapsed_time*1000), (int)(quietNodes/elapsed_time*1000));
    printf("evaluations per second: %d\n", (int)(evalCalls/elapsed_time*1000));

    // checkmate detection
    struct move_t othermoves[MAX_NUM_MOVES];
    int numOtherMoves = 0;
    if (bitboards->color) {
        numOtherMoves = possiblemoves(
            bitboards->color, 
            bitboards->allPieces, bitboards->enPassantSquare, bitboards->whitePieces, bitboards->blackPieces, 
            bitboards->whitePawns, bitboards->whiteKnights, bitboards->whiteBishops, bitboards->whiteRooks, bitboards->whiteQueens, bitboards->whiteKing, 
            bitboards->whiteCastleQueenSide, bitboards->whiteCastleKingSide, &othermoves[0]
            );
    } else {
        numOtherMoves = possiblemoves(
            bitboards->color, 
            bitboards->allPieces, bitboards->enPassantSquare, bitboards->blackPieces, bitboards->whitePieces, 
            bitboards->blackPawns, bitboards->blackKnights, bitboards->blackBishops, bitboards->blackRooks, bitboards->blackQueens, bitboards->blackKing, 
            bitboards->blackCastleQueenSide, bitboards->blackCastleKingSide, &othermoves[0]
            );
    }

    if (!hasLegalMoves(othermoves, bitboards, numOtherMoves)) {
        bitboards->color = !bitboards->color;
        if (canCaptureOpponentsKing(bitboards)) {
            printf("player is mate\n");
        } else {
            printf("draw\n");
        }
        exit(0);
    }

    printf("eval: %d moves: %d possible: %d quiet: %d\n", evalCalls, moveCalls, possibleCalls, quiescenceCalls);
    evalCalls = 0;
    moveCalls = 0;
    possibleCalls = 0;
    quiescenceCalls = 0;
}

int perft(int depth, struct bitboards_t *BITBOARDS, int originalDepth) {
    // perft function for debugging
    if (canCaptureOpponentsKing(BITBOARDS)) {
        return 0;
    }

    if (depth == 0) {
        return 1;
    }

    struct move_t possible[MAX_NUM_MOVES];
    int numMoves = 0;
    if (BITBOARDS->color) {
        numMoves = possiblemoves(
            BITBOARDS->color, 
            BITBOARDS->allPieces, BITBOARDS->enPassantSquare, BITBOARDS->whitePieces, BITBOARDS->blackPieces, 
            BITBOARDS->whitePawns, BITBOARDS->whiteKnights, BITBOARDS->whiteBishops, BITBOARDS->whiteRooks, BITBOARDS->whiteQueens, BITBOARDS->whiteKing, 
            BITBOARDS->whiteCastleQueenSide, BITBOARDS->whiteCastleKingSide, &possible[0]
            );
    } else {
        numMoves = possiblemoves(
            BITBOARDS->color, 
            BITBOARDS->allPieces, BITBOARDS->enPassantSquare, BITBOARDS->blackPieces, BITBOARDS->whitePieces, 
            BITBOARDS->blackPawns, BITBOARDS->blackKnights, BITBOARDS->blackBishops, BITBOARDS->blackRooks, BITBOARDS->blackQueens, BITBOARDS->blackKing, 
            BITBOARDS->blackCastleQueenSide, BITBOARDS->blackCastleKingSide, &possible[0]
            );
    }
    
    int nodes = 0;
    // bitboards_t copy = *BITBOARDS;
    for (int i = 0; i < numMoves; i++) {
        if (possible[i].castle && (isIllegalCastle(&possible[i], BITBOARDS) || isInCheck(BITBOARDS))) {
            continue;
        }
        struct undo_t undo;
        doMove(&possible[i], BITBOARDS, &undo);
        int nodesbefore = nodes;
        nodes += perft(depth-1, BITBOARDS, originalDepth);
        undoMove(&possible[i], BITBOARDS, &undo);
        // if (!equalBoards(&copy, BITBOARDS)) {
        //     printBoard(&copy);
        //     printBoard(BITBOARDS);

        //     printf("move %s %s\n", notation[possible[i].from], notation[possible[i].to]);
        //     printf("undo: ");
        //     printf("%d %d %d %d\n", undo.capturedPiece, undo.enPassantSquare, undo.castleQueenSide, undo.castleKingSide);
        //     exit(0);
        // }

        if (depth == originalDepth && nodes != nodesbefore) {
            printf("%s%s:", notation[possible[i].from], notation[possible[i].to]);
            printf(" %d\n", nodes-nodesbefore);
        }
        
    }

    return nodes;
}