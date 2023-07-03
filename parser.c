#include "parser.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "bitboards.h"
#include "book.h"
#include "move.h"
#include "fen.h"
#include "moveGeneration.h"
#include "print.h"

#include <errno.h>

// this is not recommended but
// in order to make your own book and run this, you need to have some additional files

// DOWNLOADED GAMES FROM: https://database.nikonoel.fr/

// the PGN has to be preparsed to FEN notation before
// example result: rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1, rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1, ... ,
// 1 line is 1 entire game, it should end with a comma
// to parse the PGN to this format I used a simple python script which used the chess library


char* fileList[79] = {
    "Lichess Elite Database fens/lichess_elite_2013-09.pgn",
    "Lichess Elite Database fens/lichess_elite_2013-11.pgn",
    "Lichess Elite Database fens/lichess_elite_2014-01.pgn",
    "Lichess Elite Database fens/lichess_elite_2014-02.pgn",
    "Lichess Elite Database fens/lichess_elite_2014-03.pgn",
    "Lichess Elite Database fens/lichess_elite_2014-04.pgn",
    "Lichess Elite Database fens/lichess_elite_2014-05.pgn",
    "Lichess Elite Database fens/lichess_elite_2014-06.pgn",
    "Lichess Elite Database fens/lichess_elite_2014-07.pgn",
    "Lichess Elite Database fens/lichess_elite_2014-08.pgn",
    "Lichess Elite Database fens/lichess_elite_2014-09.pgn",
    "Lichess Elite Database fens/lichess_elite_2014-10.pgn",
    "Lichess Elite Database fens/lichess_elite_2014-11.pgn",
    "Lichess Elite Database fens/lichess_elite_2014-12.pgn",
    "Lichess Elite Database fens/lichess_elite_2015-01.pgn",
    "Lichess Elite Database fens/lichess_elite_2015-02.pgn",
    "Lichess Elite Database fens/lichess_elite_2015-03.pgn",
    "Lichess Elite Database fens/lichess_elite_2015-04.pgn",
    "Lichess Elite Database fens/lichess_elite_2015-05.pgn",
    "Lichess Elite Database fens/lichess_elite_2015-06.pgn",
    "Lichess Elite Database fens/lichess_elite_2015-07.pgn",
    "Lichess Elite Database fens/lichess_elite_2015-08.pgn",
    "Lichess Elite Database fens/lichess_elite_2015-09.pgn",
    "Lichess Elite Database fens/lichess_elite_2015-10.pgn",
    "Lichess Elite Database fens/lichess_elite_2015-11.pgn",
    "Lichess Elite Database fens/lichess_elite_2015-12.pgn",
    "Lichess Elite Database fens/lichess_elite_2016-01.pgn",
    "Lichess Elite Database fens/lichess_elite_2016-02.pgn",
    "Lichess Elite Database fens/lichess_elite_2016-03.pgn",
    "Lichess Elite Database fens/lichess_elite_2016-04.pgn",
    "Lichess Elite Database fens/lichess_elite_2016-05.pgn",
    "Lichess Elite Database fens/lichess_elite_2016-06.pgn",
    "Lichess Elite Database fens/lichess_elite_2016-07.pgn",
    "Lichess Elite Database fens/lichess_elite_2016-08.pgn",
    "Lichess Elite Database fens/lichess_elite_2016-09.pgn",
    "Lichess Elite Database fens/lichess_elite_2016-10.pgn",
    "Lichess Elite Database fens/lichess_elite_2016-11.pgn",
    "Lichess Elite Database fens/lichess_elite_2016-12.pgn",
    "Lichess Elite Database fens/lichess_elite_2017-01.pgn",
    "Lichess Elite Database fens/lichess_elite_2017-02.pgn",
    "Lichess Elite Database fens/lichess_elite_2017-03.pgn",
    "Lichess Elite Database fens/lichess_elite_2017-04.pgn",
    "Lichess Elite Database fens/lichess_elite_2017-05.pgn",
    "Lichess Elite Database fens/lichess_elite_2017-06.pgn",
    "Lichess Elite Database fens/lichess_elite_2017-07.pgn",
    "Lichess Elite Database fens/lichess_elite_2017-08.pgn",
    "Lichess Elite Database fens/lichess_elite_2017-09.pgn",
    "Lichess Elite Database fens/lichess_elite_2017-10.pgn",
    "Lichess Elite Database fens/lichess_elite_2017-11.pgn",
    "Lichess Elite Database fens/lichess_elite_2017-12.pgn",
    "Lichess Elite Database fens/lichess_elite_2018-01.pgn",
    "Lichess Elite Database fens/lichess_elite_2018-02.pgn",
    "Lichess Elite Database fens/lichess_elite_2018-03.pgn",
    "Lichess Elite Database fens/lichess_elite_2018-04.pgn",
    "Lichess Elite Database fens/lichess_elite_2018-05.pgn",
    "Lichess Elite Database fens/lichess_elite_2018-06.pgn",
    "Lichess Elite Database fens/lichess_elite_2018-07.pgn",
    "Lichess Elite Database fens/lichess_elite_2018-08.pgn",
    "Lichess Elite Database fens/lichess_elite_2018-09.pgn",
    "Lichess Elite Database fens/lichess_elite_2018-10.pgn",
    "Lichess Elite Database fens/lichess_elite_2018-11.pgn",
    "Lichess Elite Database fens/lichess_elite_2018-12.pgn",
    "Lichess Elite Database fens/lichess_elite_2019-01.pgn",
    "Lichess Elite Database fens/lichess_elite_2019-02.pgn",
    "Lichess Elite Database fens/lichess_elite_2019-03.pgn",
    "Lichess Elite Database fens/lichess_elite_2019-04.pgn",
    "Lichess Elite Database fens/lichess_elite_2019-05.pgn",
    "Lichess Elite Database fens/lichess_elite_2019-06.pgn",
    "Lichess Elite Database fens/lichess_elite_2019-07.pgn",
    "Lichess Elite Database fens/lichess_elite_2019-08.pgn",
    "Lichess Elite Database fens/lichess_elite_2019-09.pgn",
    "Lichess Elite Database fens/lichess_elite_2019-10.pgn",
    "Lichess Elite Database fens/lichess_elite_2019-11.pgn",
    "Lichess Elite Database fens/lichess_elite_2019-12.pgn",
    "Lichess Elite Database fens/lichess_elite_2020-01.pgn",
    "Lichess Elite Database fens/lichess_elite_2020-02.pgn",
    "Lichess Elite Database fens/lichess_elite_2020-03.pgn",
    "Lichess Elite Database fens/lichess_elite_2020-04.pgn",
    "Lichess Elite Database fens/lichess_elite_2020-05.pgn"
};

