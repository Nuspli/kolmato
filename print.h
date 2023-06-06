#ifndef PRINT_H
#define PRINT_H

#include <stdio.h>
#include "bitboards.h"
#include "counters.h"

void printLogo();

void printHelp();

extern char notation[64][3];

void printBinary(u64 x);

void printBoard(struct bitboards_t *BITBOARDS);

void displayBoard(u64 bitboard);

#endif