#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "move.h"
#include "bitboards.h"

#define TABLE_SIZE 4194304
#define tableEntry(table, hash) ((table)->entries + ((hash) & (table)->mask))
#define EXACT 0
#define LOWERBOUND 1
#define UPPERBOUND 2

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

void tableSetMove(struct table_t *table, u64 hash, int8_t depth, move_t *move);

int tableGetEntry(struct table_t *table, u64 hash, int8_t depth, int *value, int alpha, int beta);

move_t *tableGetMove(struct table_t *table, u64 hash);

void initTables();

void freeTables();