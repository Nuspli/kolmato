#include "magic.h"

int rookRelevantOccupancyAmount[64] = {
    12,11,11,11,11,11,11,12,
    11,10,10,10,10,10,10,11,
    11,10,10,10,10,10,10,11,
    11,10,10,10,10,10,10,11,
    11,10,10,10,10,10,10,11,
    11,10,10,10,10,10,10,11,
    11,10,10,10,10,10,10,11,
    12,11,11,11,11,11,11,12
};

int bishopRelevantOccupancyAmount[64] = {
     6, 5, 5, 5, 5, 5, 5, 6,
     5, 5, 5, 5, 5, 5, 5, 5,
     5, 5, 7, 7, 7, 7, 5, 5,
     5, 5, 7, 9, 9, 7, 5, 5,
     5, 5, 7, 9, 9, 7, 5, 5,
     5, 5, 7, 7, 7, 7, 5, 5,
     5, 5, 5, 5, 5, 5, 5, 5,
     6, 5, 5, 5, 5, 5, 5, 6
};

u64 bishopAttackMasks[64] = {
    0x0040201008040200ULL, 0x0000402010080400ULL, 0x0000004020100a00ULL, 0x0000000040221400ULL, 0x0000000002442800ULL, 0x0000000204085000ULL, 0x0000020408102000ULL, 0x0002040810204000ULL,
    0x0020100804020000ULL, 0x0040201008040000ULL, 0x00004020100a0000ULL, 0x0000004022140000ULL, 0x0000000244280000ULL, 0x0000020408500000ULL, 0x0002040810200000ULL, 0x0004081020400000ULL, 
    0x0010080402000200ULL, 0x0020100804000400ULL, 0x004020100a000a00ULL, 0x0000402214001400ULL, 0x0000024428002800ULL, 0x0002040850005000ULL, 0x0004081020002000ULL, 0x0008102040004000ULL,
    0x0008040200020400ULL, 0x0010080400040800ULL, 0x0020100a000a1000ULL, 0x0040221400142200ULL, 0x0002442800284400ULL, 0x0004085000500800ULL, 0x0008102000201000ULL, 0x0010204000402000ULL, 
    0x0004020002040800ULL, 0x0008040004081000ULL, 0x00100a000a102000ULL, 0x0022140014224000ULL, 0x0044280028440200ULL, 0x0008500050080400ULL, 0x0010200020100800ULL, 0x0020400040201000ULL, 
    0x0002000204081000ULL, 0x0004000408102000ULL, 0x000a000a10204000ULL, 0x0014001422400000ULL, 0x0028002844020000ULL, 0x0050005008040200ULL, 0x0020002010080400ULL, 0x0040004020100800ULL, 
    0x0000020408102000ULL, 0x0000040810204000ULL, 0x00000a1020400000ULL, 0x0000142240000000ULL, 0x0000284402000000ULL, 0x0000500804020000ULL, 0x0000201008040200ULL, 0x0000402010080400ULL, 
    0x0002040810204000ULL, 0x0004081020400000ULL, 0x000a102040000000ULL, 0x0014224000000000ULL, 0x0028440200000000ULL, 0x0050080402000000ULL, 0x0020100804020000ULL, 0x0040201008040200ULL
};

u64 rookAttackMasks[64] = {
    0x000101010101017eULL, 0x000202020202027cULL, 0x000404040404047aULL, 0x0008080808080876ULL, 0x001010101010106eULL, 0x002020202020205eULL, 0x004040404040403eULL, 0x008080808080807eULL, 
    0x0001010101017e00ULL, 0x0002020202027c00ULL, 0x0004040404047a00ULL, 0x0008080808087600ULL, 0x0010101010106e00ULL, 0x0020202020205e00ULL, 0x0040404040403e00ULL, 0x0080808080807e00ULL, 
    0x00010101017e0100ULL, 0x00020202027c0200ULL, 0x00040404047a0400ULL, 0x0008080808760800ULL, 0x00101010106e1000ULL, 0x00202020205e2000ULL, 0x00404040403e4000ULL, 0x00808080807e8000ULL, 
    0x000101017e010100ULL, 0x000202027c020200ULL, 0x000404047a040400ULL, 0x0008080876080800ULL, 0x001010106e101000ULL, 0x002020205e202000ULL, 0x004040403e404000ULL, 0x008080807e808000ULL, 
    0x0001017e01010100ULL, 0x0002027c02020200ULL, 0x0004047a04040400ULL, 0x0008087608080800ULL, 0x0010106e10101000ULL, 0x0020205e20202000ULL, 0x0040403e40404000ULL, 0x0080807e80808000ULL, 
    0x00017e0101010100ULL, 0x00027c0202020200ULL, 0x00047a0404040400ULL, 0x0008760808080800ULL, 0x00106e1010101000ULL, 0x00205e2020202000ULL, 0x00403e4040404000ULL, 0x00807e8080808000ULL, 
    0x007e010101010100ULL, 0x007c020202020200ULL, 0x007a040404040400ULL, 0x0076080808080800ULL, 0x006e101010101000ULL, 0x005e202020202000ULL, 0x003e404040404000ULL, 0x007e808080808000ULL, 
    0x7e01010101010100ULL, 0x7c02020202020200ULL, 0x7a04040404040400ULL, 0x7608080808080800ULL, 0x6e10101010101000ULL, 0x5e20202020202000ULL, 0x3e40404040404000ULL, 0x7e80808080808000ULL
};

