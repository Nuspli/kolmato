#include "tables.h"

struct table_t *transTable;
struct table_t *quietTable;

void tableSetEntry(struct table_t *table, u64 hash, int8_t depth, int value, u8 flag) {
    hashEntry_t *entry = tableEntry(table, hash);
    if (entry->depth <= depth) {
        entry->hash = hash;
        entry->depth = depth;
        entry->value = value;
        entry->flag = flag;
    }
}

void tableSetMove(struct table_t *table, u64 hash, int8_t depth, move_t *move) {
    hashEntry_t *entry = tableEntry(table, hash);
    if (entry->depth <= depth) {
        entry->hash = hash;
        entry->depth = depth;
        memcpy(&entry->bestMove, move, sizeof(move_t));
    }
}

int tableGetEntry(struct table_t *table, u64 hash, int8_t depth, int *value, int alpha, int beta) {
    hashEntry_t *entry = tableEntry(table, hash);
    if (entry->hash == hash && entry->depth >= depth) {
        if ((entry->flag == EXACT) || 
            (entry->flag == UPPERBOUND && entry->value <= alpha) ||
            (entry->flag == LOWERBOUND && entry->value >= beta)) {
            *value = entry->value;
            return 1;
        }
    }
    return 0;
}

move_t *tableGetMove(struct table_t *table, u64 hash) {
    hashEntry_t *entry = tableEntry(table, hash);
    if (entry->hash == hash) {
        return &entry->bestMove;
    }
    return NULL;
}

void initTables() {
    // allocate memory for the transposition tables and initialize them to 0
    transTable = calloc(1, sizeof(struct table_t));
    transTable->mask = TABLE_SIZE - 1;
    transTable->entries = calloc(TABLE_SIZE, sizeof(struct hashEntry_t));

    if (transTable->entries == NULL || transTable == NULL) {
        printf("Error: failed to allocate memory for transposition table\n");
        exit(1);
    }
    
    quietTable = calloc(1, sizeof(struct table_t));
    quietTable->mask = TABLE_SIZE - 1;
    quietTable->entries = calloc(TABLE_SIZE, sizeof(struct hashEntry_t));

    if (quietTable->entries == NULL || quietTable == NULL) {
        printf("Error: failed to allocate memory for quiet table\n");
        exit(1);
    }
}

void freeTables() {
    // free the memory allocated for the transposition table
    free(transTable->entries);
    free(transTable);
    free(quietTable->entries);
    free(quietTable);
}