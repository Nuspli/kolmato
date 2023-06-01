#include "random.h"

unsigned int randomNum = 1415926535;

unsigned int randInt() {
    // pseudo random unsigned integer
	unsigned int x = randomNum;
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
	randomNum = x;
	return x;
}

u64 rand64() {
    u64 u1, u2, u3, u4;
    
    u1 = (u64)(randInt()) & 0xFFFF;
    u2 = (u64)(randInt()) & 0xFFFF;
    u3 = (u64)(randInt()) & 0xFFFF;
    u4 = (u64)(randInt()) & 0xFFFF;
    
    return u1 | (u2 << 16) | (u3 << 32) | (u4 << 48);
}

u64 Rand64FewBits() {
    return rand64() & rand64() & rand64();
}