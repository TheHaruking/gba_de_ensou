#include "common.h"
#include <stdlib.h>

// 2次元配列確保
void** malloc_arr(void** adr, int size, int y, int x) {
    adr     = malloc(sizeof(int*) * y);
	adr[0]  = malloc(size * x * y);
    // 先頭アドレスをセット
    for (int i = 1; i < y ; i++) {
		adr[i]  = adr[i-1] + size * x;
	}
    return adr;
}

// 2次元配列開放
void free_arr(void** adr) {
	free(adr[0]);
	free(adr);
}