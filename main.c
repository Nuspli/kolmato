#include "main.h"

int main(int argc, char *argv[]) {
    // main function with command line interface

    printf("initializing...");
    initKingAttacks();
    initKnightAttacks();
    initSlidingPieceAttacks(true);
    initSlidingPieceAttacks(false);
    initZobrist();
    initTables();
    allocateBoards();

    srand(time(NULL));

    printf("OK\n");

    // generateNewMagics();
    // parseBook();

    if (argc > 1)
    {
        if (
            strcmp(argv[1], "-h") == 0 || 
            strcmp(argv[1], "--help") == 0 ||
            strcmp(argv[1], "-help") == 0
            ) {
            printLogo();
            printHelp();
            return 0;
        }
        else 
        if (
            strcmp(argv[1], "-e") == 0 ||
            strcmp(argv[1], "--evaluate") == 0 ||
            strcmp(argv[1], "-evaluate") == 0
            ) {

            if (argc < 3) {
                printf("please provide a fen string\n");
                return 0;
            }
            fenToPosition(argv[2], bitboards);
            int isWhite = strcmp(argv[3], "w") == 0 ? 1 : 0;
            initBoards(bitboards, isWhite, argv[4], argv[5], atoi(argv[6]), atoi(argv[7]));
            int eval = quiescenceSearch(bitboards, -INF, INF, 0);
            printf("quick evaluation: %d\n", eval);
            printf("running deeper evaluation (7ply)...\n");
            int deepEval = negaMax(bitboards, 7, -INF, INF, true);
            printf("deeper evaluation: %d\n", deepEval);
            printBoard(bitboards);
            return 0;
        }
        else
        if (
            strcmp(argv[1], "-b") == 0 ||
            strcmp(argv[1], "--best") == 0 ||
            strcmp(argv[1], "-best") == 0
            ) {
            if (argc < 3) {
                printf("please provide a fen string\n");
                return 0;
            }
            int isWhite = strcmp(argv[3], "w") == 0 ? 1 : 0;

            fenToPosition(argv[2], bitboards);
            initBoards(bitboards, isWhite, argv[4], argv[5], atoi(argv[6]), atoi(argv[7]));
            printBoard(bitboards);

            maxDepth = 50;
            maxTime = 10;

            if (argc > 8) {
                if (
                    strcmp(argv[8], "-t") == 0 ||
                    strcmp(argv[8], "--time") == 0 ||
                    strcmp(argv[8], "-time") == 0
                    ) {
                        maxTime = atoi(argv[9]);
                } else if (
                    strcmp(argv[8], "-d") == 0 ||
                    strcmp(argv[8], "--depth") == 0 ||
                    strcmp(argv[8], "-depth") == 0
                    ) {
                        maxDepth = atoi(argv[9]);
                } else if (
                    strcmp(argv[8], "-b") == 0 ||
                    strcmp(argv[8], "--book") == 0 ||
                    strcmp(argv[8], "-book") == 0
                    ) {
                        useBook = true;
                        bookName = argv[9];
                }
            }

            if (argc > 10) {
                if (
                    strcmp(argv[10], "-d") == 0 ||
                    strcmp(argv[10], "--depth") == 0 ||
                    strcmp(argv[10], "-depth") == 0
                    ) {
                        maxDepth = atoi(argv[11]);
                } else if (
                    strcmp(argv[10], "-t") == 0 ||
                    strcmp(argv[10], "--time") == 0 ||
                    strcmp(argv[10], "-time") == 0
                    ) {
                        maxTime = atoi(argv[11]);
                } else if (
                    strcmp(argv[10], "-b") == 0 ||
                    strcmp(argv[10], "--book") == 0 ||
                    strcmp(argv[10], "-book") == 0
                    ) {
                        useBook = true;
                        bookName = argv[11];
                }
            }

            if (argc > 12) {
                if (
                    strcmp(argv[12], "-t") == 0 ||
                    strcmp(argv[12], "--time") == 0 ||
                    strcmp(argv[12], "-time") == 0
                    ) {
                        maxTime = atoi(argv[13]);
                } else if (
                    strcmp(argv[12], "-d") == 0 ||
                    strcmp(argv[12], "--depth") == 0 ||
                    strcmp(argv[12], "-depth") == 0
                    ) {
                        maxDepth = atoi(argv[13]);
                } else if (
                    strcmp(argv[12], "-b") == 0 ||
                    strcmp(argv[12], "--book") == 0 ||
                    strcmp(argv[12], "-book") == 0
                    ) {
                        useBook = true;
                        bookName = argv[13];
                }
            }

            printf("max time: %d\n", maxTime);
            printf("max depth: %d\n", maxDepth);
            printf("use book: %d\n", useBook);

            if (useBook) {
                initBook(bookName);
            }

            engineMove();

            if (useBook) {
                free(bookEntries);
            }
            freeTables();
            freeBoards();

            return 0;
        }
        else
        if (
            strcmp(argv[1], "-g") == 0 ||
            strcmp(argv[1], "--game") == 0 ||
            strcmp(argv[1], "-game") == 0
            ) {
            
            if (argc < 3) {
                printf("please provide a fen string\n");
                return 0;
            }
            int isWhiteToMove = strcmp(argv[3], "w") == 0 ? 1 : 0;
            fenToPosition(argv[2], bitboards);
            initBoards(bitboards, isWhiteToMove, argv[4], argv[5], atoi(argv[6]), atoi(argv[7]));
            printBoard(bitboards);
            maxDepth = 50;
            maxTime = 10;

            if (argc > 8) {
                if (
                    strcmp(argv[8], "-t") == 0 ||
                    strcmp(argv[8], "--time") == 0 ||
                    strcmp(argv[8], "-time") == 0
                    ) {
                        maxTime = atoi(argv[9]);
                } else if (
                    strcmp(argv[8], "-d") == 0 ||
                    strcmp(argv[8], "--depth") == 0 ||
                    strcmp(argv[8], "-depth") == 0
                    ) {
                        maxDepth = atoi(argv[9]);
                } else if (
                    strcmp(argv[8], "-b") == 0 ||
                    strcmp(argv[8], "--book") == 0 ||
                    strcmp(argv[8], "-book") == 0
                    ) {
                        useBook = 1;
                        bookName = argv[9];
                }
            }

            if (argc > 10) {
                if (
                    strcmp(argv[10], "-d") == 0 ||
                    strcmp(argv[10], "--depth") == 0 ||
                    strcmp(argv[10], "-depth") == 0
                    ) {
                        maxDepth = atoi(argv[11]);
                } else if (
                    strcmp(argv[10], "-t") == 0 ||
                    strcmp(argv[10], "--time") == 0 ||
                    strcmp(argv[10], "-time") == 0
                    ) {
                        maxTime = atoi(argv[11]);
                } else if (
                    strcmp(argv[10], "-b") == 0 ||
                    strcmp(argv[10], "--book") == 0 ||
                    strcmp(argv[10], "-book") == 0
                    ) {
                        useBook = 1;
                        bookName = argv[11];
                }
            }

            if (argc > 12) {
                if (
                    strcmp(argv[12], "-t") == 0 ||
                    strcmp(argv[12], "--time") == 0 ||
                    strcmp(argv[12], "-time") == 0
                    ) {
                        maxTime = atoi(argv[13]);
                } else if (
                    strcmp(argv[12], "-d") == 0 ||
                    strcmp(argv[12], "--depth") == 0 ||
                    strcmp(argv[12], "-depth") == 0
                    ) {
                        maxDepth = atoi(argv[13]);
                } else if (
                    strcmp(argv[12], "-b") == 0 ||
                    strcmp(argv[12], "--book") == 0 ||
                    strcmp(argv[12], "-book") == 0
                    ) {
                        useBook = 1;
                        bookName = argv[13];
                }
            }

            printf("max time: %d\n", maxTime);
            printf("max depth: %d\n", maxDepth);
            printf("use book: %d\n", useBook);

            if (useBook) {
                initBook(bookName);
            }

            printf("play mode entered\n");
            printf("what color do you want to play as? (w/b)\n");
            char color[2];
            scanf("%s", &color);
            int isPlayerWhite = strcmp(color, "w") == 0 ? 1 : 0;
            printf("you play as %s\n", isPlayerWhite ? "white" : "black");
            printf("to make a move type it in the format: e2e4\n");
            printf("to quit type: quit [WARNING: NOT QUITTING CAN CAUSE MEMORY LEAKS]\n");
            printf("on promoting, you will be asked to enter a piece type in this format:\n");
            printf("q for queen, r for rook, b for bishop, n for knight\n");
            printf("continue? (1/0)\n");

            int cont = 0;
            scanf("%d", &cont);
            if (!cont) {
                return 0;
            }

            gameHistory[gameMovesPlayed++] = bitboards->hash;

            if (isWhiteToMove != isPlayerWhite) {
                engineMove();
            }

            while (true) {
                printf("your move: ");
                char notationMove[4];
                scanf("%s", &notationMove);
                if (strcmp(notationMove, "quit") == 0) {
                    freeTables();
                    freeBoards();
                    if (useBook) {
                        free(bookEntries);
                    }
                    return 0;
                }
                move_t move = buildMove(notationMove, bitboards);

                move_t legalMoves[MAX_NUM_MOVES];
                int numMoves;

                if (isPlayerWhite) {
                    numMoves = possiblemoves(
                        isPlayerWhite, 
                        bitboards->allPieces, bitboards->enPassantSquare, bitboards->whitePieces, bitboards->blackPieces, 
                        bitboards->whitePawns, bitboards->whiteKnights, bitboards->whiteBishops, bitboards->whiteRooks, bitboards->whiteQueens, bitboards->whiteKing, 
                        bitboards->whiteCastleQueenSide, bitboards->whiteCastleKingSide, &legalMoves[0]
                    );
                } else {
                    numMoves = possiblemoves(
                        isPlayerWhite, 
                        bitboards->allPieces, bitboards->enPassantSquare, bitboards->blackPieces, bitboards->whitePieces, 
                        bitboards->blackPawns, bitboards->blackKnights, bitboards->blackBishops, bitboards->blackRooks, bitboards->blackQueens, bitboards->blackKing, 
                        bitboards->blackCastleQueenSide, bitboards->blackCastleKingSide, &legalMoves[0]
                    );
                }
                bool isLegal = false;
                for (int i = 0; i < numMoves; i++) {
                    if (legalMoves[i] == move) {
                        if (mCastle(legalMoves[i]) && (isIllegalCastle(legalMoves[i], bitboards) || isInCheck(bitboards))) {
                            continue;
                        }
                        struct undo_t undo;
                        doMove(move, bitboards, &undo);
                        if (!canCaptureOpponentsKing(bitboards)) {
                            isLegal = true;
                        }
                        undoMove(move, bitboards, &undo);
                        break;
                    }
                }
                if (isLegal) {
                    printf("updated board:\n");
                    updateFenClocks(move);
                    struct undo_t undo;
                    doMove(move, bitboards, &undo);

                    if (isThreeFoldRepetition(bitboards)) {
                        printf("threefold repetition\n");
                        freeTables();
                        freeBoards();
                        if (useBook) {
                            free(bookEntries);
                        }
                        return 0;
                    }

                    gameHistory[gameMovesPlayed++] = bitboards->hash;

                    printBoard(bitboards);
                    engineMove();
                } else {
                    printf("illegal move\n");
                    printf("legal moves:\n");
                    for (int i = 0; i < numMoves; i++) {
                        printf("%s%s\n", notation[mFrom(legalMoves[i])], notation[mTo(legalMoves[i])]);
                    }
                }
            }

            return 0;
        } 
        else if (
            strcmp(argv[1], "-p") == 0 ||
            strcmp(argv[1], "--perft") == 0 ||
            strcmp(argv[1], "-perft") == 0
        ) {
            // used for debugging

            if (argc < 3) {
                    printf("please provide a fen string\n");
                    return 0;
                }
            int isWhite = strcmp(argv[3], "w") == 0 ? 1 : 0;
            fenToPosition(argv[2], bitboards);
            initBoards(bitboards, isWhite, argv[4], argv[5], 0, 0);
            printBoard(bitboards);

            printf("depth: ");
            int depth;
            scanf("%d", &depth);

            clock_t start = clock();
            u64 bulk = perft(depth, bitboards, depth);
            clock_t end = clock();

            printf("bulk: %d\n", bulk);
            printf("Elapsed time: %.2f milliseconds\n", (double)(end - start) / CLOCKS_PER_SEC * 1000);
        } else {
            printLogo();
            printHelp();
            printf("unknown command, press any key to exit...\n");
            getchar();
        }
    } else {
        printLogo();
        printHelp();
        printf("press any key to exit...\n");
        getchar();
    }
    
    return 0;
}