#include "parser.h"

int isMoveInMoves(move_t move, move_t* moves, int numMoves) {
    for (int i = 0; i < numMoves; i++) {
        if (moves[i].from == move.from && 
            moves[i].to == move.to && 
            moves[i].promotesTo == move.promotesTo && 
            moves[i].isEnPassantCapture == move.isEnPassantCapture && 
            moves[i].castle == move.castle &&
            moves[i].createsEnPassant == move.createsEnPassant &&
            moves[i].pieceType == move.pieceType) {
            return i;
        }
    }
    return -1;
}

void parseBook() {
    // parse the opening book txt files to a binary file
    struct book_t* bookPgs;
    FILE *fens;
    FILE *out;
    char *line = NULL;
    size_t len = 0;
    int read;
    int fileCounter = 0;

    bookPgs = calloc(BOOK_SIZE, sizeof(book_t));
    // readBook(bookPgs, "book40.dat");

    if (bookPgs == NULL) {
        printf("malloc book failed\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < 40; i++) {
        fens = fopen(fileList[i], "r");
        if (fens == NULL) {
            printf("fens book not found\n");
            exit(EXIT_FAILURE);
        }

        printf("file: %d\n", fileCounter);

        while ((read = getline(&line, &len, fens)) != -1) {
            if (line[0] == '\n') {break;}
            char fen[100];
            int j = 0;
            u64 lastHash = 0;
            move_t tmpMoves[MAX_NUM_MOVES];
            u64 *hashList = NULL;
            
            for (int i = 0; i < strlen(line); i++) {
                bool isWhite = true;
                char castlingStr[5] = {'\0', '\0', '\0', '\0', '\0'};
                char enPassantStr[3] = {'\0', '\0', '\0'};
                int halfmoveClock;
                int fullmoveNumber;

                if (line[i] == ' ') {
                    fen[j] = '\0';
                    i++;
                    if (line[i] == 'w') {
                        isWhite = true;
                    } else {
                        isWhite = false;
                    }
                    i += 2;
                    castlingStr[0] = line[i];
                    i++;
                    if (line[i] != ' ') {
                        castlingStr[1] = line[i];
                        i++;
                        if (line[i] != ' ') {
                            castlingStr[2] = line[i];
                            i++;
                            if (line[i] != ' ') {
                                castlingStr[3] = line[i];
                                i++;
                            }
                        }
                    }
                    i++;
                    enPassantStr[0] = line[i];
                    i++;
                    if (line[i] != ' ') {
                        enPassantStr[1] = line[i];
                        i++;
                    }
                    i++;
                    if (line[i+1] == ' ') {
                        halfmoveClock = (line[i] - '0');
                    } else {
                        if (line[i+2] == ' ') {
                            halfmoveClock = (line[i] - '0') * 10 + (line[i+1] - '0');
                            i++;
                        } else {
                            halfmoveClock = (line[i] - '0') * 100 + (line[i+1] - '0') * 10 + (line[i+2] - '0');
                            i += 2;
                        }
                    }
                    i += 2;
                    if (line[i+1] == ',') {
                        fullmoveNumber = (line[i] - '0');
                    } else {
                        if (line[i+2] == ',') {
                            fullmoveNumber = (line[i] - '0') * 10 + (line[i+1] - '0');
                            i++;
                        } else {
                            fullmoveNumber = (line[i] - '0') * 100 + (line[i+1] - '0') * 10 + (line[i+2] - '0');
                            i += 2;
                        }
                    }
                    i += 2;
                    j = 0;

                    int position[64] = {0};
                    fenToPosition(fen, position);
                    initBoards(position, isWhite, castlingStr, enPassantStr, halfmoveClock, fullmoveNumber);

                    if (hashList != NULL) {
                        if (tmpMoves != NULL) {
                            if (lastHash) {
                                int y = 0;
                                while (hashList[y] != 0) {
                                    if (hashList[y] == bitboards.hash) {
                                        book_t page = bookPgs[lastHash % BOOK_SIZE];
                                        bool entryFound = false;
                                        for (int t = 0; t < page.numEntries; t++) {
                                            if (lastHash == page.entries[t].hash) {
                                                entryFound = true;
                                                int index = isMoveInMoves(tmpMoves[y], page.entries[t].moves, page.entries[t].numMoves);
                                                if (index != -1) {
                                                    page.entries[t].occourences[index]++;
                                                } else {
                                                    page.entries[t].numMoves++;
                                                    page.entries[t].moves = realloc(page.entries[t].moves, page.entries[t].numMoves * sizeof(move_t));
                                                    page.entries[t].moves[page.entries[t].numMoves-1] = tmpMoves[y];
                                                    page.entries[t].occourences = realloc(page.entries[t].occourences, page.entries[t].numMoves * sizeof(int));
                                                    page.entries[t].occourences[page.entries[t].numMoves-1] = 1;
                                                    if (page.entries[t].moves == NULL || page.entries[t].occourences == NULL) {
                                                        printf("realloc failed\n");
                                                        exit(EXIT_FAILURE);
                                                    }
                                                }
                                                bookPgs[lastHash % BOOK_SIZE] = page;
                                                break;
                                            }
                                        }
                                        if (!entryFound) {
                                            if (page.numEntries == 0) {
                                                page.entries = malloc(sizeof(bookEntry_t));
                                            } else {
                                                page.entries = realloc(page.entries, (page.numEntries + 1) * sizeof(bookEntry_t));
                                            }
                                            page.entries[page.numEntries].hash = lastHash;
                                            page.entries[page.numEntries].numMoves = 1;
                                            page.entries[page.numEntries].moves = malloc(sizeof(move_t));
                                            page.entries[page.numEntries].moves[0] = tmpMoves[y];
                                            page.entries[page.numEntries].occourences = malloc(sizeof(int));
                                            page.entries[page.numEntries].occourences[0] = 1;
                                            if (page.entries[page.numEntries].moves == NULL || page.entries[page.numEntries].occourences == NULL) {
                                                printf("malloc failed\n");
                                                exit(EXIT_FAILURE);
                                            }
                                            page.numEntries++;
                                            bookPgs[lastHash % BOOK_SIZE] = page;
                                        }
                                        break;
                                    }
                                    y++;
                                }
                            }
                        }
                        free(hashList);
                    }

                    int numMoves = 0;

                    if (bitboards.color) {
                        numMoves = possiblemoves(
                            bitboards.color, 
                            bitboards.allPieces, bitboards.enPassantSquare, bitboards.whitePieces, bitboards.blackPieces, 
                            bitboards.whitePawns, bitboards.whiteKnights, bitboards.whiteBishops, bitboards.whiteRooks, bitboards.whiteQueens, bitboards.whiteKing, 
                            bitboards.whiteCastleQueenSide, bitboards.whiteCastleKingSide, &tmpMoves[0]
                            );
                    } else {
                        numMoves = possiblemoves(
                            bitboards.color, 
                            bitboards.allPieces, bitboards.enPassantSquare, bitboards.blackPieces, bitboards.whitePieces, 
                            bitboards.blackPawns, bitboards.blackKnights, bitboards.blackBishops, bitboards.blackRooks, bitboards.blackQueens, bitboards.blackKing, 
                            bitboards.blackCastleQueenSide, bitboards.blackCastleKingSide, &tmpMoves[0]
                            );
                    }

                    hashList = malloc(sizeof(u64) * numMoves+1);

                    if (hashList == NULL) {
                        printf("malloc failed\n");
                        exit(EXIT_FAILURE);
                    }

                    for (int i = 0; i < numMoves; i++) {
                        hashList[i] = doMove(tmpMoves[i], bitboards).hash;
                        hashList[i+1] = 0;
                    }

                    lastHash = bitboards.hash;

                    resetBoards();
                } else {
                    fen[j] = line[i];
                    j++;
                }
            }
        }
        fileCounter++;

        fclose(fens);
    }

    printf("finished reading fens\n");

    // FMA 35 is next

    // test the book using some fen
    int position[64] = {0};

    // rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1
    fenToPosition("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR\0", position);
    initBoards(position, 1, "KQkq", "-", 0, 1);

    printBoard(bitboards);
    u64 hash = bitboards.hash % BOOK_SIZE;
    book_t page = bookPgs[hash];
    bool foundHash = false;

    for (int i = 0; i < page.numEntries; i++) {
        if (page.entries[i].hash == bitboards.hash) {
            printf("found hash\n");
            printf("moves: %d\n", page.entries[i].numMoves);
            printf("%llu [ ", page.entries[i].hash);
            for (int j = 0; j < page.entries[i].numMoves; j++) {
                printf("<{%d, %d, %d, %d, %d, %d, %d}, %d> ", 
                        page.entries[i].moves[j].from, page.entries[i].moves[j].to, 
                        page.entries[i].moves[j].pieceType, page.entries[i].moves[j].castle, 
                        page.entries[i].moves[j].isEnPassantCapture, page.entries[i].moves[j].createsEnPassant, 
                        page.entries[i].moves[j].promotesTo, 
                        page.entries[i].occourences[j]);
            }
            printf("]\n");
            foundHash = true;
        }
    }

    if (!foundHash) {
        printf("hash not found\n");
    }

    printf("writing...\n");
    writeBook(bookPgs, "book40.dat");
    free(bookPgs);

    printf("opening book again\n");
    struct book_t* loadedBookPgs;
    loadedBookPgs = calloc(BOOK_SIZE, sizeof(book_t));
    readBook(loadedBookPgs, "book40.dat");

    int position2[64] = {0};
    resetBoards();

    // rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1
    fenToPosition("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR\0", position2);
    initBoards(position2, 1, "KQkq", "-", 0, 1);

    printBoard(bitboards);
    u64 hash2 = bitboards.hash % BOOK_SIZE;
    book_t page2 = loadedBookPgs[hash2];
    bool foundHash2 = false;

    for (int i = 0; i < page2.numEntries; i++) {
        if (page2.entries[i].hash == bitboards.hash) {
            printf("found hash\n");
            printf("moves: %d\n", page2.entries[i].numMoves);
            printf("%llu [ ", page2.entries[i].hash);
            for (int j = 0; j < page2.entries[i].numMoves; j++) {
                printf("<{%d, %d, %d, %d, %d, %d, %d}, %d> ", 
                        page2.entries[i].moves[j].from, page2.entries[i].moves[j].to, 
                        page2.entries[i].moves[j].pieceType, page2.entries[i].moves[j].castle, 
                        page2.entries[i].moves[j].isEnPassantCapture, page2.entries[i].moves[j].createsEnPassant, 
                        page2.entries[i].moves[j].promotesTo, 
                        page2.entries[i].occourences[j]);
            }
            printf("]\n");
            foundHash2 = true;
        }
    }

    if (!foundHash2) {
        printf("hash not found\n");
    }

    free(loadedBookPgs);

    exit(0);
}