u64 rookMagics[64] = {
    0x0a8002c000108020ULL, 0x06c00049b0002001ULL, 0x0100200010090040ULL, 0x2480041000800801ULL, 
    0x0280028004000800ULL, 0x0900410008040022ULL, 0x0280020001001080ULL, 0x2880002041000080ULL, 
    0xa000800080400034ULL, 0x0004808020004000ULL, 0x2290802004801000ULL, 0x0411000d00100020ULL, 
    0x0402800800040080ULL, 0x000b000401004208ULL, 0x2409000100040200ULL, 0x0001002100004082ULL, 
    0x0022878001e24000ULL, 0x1090810021004010ULL, 0x0801030040200012ULL, 0x0500808008001000ULL, 
    0x0a08018014000880ULL, 0x8000808004000200ULL, 0x0201008080010200ULL, 0x0801020000441091ULL, 
    0x0000800080204005ULL, 0x1040200040100048ULL, 0x0000120200402082ULL, 0x0d14880480100080ULL, 
    0x0012040280080080ULL, 0x0100040080020080ULL, 0x9020010080800200ULL, 0x0813241200148449ULL, 
    0x0491604001800080ULL, 0x0100401000402001ULL, 0x4820010021001040ULL, 0x0400402202000812ULL, 
    0x0209009005000802ULL, 0x0810800601800400ULL, 0x4301083214000150ULL, 0x204026458e001401ULL, 
    0x0040204000808000ULL, 0x8001008040010020ULL, 0x8410820820420010ULL, 0x1003001000090020ULL, 
    0x0804040008008080ULL, 0x0012000810020004ULL, 0x1000100200040208ULL, 0x430000a044020001ULL, 
    0x0280009023410300ULL, 0x00e0100040002240ULL, 0x0000200100401700ULL, 0x2244100408008080ULL, 
    0x0008000400801980ULL, 0x0002000810040200ULL, 0x8010100228810400ULL, 0x2000009044210200ULL, 
    0x4080008040102101ULL, 0x0040002080411d01ULL, 0x2005524060000901ULL, 0x0502001008400422ULL, 
    0x489a000810200402ULL, 0x0001004400080a13ULL, 0x4000011008020084ULL, 0x0026002114058042ULL
};

u64 bishopMagics[64] = {
    0x89a1121896040240ULL, 0x2004844802002010ULL, 0x2068080051921000ULL, 0x62880a0220200808ULL, 
    0x0004042004000000ULL, 0x0100822020200011ULL, 0xc00444222012000aULL, 0x0028808801216001ULL, 
    0x0400492088408100ULL, 0x0201c401040c0084ULL, 0x00840800910a0010ULL, 0x0000082080240060ULL, 
    0x2000840504006000ULL, 0x30010c4108405004ULL, 0x1008005410080802ULL, 0x8144042209100900ULL, 
    0x0208081020014400ULL, 0x004800201208ca00ULL, 0x0f18140408012008ULL, 0x1004002802102001ULL, 
    0x0841000820080811ULL, 0x0040200200a42008ULL, 0x0000800054042000ULL, 0x88010400410c9000ULL, 
    0x0520040470104290ULL, 0x1004040051500081ULL, 0x2002081833080021ULL, 0x000400c00c010142ULL, 
    0x941408200c002000ULL, 0x0658810000806011ULL, 0x0188071040440a00ULL, 0x4800404002011c00ULL, 
    0x0104442040404200ULL, 0x0511080202091021ULL, 0x0004022401120400ULL, 0x80c0040400080120ULL, 
    0x8040010040820802ULL, 0x0480810700020090ULL, 0x0102008e00040242ULL, 0x0809005202050100ULL, 
    0x8002024220104080ULL, 0x0431008804142000ULL, 0x0019001802081400ULL, 0x0200014208040080ULL, 
    0x3308082008200100ULL, 0x041010500040c020ULL, 0x4012020c04210308ULL, 0x208220a202004080ULL, 
    0x0111040120082000ULL, 0x6803040141280a00ULL, 0x2101004202410000ULL, 0x8200000041108022ULL, 
    0x0000021082088000ULL, 0x0002410204010040ULL, 0x0040100400809000ULL, 0x0822088220820214ULL, 
    0x0040808090012004ULL, 0x00910224040218c9ULL, 0x0402814422015008ULL, 0x0090014004842410ULL, 
    0x0001000042304105ULL, 0x0010008830412a00ULL, 0x2520081090008908ULL, 0x40102000a0a60140ULL
};

