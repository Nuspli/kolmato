#include "book.h"

struct book_t* readBook(book_t *bookPages, char* fileName) {

    // reads a custom opening book .dat file which was parsed by the parser into memory

    FILE *origin;
    origin = fopen(fileName, "rb");
    if (origin == NULL) {
        printf("opening book not found\n");
        exit(EXIT_FAILURE);
    }

    printf("reading book\n");

    for (int i = 0; i < BOOK_SIZE; i++) {

        // the book is made of BOOK_SIZE pages each containing an unknown number of entries
        // this is because the book cant be directly accessed by the hash as that would require too much space (2^64)
        // different hashes can share the same index (page) of the book since it is calculated by hash % BOOK_SIZE

        fread(&bookPages[i].numEntries, sizeof(int), 1, origin);
        bookPages[i].entries = malloc(sizeof(bookEntry_t) * bookPages[i].numEntries);

        if (bookPages[i].entries == NULL) {
            printf("malloc failed\n");
            exit(EXIT_FAILURE);
        }

        for (int j = 0; j < bookPages[i].numEntries; j++) {

            // each of the entries should have a unique hash and all of the moves that have been played at that position
            // the entry also contains the corresponding amount of times the move was played

            fread(&bookPages[i].entries[j].hash, sizeof(u64), 1, origin);
            fread(&bookPages[i].entries[j].numMoves, sizeof(u8), 1, origin);
            bookPages[i].entries[j].moves = malloc(sizeof(move_t) * bookPages[i].entries[j].numMoves);
            bookPages[i].entries[j].occourences = malloc(sizeof(int) * bookPages[i].entries[j].numMoves);

            if (bookPages[i].entries[j].moves == NULL || bookPages[i].entries[j].occourences == NULL) {
                printf("malloc failed\n");
                exit(EXIT_FAILURE);
            }
            
            for (int l = 0; l < bookPages[i].entries[j].numMoves; l++) {
                fread(&bookPages[i].entries[j].moves[l], sizeof(move_t), 1, origin);
                fread(&bookPages[i].entries[j].occourences[l], sizeof(int), 1, origin);
            }
        }
    }

    fclose(origin);
    return bookPages;
}

void writeBook(book_t *bookPages, char *fileName) {

    // writes a book from memory to a .dat file
    // this is used in the parser only

    FILE *out;
    out = fopen(fileName, "wb");
    if (out == NULL) {
        printf("opening book not found\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < BOOK_SIZE; i++) {
        // for page i, write the amount of entries on that page
        // then all of the entries
        fwrite(&bookPages[i].numEntries, sizeof(int), 1, out);

        for (int j = 0; j < bookPages[i].numEntries; j++) {
            // for entry j, write the hash and the amount of moves played for that position
            // followed by all of the moves and their occourences
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

void initBook(char *bookName) {
    bookEntries = malloc(sizeof(book_t) * BOOK_SIZE);
    bookEntries = memset(bookEntries, 0, sizeof(book_t) * BOOK_SIZE);
    if (bookEntries == NULL) {
        printf("malloc failed\n");
        exit(EXIT_FAILURE);
    }
    // specify the filename here if you compiled your own book
    bookEntries = readBook(bookEntries, bookName);
    printf("book read\n");
}
