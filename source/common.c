#include "common.h"
#include <stdlib.h>

// 2次元配列確保
void** malloc_arr(void** adr, int size, int y, int x) {
    adr     = malloc(sizeof(int*) * y);
	adr[0]  = malloc(size * x * y);

    // 先頭アドレスをセットしていく。
	// これで配列として使えるように
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


// Window
void SetWIN0(int x, int y, int x2, int y2){
	REG_WIN0H	= WIN_LEFT(x) | WIN_RIGHT(x2);
	REG_WIN0V	= WIN_TOP(y)  | WIN_BOTTOM(y2);
}

void SetWIN1(int x, int y, int x2, int y2){
	REG_WIN1H	= WIN_LEFT(x) | WIN_RIGHT(x2);
	REG_WIN1V	= WIN_TOP(y)  | WIN_BOTTOM(y2);
}