u64 normBishopAttacks(int square, u64 occupied) {
    u64 attacks = 0ULL;
    
    int targetFile, targetRank;
    
    int rank = square / 8;
    int file = square % 8;
    
    for (targetRank = rank + 1, targetFile = file + 1; targetRank <= 7 && targetFile <= 7; targetRank++, targetFile++) { // up left
        attacks |= (1ULL << (targetRank * 8 + targetFile));
        if (occupied & (1ULL << (targetRank * 8 + targetFile))) {break;}
    }
    
    for (targetRank = rank + 1, targetFile = file - 1; targetRank <= 7 && targetFile >= 0; targetRank++, targetFile--) { // up right
        attacks |= (1ULL << (targetRank * 8 + targetFile));
        if (occupied & (1ULL << (targetRank * 8 + targetFile))) {break;}
    }
    
    for (targetRank = rank - 1, targetFile = file + 1; targetRank >= 0 && targetFile <= 7; targetRank--, targetFile++) { // down left
        attacks |= (1ULL << (targetRank * 8 + targetFile));
        if (occupied & (1ULL << (targetRank * 8 + targetFile))) {break;}
    }
    
    for (targetRank = rank - 1, targetFile = file - 1; targetRank >= 0 && targetFile >= 0; targetRank--, targetFile--) { // down right
        attacks |= (1ULL << (targetRank * 8 + targetFile));
        if (occupied & (1ULL << (targetRank * 8 + targetFile))) {break;}
    }
    return attacks;
}

u64 normRookAttacks(int square, u64 occupied) {
    u64 attacks = 0ULL;
    
    int targetFile, targetRank;
    
    int rank = square / 8;
    int file = square % 8;
    
    for (targetRank = rank + 1; targetRank <= 7; targetRank++) {
        attacks |= (1ULL << (targetRank * 8 + file));
        if (occupied & (1ULL << (targetRank * 8 + file))) {break;}
    }
    
    for (targetRank = rank - 1; targetRank >= 0; targetRank--) {
        attacks |= (1ULL << (targetRank * 8 + file));
        if (occupied & (1ULL << (targetRank * 8 + file))) {break;}
    }
    
    for (targetFile = file + 1; targetFile <= 7; targetFile++) {
        attacks |= (1ULL << (rank * 8 + targetFile));
        if (occupied & (1ULL << (rank * 8 + targetFile))) {break;}
    }
    
    for (targetFile = file - 1; targetFile >= 0; targetFile--) {
        attacks |= (1ULL << (rank * 8 + targetFile));
        if (occupied & (1ULL << (rank * 8 + targetFile))) {break;}
    }
    
    return attacks;
}

u64 setOccupancy(int index, int maskBits, u64 attackMask) {
    u64 occupancy = 0ULL;
    for (int count = 0; count < maskBits; count++) {
        int square = lsb(attackMask);
        flipBit(attackMask, square);
        
        if (index & (1 << count))
            occupancy |= (1ULL << square);
    }
    return occupancy;
}

u64 getMagic(int square, int relevantOccupiedBitAmount, bool isRook) {
    u64 occupancies[4096];
    u64 attacks[4096];
    u64 attacksInUse[4096];
    u64 attackMask = isRook ? rookAttackMasks[square] : bishopAttackMasks[square];

    int occupancyVariations = 1 << relevantOccupiedBitAmount;

    for (int count = 0; count < occupancyVariations; count++) {
        occupancies[count] = setOccupancy(count, relevantOccupiedBitAmount, attackMask);
        attacks[count] = isRook ? normRookAttacks(square, occupancies[count]) : normBishopAttacks(square, occupancies[count]);
    }

    // test magic numbers
    for (int i = 0; i < 100000000; i++) {
        u64 magic = Rand64FewBits();

        // skip testing magic number if inappropriate
        if(bitCount((attackMask * magic) & 0xFF00000000000000ULL) < 6) {continue;}

        // reset used attacks array
        memset(attacksInUse, 0ULL, sizeof(attacksInUse));

        int count, fail;
        
        // test magic index
        for (count = 0, fail = 0; !fail && count < occupancyVariations; count++) {
            int magic_index = (int)((occupancies[count] * magic) >> (64 - relevantOccupiedBitAmount));
          
            if (attacksInUse[magic_index] == 0ULL) {
                attacksInUse[magic_index] = attacks[count];
            } else if (attacksInUse[magic_index] != attacks[count]) {fail = 1;}
        }

        if (!fail) {return magic;}
    }
    
    printf("Error: failed to generate magic numbers\n");
    return 0ULL;
}

void generateNewMagics() {
    printf("u64 rookMagics[64] = {\n");
    
    for(int square = 0; square < 63; square++) {
        printf("    0x%llxULL,\n", getMagic(square, rookRelevantOccupancyAmount[square], true));
    }
    printf("   0x%llxULL\n", getMagic(63, rookRelevantOccupancyAmount[63], true));
    
    printf("};\n\nu64 bishopMagics[64] = {\n");
    
    for(int square = 0; square < 63; square++) {
        printf("    0x%llxULL,\n", getMagic(square, bishopRelevantOccupancyAmount[square], false));
    }
    printf("   0x%llxULL\n", getMagic(63, bishopRelevantOccupancyAmount[63], false));

    printf("};\n\n");
}