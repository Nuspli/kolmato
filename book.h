#ifndef BOOK_H
#define BOOK_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "bitboards.h"
#include "counters.h"
#include "move.h"

#define BOOK_SIZE 2000000

typedef struct bookEntry_t {
    u64 hash;
    move_t *moves;
    int *occourences;
    u8 numMoves;
} bookEntry_t;

typedef struct book_t {
    bookEntry_t *entries;
    int numEntries;
} book_t;

struct book_t *readBook(book_t *bookPages, char *fileName);

void writeBook(book_t *bookPages, char *fileName);

void initBook(char *bookName);

struct book_t *bookEntries;

#endif