char bookFileName[64] = "bookMax.dat";

// all of the files combined is exactly 3818730 games

void parseBook() {
    // parse the opening book txt files to a binary file
    struct book_t* bookPgs;
    FILE *fens;
    FILE *out;
    char *line = NULL;
    int lineNum = 0;
    size_t len = 0;
    int read;
    int fileCounter = 0;

    bookPgs = calloc(BOOK_SIZE, sizeof(book_t));
    // readBook(bookPgs, "book45.dat"); // continue from an already parsed book

    if (bookPgs == NULL) {
        printf("malloc book failed\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < 79; i++) {
        fens = fopen(fileList[i], "r");
        if (fens == NULL) {
            printf("fens book not found\n");
            exit(EXIT_FAILURE);
        }

        printf("file: %d\n", fileCounter);

        while ((read = getline(&line, &len, fens)) != -1) {
            if (lineNum % 5000 == 0) {printf("line: %d\n", lineNum);}
            lineNum++;
            if (line[0] == '\n') {break;}
            char fen[90];
            int j = 0;
            u64 lastHash = 0;
            move_t tmpMoves[MAX_NUM_MOVES];
            u64 hashList[MAX_NUM_MOVES] = {0};

            int positionCounter = 0;
            
            for (int i = 0; i < strlen(line); i++) {
                if (positionCounter >= 20) {break;} // anything above 20 moves is too deep for the opening book
                bool isWhite = true;
                char castlingStr[5] = {'\0', '\0', '\0', '\0', '\0'};
                char enPassantStr[3] = {'\0', '\0', '\0'};

                if (line[i] == ' ') {
                    positionCounter++;

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
                    if (line[i+1] != ' ') {
                        if (line[i+2] == ' ') {
                            i++;
                        } else {
                            i += 2;
                        }
                    }
                    i += 2;
                    if (line[i+1] != ',') {
                        if (line[i+2] == ',') {
                            i++;
                        } else {
                            i += 2;
                        }
                    }
                    i += 2;
                    j = 0;

                    fenToPosition(fen, bitboards);
                    initBoardsLight(bitboards, isWhite, castlingStr, enPassantStr);

                    if (hashList[0] != 0) {
                        if (tmpMoves != NULL) {
                            if (lastHash) {
                                int y = 0;

                                while (hashList[y] != 0) {

                                    if (hashList[y] == bitboards->hash) {

                                        book_t page = bookPgs[lastHash % BOOK_SIZE];
                                        bool entryFound = false;
                                        for (int t = 0; t < page.numEntries; t++) {
                                            if (lastHash == page.entries[t].hash) {

                                                entryFound = true;
                                                int index = -1;
                                                for (int z = 0; z < page.entries[t].numMoves; z++) {
                                                    if (tmpMoves[y] == page.entries[t].moves[z]) {
                                                        index = z;
                                                        break;
                                                    }
                                                }
                                                if (index != -1) {
                                                    page.entries[t].occourences[index]++;
                                                } else {
                                                    page.entries[t].numMoves++;
                                                    page.entries[t].moves = realloc(page.entries[t].moves, page.entries[t].numMoves * sizeof(move_t));
                                                    page.entries[t].moves[page.entries[t].numMoves-1] = tmpMoves[y];
                                                    page.entries[t].occourences = realloc(page.entries[t].occourences, page.entries[t].numMoves * sizeof(int));
                                                    page.entries[t].occourences[page.entries[t].numMoves-1] = 1;
                                                    if (page.entries[t].moves == NULL) {
                                                        printf("realloc failed for moves\n");
                                                        writeBook(bookPgs, bookFileName);
                                                        exit(EXIT_FAILURE);
                                                    }
                                                    if (page.entries[t].occourences == NULL) {
                                                        printf("realloc failed for occourences\n");
                                                        writeBook(bookPgs, bookFileName);
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
                                                if (page.entries == NULL) {
                                                    printf("malloc failed for new entry\n");
                                                    writeBook(bookPgs, bookFileName);
                                                    exit(EXIT_FAILURE);
                                                }
                                            } else {
                                                page.entries = realloc(page.entries, (page.numEntries + 1) * sizeof(bookEntry_t));
                                                if (page.entries == NULL) {
                                                    printf("realloc failed for new entry\n");
                                                    writeBook(bookPgs, bookFileName);
                                                    exit(EXIT_FAILURE);
                                                }
                                            }
                                            page.entries[page.numEntries].hash = lastHash;
                                            page.entries[page.numEntries].numMoves = 1;
                                            page.entries[page.numEntries].moves = malloc(sizeof(move_t));
                                            page.entries[page.numEntries].moves[0] = tmpMoves[y];
                                            page.entries[page.numEntries].occourences = malloc(sizeof(int));
                                            page.entries[page.numEntries].occourences[0] = 1;
                                            if (page.entries[page.numEntries].moves == NULL) {
                                                printf("malloc failed for moves of new entry\n");
                                                writeBook(bookPgs, bookFileName);
                                                exit(EXIT_FAILURE);
                                            }
                                            if (page.entries[page.numEntries].occourences == NULL) {
                                                printf("malloc failed for occourences of new entry\n");
                                                writeBook(bookPgs, bookFileName);
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
                    }

                    int numMoves = 0;

                    if (bitboards->color) {
                        numMoves = possiblemoves(
                            bitboards->color, 
                            bitboards->bits[allPieces], bitboards->enPassantSquare, bitboards->bits[whitePieces], bitboards->bits[blackPieces], 
                            bitboards->bits[whitePawns], bitboards->bits[whiteKnights], bitboards->bits[whiteBishops], bitboards->bits[whiteRooks], bitboards->bits[whiteQueens], bitboards->bits[whiteKing], 
                            bitboards->whiteCastleQueenSide, bitboards->whiteCastleKingSide, &tmpMoves[0]
                        );
                    } else {
                        numMoves = possiblemoves(
                            bitboards->color, 
                            bitboards->bits[allPieces], bitboards->enPassantSquare, bitboards->bits[blackPieces], bitboards->bits[whitePieces], 
                            bitboards->bits[blackPawns], bitboards->bits[blackKnights], bitboards->bits[blackBishops], bitboards->bits[blackRooks], bitboards->bits[blackQueens], bitboards->bits[blackKing], 
                            bitboards->blackCastleQueenSide, bitboards->blackCastleKingSide, &tmpMoves[0]
                        );
                    }

                    for (int i = 0; i < numMoves; i++) {
                        hashList[i] = doMoveLight(tmpMoves[i], bitboards);
                    }

                    lastHash = bitboards->hash;

                    resetBoardsLight(bitboards);

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
    resetBoards(bitboards);
    // rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1
    fenToPosition("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR\0", bitboards);
    initBoards(bitboards, 1, "KQkq", "-", 0, 1);

    printBoard(bitboards);
    u64 hash = bitboards->hash % BOOK_SIZE;
    book_t page = bookPgs[hash];
    bool foundHash = false;

    for (int i = 0; i < page.numEntries; i++) {
        if (page.entries[i].hash == bitboards->hash) {
            printf("found hash: %llu\n", page.entries[i].hash);
            printf("moves: %d\n", page.entries[i].numMoves);
            for (int j = 0; j < page.entries[i].numMoves; j++) {
                printf("%s%s %d\n", notation[mFrom(page.entries[i].moves[j])], notation[mTo(page.entries[i].moves[j])], page.entries[i].occourences[j]);
            }
            foundHash = true;
        }
    }

    if (!foundHash) {
        printf("hash not found\n");
    }

    printf("writing...\n");
    writeBook(bookPgs, bookFileName);
    free(bookPgs);

    printf("opening book again\n");
    struct book_t* loadedBookPgs;
    loadedBookPgs = calloc(BOOK_SIZE, sizeof(book_t));
    readBook(loadedBookPgs, bookFileName);

    resetBoards(bitboards);

    // rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1
    fenToPosition("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR\0", bitboards);
    initBoards(bitboards, 1, "KQkq", "-", 0, 1);

    printBoard(bitboards);
    u64 hash2 = bitboards->hash % BOOK_SIZE;
    book_t page2 = loadedBookPgs[hash2];
    bool foundHash2 = false;

    for (int i = 0; i < page2.numEntries; i++) {
        if (page2.entries[i].hash == bitboards->hash) {
            printf("found hash: %llu\n", page2.entries[i].hash);
            printf("moves: %d\n", page2.entries[i].numMoves);
            for (int j = 0; j < page2.entries[i].numMoves; j++) {
                printf("%s%s %d\n", notation[mFrom(page2.entries[i].moves[j])], notation[mTo(page2.entries[i].moves[j])], page2.entries[i].occourences[j]);
            }

            foundHash2 = true;
        }
    }

    if (!foundHash2) {
        printf("hash not found\n");
    }

    free(loadedBookPgs);

    exit(0);
}