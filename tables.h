#ifndef TABLES_H
#define TABLES_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "move.h"
#include "bitboards.h"

#define TABLE_SIZE 16777216 // must be a power of 2
#define tableEntry(table, hash) ((table)->entries + ((hash) & (table)->mask))
// #define tableEntry(table, hash) ((table)->entries + ((hash) % TABLE_SIZE))

#define NOENTRY 0
#define EXACT 1
#define LOWERBOUND 2
#define UPPERBOUND 3
#define INF 100000

typedef struct hashEntry_t {
    u64 hash;
    int8_t depth;
    int value;
    u8 flag;
    move_t bestMove;
} hashEntry_t;

typedef struct table_t {
    hashEntry_t *entries;
    int mask;
    u8 capacity;
} table_t;

extern struct table_t *transTable;
extern struct table_t *quietTable;

void tableSetEntry(struct table_t *table, u64 hash, int8_t depth, int value, u8 flag);

void tableSetMove(struct table_t *table, u64 hash, int8_t depth, move_t move);

int tableGetEntry(struct table_t *table, u64 hash, int8_t depth, int *value, int alpha, int beta);

move_t tableGetMove(struct table_t *table, u64 hash);

void initTables();

void freeTables();

#endif