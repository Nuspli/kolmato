#include "book.h"

struct book_t* readBook(book_t *bookPages, char* fileName) {
    FILE *origin;
    origin = fopen(fileName, "rb");
    if (origin == NULL) {
        printf("opening book not found\n");
        exit(EXIT_FAILURE);
    }

    printf("reading book\n");

    for (int i = 0; i < BOOK_SIZE; i++) {
        // for each page in the book (the number of indices useable to access the book)
        fread(&bookPages[i].numEntries, sizeof(int), 1, origin);
        bookPages[i].entries = malloc(sizeof(bookEntry_t) * bookPages[i].numEntries);
        if (bookPages[i].entries == NULL) {
            printf("malloc failed\n");
            exit(EXIT_FAILURE);
        }
        for (int j = 0; j < bookPages[i].numEntries; j++) {
            // for each of the entries read its hash and the number of moves
            fread(&bookPages[i].entries[j].hash, sizeof(u64), 1, origin);
            fread(&bookPages[i].entries[j].numMoves, sizeof(u8), 1, origin);
            bookPages[i].entries[j].moves = malloc(sizeof(move_t) * bookPages[i].entries[j].numMoves);
            bookPages[i].entries[j].occourences = malloc(sizeof(int) * bookPages[i].entries[j].numMoves);
            if (bookPages[i].entries[j].moves == NULL || bookPages[i].entries[j].occourences == NULL) {
                printf("malloc failed\n");
                exit(EXIT_FAILURE);
            }
            // read all of the moves and move occourences of that entry
            for (int l = 0; l < bookPages[i].entries[j].numMoves; l++) {
                fread(&bookPages[i].entries[j].moves[l], sizeof(move_t), 1, origin);
                fread(&bookPages[i].entries[j].occourences[l], sizeof(int), 1, origin);
            }
        }
    }

    fclose(origin);
    return bookPages;
}

void writeBook(book_t *bookPages, char* fileName) {
    FILE *out;
    out = fopen(fileName, "wb");
    if (out == NULL) {
        printf("opening book not found\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < BOOK_SIZE; i++) {
        fwrite(&bookPages[i].numEntries, sizeof(int), 1, out);
        for (int j = 0; j < bookPages[i].numEntries; j++) {
            fwrite(&bookPages[i].entries[j].hash, sizeof(u64), 1, out);
            fwrite(&bookPages[i].entries[j].numMoves, sizeof(u8), 1, out);
            for (int l = 0; l < bookPages[i].entries[j].numMoves; l++) {
                fwrite(&bookPages[i].entries[j].moves[l], sizeof(move_t), 1, out);
                fwrite(&bookPages[i].entries[j].occourences[l], sizeof(int), 1, out);
            }
        }
    }
    fclose(out);
    printf("finished writing book\n");
}

void initBook() {
    bookEntries = malloc(sizeof(book_t) * BOOK_SIZE);
    bookEntries = memset(bookEntries, 0, sizeof(book_t) * BOOK_SIZE);
    if (bookEntries == NULL) {
        printf("malloc failed\n");
        exit(EXIT_FAILURE);
    }
    bookEntries = readBook(bookEntries, "book30.dat");
    printf("book read\n");
}
